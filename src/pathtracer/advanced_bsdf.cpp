#include "bsdf.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "application/visual_debugger.h"

using std::max;
using std::min;
using std::swap;

// External global variable declaration
extern double current_wavelength;

namespace CGL {

// Mirror BSDF //

Vector3D MirrorBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D MirrorBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO Assignment 7: Part 1
  // Implement MirrorBSDF
  return Vector3D();
}

void MirrorBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Mirror BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    ImGui::TreePop();
  }
}

// Microfacet BSDF //

double MicrofacetBSDF::G(const Vector3D wo, const Vector3D wi) {
  return 1.0 / (1.0 + Lambda(wi) + Lambda(wo));
}

double MicrofacetBSDF::D(const Vector3D h) {
  // TODO Assignment 7: Part 2
  // Compute Beckmann normal distribution function (NDF) here.
  // You will need the roughness alpha.
  double cos_theta_h = h.z;
  
  if (cos_theta_h <= 0.0) return 0.0;
  
  double cos2_theta_h = cos_theta_h * cos_theta_h;
  double cos4_theta_h = cos2_theta_h * cos2_theta_h;
  double tan2_theta_h = (1.0 - cos2_theta_h) / cos2_theta_h;
  
  double exponent = -tan2_theta_h / (alpha * alpha);
  double numerator = exp(exponent);
  double denominator = PI * alpha * alpha * cos4_theta_h;
  
  return numerator / denominator;
}

Vector3D MicrofacetBSDF::F(const Vector3D wi) {
  // TODO Assignment 7: Part 2
  // Compute Fresnel term for reflection on dielectric-conductor interface.
  // You will need both eta and etaK, both of which are Vector3D.
  double cos_theta_i = wi.z;
  if (cos_theta_i <= 0.0) return Vector3D(0.0);
  
  double cos2_theta_i = cos_theta_i * cos_theta_i;
  
  Vector3D eta2 = eta * eta;
  Vector3D k2 = k * k;
  Vector3D eta2_plus_k2 = eta2 + k2;
  
  Vector3D Rs_numerator = eta2_plus_k2 - 2.0 * eta * cos_theta_i + Vector3D(cos2_theta_i);
  Vector3D Rs_denominator = eta2_plus_k2 + 2.0 * eta * cos_theta_i + Vector3D(cos2_theta_i);
  Vector3D Rs = Rs_numerator / Rs_denominator;
  
  Vector3D Rp_numerator = eta2_plus_k2 * Vector3D(cos2_theta_i) - 2.0 * eta * cos_theta_i + Vector3D(1.0);
  Vector3D Rp_denominator = eta2_plus_k2 * Vector3D(cos2_theta_i) + 2.0 * eta * cos_theta_i + Vector3D(1.0);
  Vector3D Rp = Rp_numerator / Rp_denominator;
  
  return (Rs + Rp) * 0.5;
}

Vector3D MicrofacetBSDF::f(const Vector3D wo, const Vector3D wi) {
  // TODO Assignment 7: Part 2
  // Implement microfacet model here.
   if (wo.z <= 0.0 || wi.z <= 0.0) {
    return Vector3D(0.0);
  }
  
  Vector3D h = (wo + wi).unit();
  
  Vector3D F_term = F(wi);
  double G_term = G(wo, wi);
  double D_term = D(h);
  
  double denominator = 4.0 * wo.z * wi.z;
  Vector3D result = (F_term * G_term * D_term) / denominator;
  
  return result;
}

Vector3D MicrofacetBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  // TODO Assignment 7: Part 2
  // *Importance* sample Beckmann normal distribution function (NDF) here.
  // Note: You should fill in the sampled direction *wi and the corresponding *pdf,
  //       and return the sampled BRDF value.

  Vector2D sample = sampler.get_sample();
  double r1 = sample.x;
  double r2 = sample.y;
  
  double theta_h = atan(sqrt(-alpha * alpha * log(1.0 - r1)));
  double phi_h = 2.0 * PI * r2;
  
  double sin_theta_h = sin(theta_h);
  double cos_theta_h = cos(theta_h);
  double sin_phi_h = sin(phi_h);
  double cos_phi_h = cos(phi_h);
  
  Vector3D h(
    sin_theta_h * cos_phi_h,
    sin_theta_h * sin_phi_h, 
    cos_theta_h
  );
  
  *wi = -wo + 2.0 * dot(wo, h) * h;
  wi->normalize();
  
  if (wi->z <= 0.0) {
    *pdf = 0.0;
    return Vector3D(0.0);
  }
  
  double p_theta = (2.0 * sin_theta_h) / (alpha * alpha * pow(cos_theta_h, 3)) * 
                   exp(-pow(tan(theta_h), 2) / (alpha * alpha));
  double p_phi = 1.0 / (2.0 * PI);
  double p_omega_h = (p_theta * p_phi) / sin_theta_h;
  
  *pdf = p_omega_h / (4.0 * dot(*wi, h));
  
  if (*pdf <= 0.0 || std::isinf(*pdf) || std::isnan(*pdf)) {
    *pdf = 0.0;
    return Vector3D(0.0);
  }
  
  return MicrofacetBSDF::f(wo, *wi);

  // *wi = cosineHemisphereSampler.get_sample(pdf);
  // return MicrofacetBSDF::f(wo, *wi);
}

void MicrofacetBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Micofacet BSDF"))
  {
    DragDouble3("eta", &eta[0], 0.005);
    DragDouble3("K", &k[0], 0.005);
    DragDouble("alpha", &alpha, 0.005);
    ImGui::TreePop();
  }
}

// Refraction BSDF //

Vector3D RefractionBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D RefractionBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  double wavelength_ior = get_ior(current_wavelength);
  
  // Calculate refracted direction using Snell's law
  double eta = 1.0 / wavelength_ior; // Air to material
  double cos_theta_i = wo.z; // wo points away from surface, so cos_theta_i = wo.z
  double sin2_theta_t = eta * eta * (1.0 - cos_theta_i * cos_theta_i);
  
  if (sin2_theta_t > 1.0) {
    // Total internal reflection
    *pdf = 1.0;
    return Vector3D();
  }
  
  double cos_theta_t = sqrt(1.0 - sin2_theta_t);
  
  // Refracted direction - correct calculation for transmitted ray
  *wi = Vector3D(eta * wo.x, eta * wo.y, -cos_theta_t);
  wi->normalize();
  
  *pdf = 1.0;
  
  // Simplified Fresnel transmission coefficient
  double r0 = pow((wavelength_ior - 1.0) / (wavelength_ior + 1.0), 2);
  double cos_theta = abs(cos_theta_i);
  double R = r0 + (1.0 - r0) * pow(1.0 - cos_theta, 5);
  double T = 1.0 - R;
  
  return transmittance * T;
}

void RefractionBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Refraction BSDF"))
  {
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("ior", &ior, 0.005);
    ImGui::TreePop();
  }
}

// Glass BSDF //

Vector3D GlassBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D GlassBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  double wavelength_ior = get_ior(current_wavelength);
  
  // Calculate reflection direction
  Vector3D reflected_wi = Vector3D(-wo.x, -wo.y, wo.z);
  
  // Calculate refraction using proper coordinate system
  double eta = 1.0 / wavelength_ior;
  double cos_theta_i = wo.z;
  double sin2_theta_t = eta * eta * (1.0 - cos_theta_i * cos_theta_i);
  
  bool total_internal_reflection = (sin2_theta_t > 1.0);
  
  Vector3D refracted_wi;
  if (!total_internal_reflection) {
    double cos_theta_t = sqrt(1.0 - sin2_theta_t);
    refracted_wi = Vector3D(eta * wo.x, eta * wo.y, -cos_theta_t);
    refracted_wi.normalize();
  }
  
  // Calculate Fresnel coefficients using Schlick's approximation
  double r0 = pow((wavelength_ior - 1.0) / (wavelength_ior + 1.0), 2);
  double cos_theta = abs(cos_theta_i);
  double R = r0 + (1.0 - r0) * pow(1.0 - cos_theta, 5);
  
  // Monte Carlo selection between reflection and refraction
  if (total_internal_reflection) {
    *wi = reflected_wi;
    *pdf = 1.0;
    return reflectance;
  } else if (rand() / double(RAND_MAX) < R) {
    // Reflection
    *wi = reflected_wi;
    *pdf = R;
    return reflectance;
  } else {
    // Refraction
    *wi = refracted_wi;
    *pdf = 1.0 - R;
    return transmittance;
  }
}

void GlassBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Refraction BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("ior", &ior, 0.005);
    ImGui::TreePop();
  }
}

void BSDF::reflect(const Vector3D wo, Vector3D* wi) {

  // TODO Assignment 7: Part 1
  // Implement reflection of wo about normal (0,0,1) and store result in wi.


}

bool BSDF::refract(const Vector3D wo, Vector3D* wi, double ior) {

  // TODO Assignment 7: Part 1
  // Use Snell's Law to refract wo surface and store result ray in wi.
  // Return false if refraction does not occur due to total internal reflection
  // and true otherwise. When dot(wo,n) is positive, then wo corresponds to a
  // ray entering the surface through vacuum.

  return true;

}

} // namespace CGL
