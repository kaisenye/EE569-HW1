#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <fstream>
#include <iostream>

// image dimensions
const int WIDTH = 768; 
const int HEIGHT = 512; 

// Helper function: read RAW image data
std::vector<unsigned char> readRawImage(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open the file: " << filename << std::endl;
        return std::vector<unsigned char>();
    }
    
    // Read the contents of the file into a vector
    std::vector<unsigned char> buffer(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    return buffer;
}

// Helper function: calculate MSE
double calculateMSE(const std::vector<unsigned char>& original, 
                    const std::vector<unsigned char>& denoised, 
                    int width, 
                    int height) {
    if (original.size() != denoised.size()) {
        throw std::invalid_argument("Images must have the same size for MSE calculation.");
    }

    double mse = 0.0;
    for (size_t i = 0; i < original.size(); ++i) {
        mse += std::pow(original[i] - denoised[i], 2);
    }
    mse /= (width * height);
    return mse;
}

// Function: calculate PSNR
double calculatePSNR(const std::vector<unsigned char>& original, 
                     const std::vector<unsigned char>& denoised, 
                     int width, 
                     int height) {
    double mse = calculateMSE(original, denoised, width, height);
    if (mse == 0) {
        return std::numeric_limits<double>::infinity();
    }
    double max_pixel_value = 255.0; // For 8-bit images
    double psnr = 10 * std::log10((max_pixel_value * max_pixel_value) / mse);
    return psnr;
}

int main() {
    std::string originalImageFilename = "./images/Flower_gray.raw";

    std::string guassianFilename = "./outputs/Flower_gray_gaussian.raw";
    std::string uniformFilename = "./outputs/Flower_gray_uniform.raw";
    std::string bilateralFilename = "./outputs/Flower_gray_bilateral.raw";
    std::string nlmFilename = "./outputs/Flower_gray_nlm.raw";

    // Read the original and denoised images into vectors
    std::vector<unsigned char> originalImage = readRawImage(originalImageFilename);
    std::vector<unsigned char> guassianImage = readRawImage(guassianFilename);
    std::vector<unsigned char> uniformImage = readRawImage(uniformFilename);
    std::vector<unsigned char> bilateralImage = readRawImage(bilateralFilename);
    std::vector<unsigned char> nlmImage = readRawImage(nlmFilename);

    // Calculate PSNR of Gaussian and uniform denoised images
    double guassianPSNR = calculatePSNR(originalImage, guassianImage, WIDTH, HEIGHT);
    std::cout << "PSNR of the Gaussian denoised image: " << guassianPSNR << " dB" << std::endl;
    
    double unifornPSNR = calculatePSNR(originalImage, uniformImage, WIDTH, HEIGHT);
    std::cout << "PSNR of the Uniform denoised image: " << unifornPSNR << " dB" << std::endl;

    double bilateralPSNR = calculatePSNR(originalImage, bilateralImage, WIDTH, HEIGHT);
    std::cout << "PSNR of the Bilateral denoised image: " << bilateralPSNR << " dB" << std::endl;

    double nlmPSNR = calculatePSNR(originalImage, nlmImage, WIDTH, HEIGHT);
    std::cout << "PSNR of the NLM denoised image: " << nlmPSNR << " dB" << std::endl;

    return 0;
}