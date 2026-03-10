#include "light.h"

#include <iostream>

#include "pathtracer/sampler.h"

namespace CGL { namespace SceneObjects {

// Directional Light //

DirectionalLight::DirectionalLight(const Vector3D rad,
                                   const Vector3D lightDir)
    : radiance(rad), light_temperature(6500.0) {
  dirToLight = -lightDir.unit();
}

DirectionalLight::DirectionalLight(const Vector3D rad,
                                   const Vector3D lightDir, double temperature)
    : radiance(rad), light_temperature(temperature) {
  dirToLight = -lightDir.unit();
  calculate_radiance_from_temperature();
}

Vector3D DirectionalLight::sample_L(const Vector3D p, Vector3D* wi,
                                    double* distToLight, double* pdf) const {
  *wi = dirToLight;
  *distToLight = INF_D;
  *pdf = 1.0;
  return radiance;
}

// Infinite Hemisphere Light //

InfiniteHemisphereLight::InfiniteHemisphereLight(const Vector3D rad)
    : radiance(rad) {
  sampleToWorld[0] = Vector3D(1,  0,  0);
  sampleToWorld[1] = Vector3D(0,  0, -1);
  sampleToWorld[2] = Vector3D(0,  1,  0);
}

Vector3D InfiniteHemisphereLight::sample_L(const Vector3D p, Vector3D* wi,
                                           double* distToLight,
                                           double* pdf) const {
  Vector3D dir = sampler.get_sample();
  *wi = sampleToWorld* dir;
  *distToLight = INF_D;
  *pdf = 1.0 / (2.0 * PI);
  return radiance;
}

// Point Light //

PointLight::PointLight(const Vector3D rad, const Vector3D pos) : 
  radiance(rad), position(pos), light_temperature(6500.0) { }

PointLight::PointLight(const Vector3D rad, const Vector3D pos, double temperature) : 
  radiance(rad), position(pos), light_temperature(temperature) { 
  calculate_radiance_from_temperature();
}

Vector3D PointLight::sample_L(const Vector3D p, Vector3D* wi,
                             double* distToLight,
                             double* pdf) const {
  Vector3D d = position - p;
  *wi = d.unit();
  *distToLight = d.norm();
  *pdf = 1.0;
  return radiance;
}


// Spot Light //

SpotLight::SpotLight(const Vector3D rad, const Vector3D pos,
                     const Vector3D dir, double angle) {

}

Vector3D SpotLight::sample_L(const Vector3D p, Vector3D* wi,
                             double* distToLight, double* pdf) const {
  return Vector3D();
}


// Area Light //

AreaLight::AreaLight(const Vector3D rad, 
                     const Vector3D pos,   const Vector3D dir, 
                     const Vector3D dim_x, const Vector3D dim_y)
  : radiance(rad), position(pos), direction(dir),
    dim_x(dim_x), dim_y(dim_y), area(dim_x.norm() * dim_y.norm()) { }

Vector3D AreaLight::sample_L(const Vector3D p, Vector3D* wi, 
                             double* distToLight, double* pdf) const {

  Vector2D sample = sampler.get_sample() - Vector2D(0.5f, 0.5f);
  Vector3D d = position + sample.x * dim_x + sample.y * dim_y - p;
  double cosTheta = dot(d, direction);
  double sqDist = d.norm2();
  double dist = sqrt(sqDist);
  *wi = d / dist;
  *distToLight = dist;
  *pdf = sqDist / (area * fabs(cosTheta));
  return cosTheta < 0 ? radiance : Vector3D();
};


// Sphere Light //

SphereLight::SphereLight(const Vector3D rad, const SphereObject* sphere) {

}

Vector3D SphereLight::sample_L(const Vector3D p, Vector3D* wi, 
                               double* distToLight, double* pdf) const {

  return Vector3D();
}

// Mesh Light

MeshLight::MeshLight(const Vector3D rad, const Mesh* mesh) {

}

Vector3D MeshLight::sample_L(const Vector3D p, Vector3D* wi, 
                             double* distToLight, double* pdf) const {
  return Vector3D();
}

// Black body radiation implementation for DirectionalLight
double DirectionalLight::black_body_intensity(double temperature, double wavelength) const {
  // Wien's displacement law and Planck's law for black body radiation
  // Simplified model using wavelength in nm and temperature in Kelvin
  
  // Physical constants
  const double h = 6.626e-34; // Planck constant (J⋅s)
  const double c = 3.0e8;     // Speed of light (m/s)
  const double k = 1.381e-23; // Boltzmann constant (J/K)
  const double wavelength_m = wavelength * 1e-9; // Convert nm to m
  
  // Planck's law (simplified)
  double intensity = (2.0 * h * c * c) / (pow(wavelength_m, 5.0)) /
                    (exp((h * c) / (wavelength_m * k * temperature)) - 1.0);
  
  // Normalize to prevent very large values
  intensity *= 1e-15; // Scaling factor for practical use
  
  return intensity;
}

void DirectionalLight::calculate_radiance_from_temperature() {
  // Calculate RGB radiance from black body temperature
  double r_intensity = black_body_intensity(light_temperature, 650.0); // Red
  double g_intensity = black_body_intensity(light_temperature, 550.0); // Green
  double b_intensity = black_body_intensity(light_temperature, 450.0); // Blue
  
  // Normalize and scale
  double max_intensity = std::max(r_intensity, std::max(g_intensity, b_intensity));
  if (max_intensity > 0) {
    radiance = Vector3D(r_intensity / max_intensity, 
                       g_intensity / max_intensity, 
                       b_intensity / max_intensity);
  }
}

double DirectionalLight::sample_L_wavelength(const Vector3D p, Vector3D* wi,
                                         double* distToLight, double* pdf, 
                                         double wavelength) const {
  *wi = dirToLight;
  *distToLight = INF_D;
  *pdf = 1.0;
  return black_body_intensity(light_temperature, wavelength);
}

// Black body radiation implementation for PointLight
double PointLight::black_body_intensity(double temperature, double wavelength) const {
  // Same implementation as DirectionalLight
  const double h = 6.626e-34;
  const double c = 3.0e8;
  const double k = 1.381e-23;
  const double wavelength_m = wavelength * 1e-9;
  
  double intensity = (2.0 * h * c * c) / (pow(wavelength_m, 5.0)) /
                    (exp((h * c) / (wavelength_m * k * temperature)) - 1.0);
  intensity *= 1e-15;
  
  return intensity;
}

void PointLight::calculate_radiance_from_temperature() {
  double r_intensity = black_body_intensity(light_temperature, 650.0);
  double g_intensity = black_body_intensity(light_temperature, 550.0);
  double b_intensity = black_body_intensity(light_temperature, 450.0);
  
  double max_intensity = std::max(r_intensity, std::max(g_intensity, b_intensity));
  if (max_intensity > 0) {
    radiance = Vector3D(r_intensity / max_intensity, 
                       g_intensity / max_intensity, 
                       b_intensity / max_intensity);
  }
}

double PointLight::sample_L_wavelength(const Vector3D p, Vector3D* wi,
                                    double* distToLight, double* pdf, 
                                    double wavelength) const {
  Vector3D d = position - p;
  *wi = d.unit();
  *distToLight = d.norm();
  *pdf = 1.0;
  return black_body_intensity(light_temperature, wavelength);
}

} // namespace SceneObjects
} // namespace CGL
