#include "pathtracer/camera.h"
#include "pathtracer/ray.h"
#include "CGL/vector3D.h"
#include <iostream>
#include <iomanip>

using namespace CGL;

void test_wavelength_sampling() {
    std::cout << "=== Wavelength Sampling Test ===" << std::endl;
    
    Camera cam;
    cam.configure(Collada::CameraInfo(), 800, 600);
    cam.place(Vector3D(0, 0, 0), PI/4, 0, 5, 0, 100);
    
    for (int channel = 0; channel < 3; channel++) {
        for (int i = 0; i < 5; i++) {
            double x = 0.5 + (i - 2) * 0.1;
            double y = 0.5;
            
            Ray r = cam.generate_ray(x, y, channel);
            std::cout << "Channel " << channel << " (" 
                     << (channel == 0 ? "Red" : channel == 1 ? "Green" : "Blue")
                     << ") at (" << x << ", " << y << "): "
                     << "wavelength = " << std::fixed << std::setprecision(1) 
                     << r.wavelength << " nm" << std::endl;
        }
    }
}

void test_wavelength_ior() {
    std::cout << "\n=== Wavelength-Dependent IOR Test ===" << std::endl;
    
    double base_ior = 1.5;
    double wavelengths[] = {450.0, 550.0, 650.0}; // Blue, Green, Red
    
    for (double wavelength : wavelengths) {
        double wavelength_um = wavelength / 1000.0; // Convert to micrometers
        double B = 0.00420; // Dispersion coefficient
        double ior = base_ior + B / (wavelength_um * wavelength_um);
        
        std::cout << "Wavelength " << wavelength << " nm: IOR = " 
                 << std::fixed << std::setprecision(4) << ior << std::endl;
    }
}

void test_black_body_radiation() {
    std::cout << "\n=== Black Body Radiation Test ===" << std::endl;
    
    double temperatures[] = {3000.0, 5500.0, 6500.0}; // Cool, Daylight, Hot white
    double wavelengths[] = {450.0, 550.0, 650.0};
    
    for (double temp : temperatures) {
        std::cout << "Temperature " << temp << "K:" << std::endl;
        for (double wavelength : wavelengths) {
            const double h = 6.626e-34;
            const double c = 3.0e8;
            const double k = 1.381e-23;
            const double wavelength_m = wavelength * 1e-9;
            
            double intensity = (2.0 * h * c * c) / (pow(wavelength_m, 5.0)) /
                            (exp((h * c) / (wavelength_m * k * temp)) - 1.0);
            intensity *= 1e-15;
            
            std::cout << "  " << wavelength << " nm: " << std::scientific 
                     << std::setprecision(2) << intensity << std::endl;
        }
    }
}

void test_thin_film_interference() {
    std::cout << "\n=== Thin Film Interference Test ===" << std::endl;
    
    double thickness = 500.0; // 500 nm film
    double ior_film = 1.33; // Water-like
    double cos_theta = 1.0; // Normal incidence
    double wavelengths[] = {400.0, 450.0, 500.0, 550.0, 600.0, 650.0, 700.0};
    
    std::cout << "Film thickness: " << thickness << " nm" << std::endl;
    std::cout << "Film IOR: " << ior_film << std::endl;
    std::cout << "Normal incidence (cos(theta) = " << cos_theta << ")" << std::endl;
    
    for (double wavelength : wavelengths) {
        double path_diff = 2.0 * ior_film * thickness * cos_theta;
        double phase_diff = 2.0 * PI * path_diff / wavelength;
        double interference = 1.0 + cos(phase_diff);
        
        std::cout << "  " << wavelength << " nm: interference = " 
                 << std::fixed << std::setprecision(3) << interference << std::endl;
    }
}

int main(int argc, char** argv) {
    std::cout << "=== Wavelength-Dependent Rendering Tester ===" << std::endl;
    
    // Parse optional wavelength argument
    double custom_wavelength = 550.0; // Default to green
    if (argc > 1) {
        custom_wavelength = atof(argv[1]);
        std::cout << "Using custom wavelength: " << custom_wavelength << " nm" << std::endl;
    }
    
    test_wavelength_sampling();
    test_wavelength_ior();
    test_black_body_radiation();
    test_thin_film_interference();
    
    // Test custom wavelength if provided
    if (argc > 1) {
        std::cout << "\n=== Custom Wavelength Test ===" << std::endl;
        Camera cam;
        cam.configure(Collada::CameraInfo(), 800, 600);
        cam.place(Vector3D(0, 0, 0), PI/4, 0, 5, 0, 100);
        
        Ray r = cam.generate_ray(0.5, 0.5);
        r.wavelength = custom_wavelength;
        
        std::cout << "Custom ray wavelength: " << r.wavelength << " nm" << std::endl;
        
        double wavelength_um = custom_wavelength / 1000.0;
        double B = 0.00420;
        double ior = 1.5 + B / (wavelength_um * wavelength_um);
        std::cout << "IOR at " << custom_wavelength << " nm: " << ior << std::endl;
    }
    
    std::cout << "\n=== All Tests Completed ===" << std::endl;
    return 0;
}