#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>

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

// Helper function: write RAW image data
void writeRawImage(const std::string& filename, 
                   const std::vector<unsigned char>& image_data) {
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(&image_data[0]), image_data.size() * sizeof(unsigned char));
}

// Helper function: to clamp pixel values
inline unsigned char clamp(int value, int low, int high) {
    return static_cast<unsigned char>(std::max(low, std::min(value, high)));
}

// Function: bilateral filter
void bilateralFilter(const std::vector<unsigned char>& flatImage,
                     std::vector<unsigned char>& filteredImage,
                     int filterSize,
                     double sigmaI,
                     double sigmaS) {
    // precompute Gaussian domain weights
    double twoSigmaS2 = 2.0 * sigmaS * sigmaS;
    double twoSigmaI2 = 2.0 * sigmaI * sigmaI;
    std::vector< std::vector<double> > gaussianDomain(filterSize, std::vector<double>(filterSize));
    int halfFilterSize = filterSize / 2;

    for (int i = -halfFilterSize; i <= halfFilterSize; ++i) {
        for (int j = -halfFilterSize; j <= halfFilterSize; ++j) {
            gaussianDomain[i + halfFilterSize][j + halfFilterSize] = exp(-(i * i + j * j) / twoSigmaS2);
        }
    }

    // apply the filter to each pixel
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            double sumWeights = 0.0;
            double sumFilteredPixel = 0.0;
            
            for (int fi = -halfFilterSize; fi <= halfFilterSize; ++fi) {
                for (int fj = -halfFilterSize; fj <= halfFilterSize; ++fj) {
                    int ni = i + fi;
                    int nj = j + fj;

                    // Mirror boundaries
                    ni = std::max(0, std::min(ni, HEIGHT - 1));
                    nj = std::max(0, std::min(nj, WIDTH - 1));

                    double rangeKernel = exp(-pow(static_cast<double>(flatImage[i * WIDTH + j]) - flatImage[ni * WIDTH + nj], 2) / twoSigmaI2);
                    double weight = gaussianDomain[fi + halfFilterSize][fj + halfFilterSize] * rangeKernel;

                    sumWeights += weight;
                    sumFilteredPixel += flatImage[ni * WIDTH + nj] * weight;
                }
            }

            filteredImage[i * WIDTH + j] = clamp(static_cast<int>(sumFilteredPixel / sumWeights), 0, 255);
        }
    }
}



int main() {
    std::string inputFilename = "./images/Flower_gray_noisy.raw";
    std::string bilateralOutputFilename = "./outputs/Flower_gray_bilateral.raw";

    std::vector<unsigned char> image_data = readRawImage(inputFilename);
    std::vector<unsigned char> bilateral_filtered_image(WIDTH * HEIGHT);

    int filterSize = 5; // 5x5 filter
    double sigmaI = 12.0; // Intensity sigma
    double sigmaS = 16.0; // Spatial sigma

    // Apply bilateral filter 
    bilateralFilter(image_data, bilateral_filtered_image, filterSize, sigmaI, sigmaS);

    // save the filtered images
    writeRawImage(bilateralOutputFilename, bilateral_filtered_image);

    std::cout << "Bilateral filtering completed." << std::endl;

    return 0;
}
