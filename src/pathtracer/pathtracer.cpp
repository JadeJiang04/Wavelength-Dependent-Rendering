#include "pathtracer.h"

#include "scene/light.h"
#include "scene/sphere.h"
#include "scene/triangle.h"
#include "pathtracer/bsdf.h"

#include <algorithm>

// Global variable for current wavelength during ray tracing
double current_wavelength = 550.0;


using namespace CGL::SceneObjects;

namespace CGL {

// Helper function to convert wavelength to RGB contributions
void wavelength_to_rgb_contrib(double wavelength, double& r, double& g, double& b) {
  // Simple wavelength to RGB conversion
  // Based on approximation of CIE color matching functions
  
  if (wavelength < 380.0 || wavelength > 780.0) {
    r = g = b = 0.0;
    return;
  }
  
  if (wavelength >= 380.0 && wavelength < 440.0) {
    r = -(wavelength - 440.0) / (440.0 - 380.0);
    g = 0.0;
    b = 1.0;
  } else if (wavelength >= 440.0 && wavelength < 490.0) {
    r = 0.0;
    g = (wavelength - 440.0) / (490.0 - 440.0);
    b = 1.0;
  } else if (wavelength >= 490.0 && wavelength < 510.0) {
    r = 0.0;
    g = 1.0;
    b = -(wavelength - 510.0) / (510.0 - 490.0);
  } else if (wavelength >= 510.0 && wavelength < 580.0) {
    r = (wavelength - 510.0) / (580.0 - 510.0);
    g = 1.0;
    b = 0.0;
  } else if (wavelength >= 580.0 && wavelength < 645.0) {
    r = 1.0;
    g = -(wavelength - 645.0) / (645.0 - 580.0);
    b = 0.0;
  } else {
    r = 1.0;
    g = 0.0;
    b = 0.0;
  }
}

PathTracer::PathTracer() {
  gridSampler = new UniformGridSampler2D();
  hemisphereSampler = new UniformHemisphereSampler3D();

  tm_gamma = 2.2f;
  tm_level = 1.0f;
  tm_key = 0.18;
  tm_wht = 5.0f;
}

PathTracer::~PathTracer() {
  delete gridSampler;
  delete hemisphereSampler;
}

void PathTracer::set_frame_size(size_t width, size_t height) {
  sampleBuffer.resize(width, height);
  sampleCountBuffer.resize(width * height);
}

void PathTracer::clear() {
  bvh = NULL;
  scene = NULL;
  camera = NULL;
  sampleBuffer.clear();
  sampleCountBuffer.clear();
  sampleBuffer.resize(0, 0);
  sampleCountBuffer.resize(0, 0);
}

void PathTracer::write_to_framebuffer(ImageBuffer &framebuffer, size_t x0,
                                      size_t y0, size_t x1, size_t y1) {
  sampleBuffer.toColor(framebuffer, x0, y0, x1, y1);
}

Vector3D
PathTracer::estimate_direct_lighting_hemisphere(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // For this function, sample uniformly in a hemisphere.

  // Note: When comparing Cornel Box (CBxxx.dae) results to importance sampling, you may find the "glow" around the light source is gone.
  // This is totally fine: the area lights in importance sampling has directionality, however in hemisphere sampling we don't model this behaviour.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);

  // This is the same number of total samples as
  // estimate_direct_lighting_importance (outside of delta lights). We keep the
  // same number of samples for clarity of comparison.
  int num_samples = scene->lights.size() * ns_area_light;
  Vector3D L_out;
  Vector3D wi, wi_world;
  Intersection *w_isect = new Intersection();

  // TODO (Part 3): Write your sampling loop here
  // TODO BEFORE YOU BEGIN
  // UPDATE `est_radiance_global_illumination` to return direct lighting instead of normal shading 

  double pdf = 1 / (2 * PI);
  for (int i = 0; i < num_samples; i++) {
    wi = hemisphereSampler->get_sample();
    wi_world = o2w * wi;

    Ray r = Ray(hit_p, wi_world);
    r.min_t = EPS_F;
    if (bvh->intersect(r, w_isect))
      L_out += w_isect->bsdf->get_emission() 
             * isect.bsdf->f(w_out, wi)
             * dot(isect.n, wi_world)
             / pdf / num_samples;
  }

  return L_out;

}

Vector3D
PathTracer::estimate_direct_lighting_importance(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // To implement importance sampling, sample only from lights, not uniformly in
  // a hemisphere.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);
  Vector3D L_out;

  Vector3D wi_world, emitted_radiance, L_light;
  double distToLight, pdf, num_samples;
  Intersection *w_isect = new Intersection();

  for (auto light : scene->lights) {
    num_samples = 0.0;
    Vector3D L_light = (0, 0, 0);
    for (int i = 0; i < ns_area_light; i++) {
      emitted_radiance = light->sample_L(hit_p, &wi_world, &distToLight, &pdf);
      num_samples++;

      Ray rl = Ray(hit_p, wi_world);
      rl.min_t = EPS_F;
      rl.max_t = distToLight - EPS_F;
      if (!bvh->intersect(rl, w_isect))
        L_light += emitted_radiance 
                 * isect.bsdf->f(w_out, w2o * wi_world)
                 * max(0.0, dot(isect.n, wi_world)) / pdf;
      if (light->is_delta_light()) break;
    }
    L_light /= num_samples;
    L_out += L_light;
  }
  
  return L_out;

}

Vector3D PathTracer::zero_bounce_radiance(const Ray &r,
                                          const Intersection &isect) {
  // TODO: Part 3, Task 2
  // Returns the light that results from no bounces of light

  return isect.bsdf->get_emission();

}

double PathTracer::zero_bounce_radiance_channel(const Ray &r,
                                               const Intersection &isect) {
  // Returns the light intensity for a specific wavelength channel
  
  Vector3D emission = isect.bsdf->get_emission();
  double wavelength = r.wavelength;
  
  // Convert wavelength to RGB contribution
  double r_contrib, g_contrib, b_contrib;
  wavelength_to_rgb_contrib(wavelength, r_contrib, g_contrib, b_contrib);
  
  return emission.r * r_contrib + emission.g * g_contrib + emission.b * b_contrib;
}

Vector3D PathTracer::one_bounce_radiance(const Ray &r,
                                         const Intersection &isect) {
  // TODO: Part 3, Task 3
  // Returns either the direct illumination by hemisphere or importance sampling
  // depending on `direct_hemisphere_sample`
  if (direct_hemisphere_sample)
    return estimate_direct_lighting_hemisphere(r, isect);
  else 
    return estimate_direct_lighting_importance(r, isect);

}

double PathTracer::one_bounce_radiance_channel(const Ray &r,
                                               const Intersection &isect) {
  // Returns either the direct illumination by hemisphere or importance sampling
  // depending on `direct_hemisphere_sample` for a specific wavelength channel
  
  if (direct_hemisphere_sample)
    return estimate_direct_lighting_hemisphere_channel(r, isect);
  else 
    return estimate_direct_lighting_importance_channel(r, isect);
}

Vector3D PathTracer::at_least_one_bounce_radiance(const Ray &r,
                                                  const Intersection &isect) {
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  Vector3D hit_p = r.o + r.d * isect.t;
  Vector3D w_out = w2o * (-r.d);

  Vector3D L_out(0, 0, 0);

  // TODO: Part 4, Task 2
  // Returns the one bounce radiance + radiance from extra bounces at this point.
  // Should be called recursively to simulate extra bounces.

  if (r.depth < max_ray_depth)
    L_out += one_bounce_radiance(r, isect);

  double PR = 0.3;

  if (r.depth >= max_ray_depth || coin_flip(PR))
    return L_out;

  double pdf;
  Vector3D wi;
  Vector3D bsdf_i = isect.bsdf->sample_f(w_out, &wi, &pdf);
  Vector3D wi_world = o2w * wi;
  Intersection w_isect;

  Ray rr = Ray(hit_p, wi_world, (int)r.depth + 1);
  rr.min_t = EPS_F;

  if (!bvh->intersect(rr, &w_isect)) return L_out;

  Vector3D L_indir = at_least_one_bounce_radiance(rr, w_isect);
  L_out += L_indir * bsdf_i * max(0.0, dot(isect.n, wi_world))
         / pdf / (1 - PR);

  return L_out;
}

double PathTracer::at_least_one_bounce_radiance_channel(const Ray &r,
                                                       const Intersection &isect) {
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  Vector3D hit_p = r.o + r.d * isect.t;
  Vector3D w_out = w2o * (-r.d);

  double L_out = 0.0;

  // Returns the one bounce radiance + radiance from extra bounces at this point for specific wavelength
  if (r.depth < max_ray_depth)
    L_out += one_bounce_radiance_channel(r, isect);

  double PR = 0.3;

  if (r.depth >= max_ray_depth || coin_flip(PR))
    return L_out;

  double pdf;
  Vector3D wi;
  Vector3D bsdf_i = isect.bsdf->sample_f(w_out, &wi, &pdf);
  Vector3D wi_world = o2w * wi;
  Intersection w_isect;

  Ray rr = Ray(hit_p, wi_world, (int)r.depth + 1, r.wavelength);
  rr.min_t = EPS_F;

  if (!bvh->intersect(rr, &w_isect)) return L_out;

  double L_indir = at_least_one_bounce_radiance_channel(rr, w_isect);
  double bsdf_channel = bsdf_value_for_wavelength(isect.bsdf, w_out, wi, r.wavelength);
  
  L_out += L_indir * bsdf_channel * max(0.0, dot(isect.n, wi_world)) / pdf / (1 - PR);

  return L_out;
}

Vector3D PathTracer::est_radiance_global_illumination(const Ray &r) {
  Intersection isect;
  Vector3D L_out;

  // You will extend this in assignment 3-2.
  // If no intersection occurs, we simply return black.
  // This changes if you implement hemispherical lighting for extra credit.

  // The following line of code returns a debug color depending
  // on whether ray intersection with triangles or spheres has
  // been implemented.
  //
  // REMOVE THIS LINE when you are ready to begin Part 3.

  // TODO (Part 3): Return the direct illumination.

  // L_out = (isect.t == INF_D) ? debug_shading(r.d) : zero_bounce_radiance(r, isect);

  // TODO (Part 4): Accumulate the "direct" and "indirect"
  // parts of global illumination into L_out rather than just direct

  if (!bvh->intersect(r, &isect))
    return envLight ? envLight->sample_dir(r) : L_out;

  if (PART_4)
    L_out = zero_bounce_radiance(r, isect) + at_least_one_bounce_radiance(r, isect);
  else if (PART_3)
    L_out = zero_bounce_radiance(r, isect) + one_bounce_radiance(r, isect);
  else 
    L_out = normal_shading(isect.n);

  return L_out;
}

double PathTracer::est_radiance_global_illumination_channel(const Ray &r) {
  Intersection isect;
  double L_out = 0.0;

  if (!bvh->intersect(r, &isect))
    return 0.0;

  if (PART_4)
    L_out = zero_bounce_radiance_channel(r, isect) + at_least_one_bounce_radiance_channel(r, isect);
  else if (PART_3)
    L_out = zero_bounce_radiance_channel(r, isect) + one_bounce_radiance_channel(r, isect);
  else 
    L_out = 0.5; // Simple normal contribution for debug

  return L_out;
}

void PathTracer::raytrace_pixel(size_t x, size_t y) {
  // TODO (Part 1.2):
  // Make a loop that generates num_samples camera rays and traces them
  // through the scene. Return the average Vector3D.
  // You should call est_radiance_global_illumination in this function.

  // TODO (Part 5):
  // Modify your implementation to include adaptive sampling.
  // Use the command line parameters "samplesPerBatch" and "maxTolerance"
  
  if (PART_5) {
    Vector2D origin = Vector2D(x, y);
    Vector3D radiance = Vector3D(0, 0, 0);
    double S1 = 0.0;
    double S2 = 0.0;
    double mu;
    double sigma;
    int num_samples;
    Vector3D radiance_i;
    
    // Sample for each color channel separately
    double channel_radiance[3] = {0, 0, 0};
    int samples_per_channel = ns_aa / 3; // Divide samples among 3 channels
    if (samples_per_channel < 1) samples_per_channel = 1;
    
    for (int channel = 0; channel < 3; channel++) {
      for (int i = 0; i < samples_per_channel; i++) {
        Vector2D sample_point = origin + gridSampler->get_sample();
        Ray r = camera->generate_ray(sample_point.x / sampleBuffer.w, 
                                     sample_point.y / sampleBuffer.h, channel);
        current_wavelength = r.wavelength;
        double channel_intensity = est_radiance_global_illumination_channel(r);
        channel_radiance[channel] += channel_intensity / samples_per_channel;
      }
    }
    
    // Combine channel results into RGB color
    radiance = Vector3D(channel_radiance[0], channel_radiance[1], channel_radiance[2]);

    sampleBuffer.update_pixel(radiance, x, y);
    sampleCountBuffer[x + y * sampleBuffer.w] = ns_aa;
  }
  

  else if (PART_1) {
    int num_samples = ns_aa;          // total samples to evaluate
    Vector2D origin = Vector2D(x, y); // bottom left corner of the pixel
    Vector3D radiance = Vector3D(0, 0, 0);
    
    // Sample for each color channel separately
    double channel_radiance[3] = {0, 0, 0};
    int samples_per_channel = num_samples / 3; // Divide samples among 3 channels
    if (samples_per_channel < 1) samples_per_channel = 1;
    
    for (int channel = 0; channel < 3; channel++) {
      for (int i = 0; i < samples_per_channel; i++) {
        Vector2D sample_point = origin + gridSampler->get_sample();
        Ray r = camera->generate_ray(sample_point.x / sampleBuffer.w,
                                     sample_point.y / sampleBuffer.h, channel);
        current_wavelength = r.wavelength;
        double channel_intensity = est_radiance_global_illumination_channel(r);
        channel_radiance[channel] += channel_intensity / samples_per_channel;
      }
    }
    
    // Combine channel results into RGB color
    radiance = Vector3D(channel_radiance[0], channel_radiance[1], channel_radiance[2]);
    
    sampleBuffer.update_pixel(radiance, x, y);
    sampleCountBuffer[x + y * sampleBuffer.w] = num_samples;
  }
}

void PathTracer::autofocus(Vector2D loc) {
  Ray r = camera->generate_ray(loc.x / sampleBuffer.w, loc.y / sampleBuffer.h);
  Intersection isect;

  bvh->intersect(r, &isect);

  camera->focalDistance = isect.t;
}

// Additional helper functions for wavelength-dependent rendering
double bsdf_value_for_wavelength(BSDF* bsdf, const Vector3D& wo, const Vector3D& wi, double wavelength) {
  Vector3D bsdf_rgb = bsdf->f(wo, wi);
  double r_contrib, g_contrib, b_contrib;
  wavelength_to_rgb_contrib(wavelength, r_contrib, g_contrib, b_contrib);
  
  return bsdf_rgb.r * r_contrib + bsdf_rgb.g * g_contrib + bsdf_rgb.b * b_contrib;
}

double PathTracer::estimate_direct_lighting_hemisphere_channel(const Ray &r,
                                                             const Intersection &isect) {
  // Hemisphere sampling for specific wavelength channel
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);

  int num_samples = scene->lights.size() * ns_area_light;
  double L_out = 0.0;
  Vector3D wi, wi_world;
  Intersection *w_isect = new Intersection();

  double pdf = 1 / (2 * PI);
  for (int i = 0; i < num_samples; i++) {
    wi = hemisphereSampler->get_sample();
    wi_world = o2w * wi;

    Ray ray_sample = Ray(hit_p, wi_world, r.wavelength);
    ray_sample.min_t = EPS_F;
    if (bvh->intersect(ray_sample, w_isect)) {
      double emission = zero_bounce_radiance_channel(ray_sample, *w_isect);
      double bsdf_val = bsdf_value_for_wavelength(isect.bsdf, w_out, wi, r.wavelength);
      L_out += emission * bsdf_val * dot(isect.n, wi_world) / pdf / num_samples;
    }
  }

  return L_out;
}

double PathTracer::estimate_direct_lighting_importance_channel(const Ray &r,
                                                             const Intersection &isect) {
  // Importance sampling for specific wavelength channel
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);
  double L_out = 0.0;

  Vector3D wi_world;
  double distToLight, pdf, num_samples;
  Intersection *w_isect = new Intersection();

  for (auto light : scene->lights) {
    num_samples = 0.0;
    double L_light = 0.0;
    for (int i = 0; i < ns_area_light; i++) {
      Vector3D emitted_radiance = light->sample_L(hit_p, &wi_world, &distToLight, &pdf);
      num_samples++;

      Ray rl = Ray(hit_p, wi_world, r.wavelength);
      rl.min_t = EPS_F;
      rl.max_t = distToLight - EPS_F;
      if (!bvh->intersect(rl, w_isect)) {
        double bsdf_val = bsdf_value_for_wavelength(isect.bsdf, w_out, w2o * wi_world, r.wavelength);
        double emission_val = emitted_radiance.r; // Simplified - should be wavelength dependent
        L_light += emission_val * bsdf_val * max(0.0, dot(isect.n, wi_world)) / pdf;
      }
      if (light->is_delta_light()) break;
    }
    L_light /= num_samples;
    L_out += L_light;
  }
  
  return L_out;
}

} // namespace CGL
