#include "camera.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "CGL/misc.h"
#include "CGL/vector2D.h"
#include "CGL/vector3D.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::ifstream;
using std::ofstream;

namespace CGL {

using Collada::CameraInfo;

Ray Camera::generate_ray_for_thin_lens(double x, double y, double rndR, double rndTheta) const {

  // TODO Assignment 7: Part 4
  // compute position and direction of ray from the input sensor sample coordinate.
  // Note: use rndR and rndTheta to uniformly sample a unit disk.

  double x_film = (x - 0.5) * 2.0 * tan(radians(hFov) / 2.0);
  double y_film = (y - 0.5) * 2.0 * tan(radians(vFov) / 2.0);
  
  Vector3D dir_pinhole = Vector3D(x_film, y_film, -1);
  dir_pinhole.normalize();
  
  double t_focus = -focalDistance / dir_pinhole.z;
  Vector3D pFocus = dir_pinhole * t_focus;
  
  double r = lensRadius * sqrt(rndR);
  double x_lens = r * cos(rndTheta);
  double y_lens = r * sin(rndTheta);
  Vector3D pLens = Vector3D(x_lens, y_lens, 0);
  
  Vector3D dir = pFocus - pLens;
  dir.normalize();
  
  Vector3D world_origin = c2w * pLens + pos;
  Vector3D world_dir = c2w * dir;
  
  Ray ray = Ray(world_origin, world_dir);
  
  ray.min_t = nClip;
  ray.max_t = fClip;
  
  return ray;
}

Ray Camera::generate_ray_for_thin_lens(double x, double y, double rndR, double rndTheta, int color_channel) const {
  double x_film = (x - 0.5) * 2.0 * tan(radians(hFov) / 2.0);
  double y_film = (y - 0.5) * 2.0 * tan(radians(vFov) / 2.0);
  
  Vector3D dir_pinhole = Vector3D(x_film, y_film, -1);
  dir_pinhole.normalize();
  
  double t_focus = -focalDistance / dir_pinhole.z;
  Vector3D pFocus = dir_pinhole * t_focus;
  
  double r = lensRadius * sqrt(rndR);
  double x_lens = r * cos(rndTheta);
  double y_lens = r * sin(rndTheta);
  Vector3D pLens = Vector3D(x_lens, y_lens, 0);
  
  Vector3D dir = pFocus - pLens;
  dir.normalize();
  
  Vector3D world_origin = c2w * pLens + pos;
  Vector3D world_dir = c2w * dir;
  
  // Sample wavelength based on color channel (Gaussian distribution)
  double wavelength;
  switch (color_channel) {
    case 0: wavelength = 650.0; break; // Red
    case 1: wavelength = 550.0; break; // Green  
    case 2: wavelength = 450.0; break; // Blue
    default: wavelength = 550.0; break; // Default to green
  }
  
  Ray ray = Ray(world_origin, world_dir, fClip, 0, wavelength);
  ray.min_t = nClip;
  ray.max_t = fClip;
  
  return ray;
}


} // namespace CGL
