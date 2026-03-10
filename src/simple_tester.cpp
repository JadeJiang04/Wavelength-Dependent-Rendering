#include <iostream>
#include <iomanip>
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323
#endif

// Simple test for wavelength-dependent functions
void test_wavelength_ior() {
    std::cout << "=== Wavelength-Dependent IOR Test ===" << std::endl;
    
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

void test_wavelength_rgb_mapping() {
    std::cout << "\n=== Wavelength to RGB Mapping Test ===" << std::endl;
    
    double wavelengths[] = {380.0, 450.0, 550.0, 650.0, 780.0};
    
    for (double wavelength : wavelengths) {
        double r, g, b;
        
        // Simple wavelength to RGB conversion
        if (wavelength < 380.0 || wavelength > 780.0) {
            r = g = b = 0.0;
        } else if (wavelength >= 380.0 && wavelength < 440.0) {
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
        
        std::cout << "Wavelength " << wavelength << " nm: RGB(" 
                 << std::fixed << std::setprecision(3) << r << ", " 
                 << g << ", " << b << ")" << std::endl;
    }
}

int main(int argc, char** argv) {
    std::cout << "=== Wavelength-Dependent Rendering Simple Tester ===" << std::endl;
    
    // Parse optional wavelength argument
    double custom_wavelength = 550.0; // Default to green
    if (argc > 1) {
        custom_wavelength = atof(argv[1]);
        std::cout << "Using custom wavelength: " << custom_wavelength << " nm" << std::endl;
    }
    
    test_wavelength_ior();
    test_black_body_radiation();
    test_thin_film_interference();
    test_wavelength_rgb_mapping();
    
    // Test custom wavelength if provided
    if (argc > 1) {
        std::cout << "\n=== Custom Wavelength Test ===" << std::endl;
        
        double wavelength_um = custom_wavelength / 1000.0;
        double B = 0.00420;
        double ior = 1.5 + B / (wavelength_um * wavelength_um);
        std::cout << "IOR at " << custom_wavelength << " nm: " << ior << std::endl;
        
        // Calculate interference for custom wavelength
        double thickness = 500.0;
        double path_diff = 2.0 * 1.33 * thickness;
        double phase_diff = 2.0 * PI * path_diff / custom_wavelength;
        double interference = 1.0 + cos(phase_diff);
        std::cout << "Interference at " << custom_wavelength << " nm: " << interference << std::endl;
    }
    
    std::cout << "\n=== All Tests Completed ===" << std::endl;
    return 0;
}