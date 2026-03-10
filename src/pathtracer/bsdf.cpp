#include "bsdf.h"
#include "application/visual_debugger.h"

#include <algorithm>
#include <iostream>
#include <utility>

#ifndef PI
#define PI 3.14159265358979323
#endif



using std::max;
using std::min;
using std::swap;

namespace CGL {

/**
 * This function creates a object space (basis vectors) from the normal vector
 */
void make_coord_space(Matrix3x3 &o2w, const Vector3D n) {

  Vector3D z = Vector3D(n.x, n.y, n.z);
  Vector3D h = z;
  if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z))
    h.x = 1.0;
  else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z))
    h.y = 1.0;
  else
    h.z = 1.0;

  z.normalize();
  Vector3D y = cross(h, z);
  y.normalize();
  Vector3D x = cross(z, y);
  x.normalize();

  o2w[0] = x;
  o2w[1] = y;
  o2w[2] = z;
}

/**
 * Evaluate diffuse lambertian BSDF.
 * Given incident light direction wi and outgoing light direction wo. Note
 * that both wi and wo are defined in the local coordinate system at the
 * point of intersection.
 * \param wo outgoing light direction in local space of point of intersection
 * \param wi incident light direction in local space of point of intersection
 * \return reflectance in the given incident/outgoing directions
 */
Vector3D DiffuseBSDF::f(const Vector3D wo, const Vector3D wi) {
  // TODO (Part 3.1):
  // This function takes in both wo and wi and returns the evaluation of
  // the BSDF for those two directions.

  return reflectance / PI;

}

/**
 * Evalutate diffuse lambertian BSDF.
 */
Vector3D DiffuseBSDF::sample_f(const Vector3D wo, Vector3D *wi, double *pdf) {
  // TODO (Part 3.1):
  // This function takes in only wo and provides pointers for wi and pdf,
  // which should be assigned by this function.
  // After sampling a value for wi, it returns the evaluation of the BSDF
  // at (wo, *wi).
  // You can use the `f` function. The reference solution only takes two lines.

  *wi = sampler.get_sample(pdf);
  return f(wo, *wi);

}

void DiffuseBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Diffuse BSDF"))
  {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    ImGui::TreePop();
  }
}

/**
 * Evalutate Emission BSDF (Light Source)
 */
Vector3D EmissionBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

/**
 * Evalutate Emission BSDF (Light Source)
 */
Vector3D EmissionBSDF::sample_f(const Vector3D wo, Vector3D *wi, double *pdf) {
  *pdf = 1.0 / PI;
  *wi = sampler.get_sample(pdf);
  return Vector3D();
}

void EmissionBSDF::render_debugger_node()
{
  if (ImGui::TreeNode(this, "Emission BSDF"))
  {
    DragDouble3("Radiance", &radiance[0], 0.005);
    ImGui::TreePop();
  }
}

// Wavelength-dependent IOR implementations
double RefractionBSDF::get_ior(double wavelength) const {
  // Cauchy's equation for wavelength-dependent IOR
  // n(λ) = n₀ + B/λ² where λ is in micrometers
  double wavelength_um = wavelength / 1000.0; // Convert nm to micrometers
  double B = 0.00420; // Dispersion coefficient for typical glass
  return ior + B / (wavelength_um * wavelength_um);
}

double GlassBSDF::get_ior(double wavelength) const {
  // Cauchy's equation for wavelength-dependent IOR
  double wavelength_um = wavelength / 1000.0; // Convert nm to micrometers
  double B = 0.00420; // Dispersion coefficient for typical glass
  return ior + B / (wavelength_um * wavelength_um);
}

// ThinFilmBSDF implementation
Vector3D ThinFilmBSDF::f(const Vector3D wo, const Vector3D wi) {
  // Simplified implementation - in full implementation this would
  // calculate proper interference patterns
  return reflectance + transmittance;
}

Vector3D ThinFilmBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  *wi = sampler.get_sample(pdf);
  return f(wo, *wi);
}

double ThinFilmBSDF::get_ior_film(double wavelength) const {
  double wavelength_um = wavelength / 1000.0;
  double B = 0.00420;
  return ior_film + B / (wavelength_um * wavelength_um);
}

double ThinFilmBSDF::get_ior_substrate(double wavelength) const {
  double wavelength_um = wavelength / 1000.0;
  double B = 0.00420;
  return ior_substrate + B / (wavelength_um * wavelength_um);
}

double ThinFilmBSDF::interference_intensity(double wavelength, double cos_theta) const {
  // Thin film interference calculation
  // Path difference: 2 * n_film * thickness * cos(theta)
  // Constructive interference when path difference = m * wavelength
  
  double n_film = get_ior_film(wavelength);
  double path_diff = 2.0 * n_film * thickness * cos_theta;
  
  // Phase difference
  double phase_diff = 2.0 * PI * path_diff / wavelength;
  
  // Interference intensity (simplified)
  double interference = 1.0 + cos(phase_diff);
  
  // Add wavelength-dependent effects
  double wavelength_factor = exp(-pow((wavelength - 550.0) / 200.0, 2.0));
  
  return interference * wavelength_factor;
}

void ThinFilmBSDF::render_debugger_node() {
  if (ImGui::TreeNode(this, "Thin Film BSDF")) {
    DragDouble3("Reflectance", &reflectance[0], 0.005);
    DragDouble3("Transmittance", &transmittance[0], 0.005);
    DragDouble("Thickness (nm)", &thickness, 1.0);
    DragDouble("Film IOR", &ior_film, 0.01);
    DragDouble("Substrate IOR", &ior_substrate, 0.01);
    ImGui::TreePop();
  }
}

} // namespace CGL
