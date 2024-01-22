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

// Helper function: clamp pixel values
inline unsigned char clamp(int value, int low, int high) {
    return static_cast<unsigned char>(std::max(low, std::min(value, high)));
}

// Helper function: compute Gaussian weight
inline double gaussian(double x, double sigma) {
    return std::exp(-(x * x) / (2 * sigma * sigma)) / (std::sqrt(2 * M_PI) * sigma);
}

// Function to apply the Non-Local Means filter
void nonLocalMeansFilter(const std::vector<unsigned char>& image,
                         std::vector<unsigned char>& result,
                         int patchSize,
                         int windowSize,
                         double h,
                         double sigma) {
                         
    const int halfPatchSize = patchSize / 2;
    const int halfWindowSize = windowSize / 2;
    
    std::vector<std::vector<double> > weights(windowSize, std::vector<double>(windowSize));

    // precompute Gaussian weights
    for (int i = -halfWindowSize; i <= halfWindowSize; ++i) {
        for (int j = -halfWindowSize; j <= halfWindowSize; ++j) {
            weights[i + halfWindowSize][j + halfWindowSize] = gaussian(std::sqrt(i * i + j * j), sigma);
        }
    }

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            double weightSum = 0.0;
            double pixelValue = 0.0;

            for (int wi = -halfWindowSize; wi <= halfWindowSize; ++wi) {
                for (int wj = -halfWindowSize; wj <= halfWindowSize; ++wj) {
                    double patchDistance = 0.0;

                    for (int pi = -halfPatchSize; pi <= halfPatchSize; ++pi) {
                        for (int pj = -halfPatchSize; pj <= halfPatchSize; ++pj) {
                            int refI = std::max(0, std::min(i + pi, HEIGHT - 1));
                            int refJ = std::max(0, std::min(j + pj, WIDTH - 1));
                            int winI = std::max(0, std::min(i + wi + pi, HEIGHT - 1));
                            int winJ = std::max(0, std::min(j + wj + pj, WIDTH - 1));
                            
                            patchDistance += (image[refI * WIDTH + refJ] - image[winI * WIDTH + winJ]) *
                                             (image[refI * WIDTH + refJ] - image[winI * WIDTH + winJ]);
                        }
                    }

                    double w = std::exp(-patchDistance / (h * h)) * weights[wi + halfWindowSize][wj + halfWindowSize];
                    weightSum += w;
                    pixelValue += w * image[(i + wi) * WIDTH + (j + wj)];
                }
            }

            result[i * WIDTH + j] = clamp(static_cast<int>(pixelValue / weightSum), 0, 255);
        }
    }
}

int main() {
    std::string inputFilename = "./images/Flower_gray_noisy.raw";
    std::string nlmOutputFilename = "./outputs/Flower_gray_nlm.raw";

    std::vector<unsigned char> image_data = readRawImage(inputFilename);
    std::vector<unsigned char> nlm_filtered_image(WIDTH * HEIGHT);

    // filter parameters
    int patchSize = 10; // Patch size for the local neighborhood
    int windowSize = 21; // Window size for searching similar patches
    double h = 16.0; // Filtering parameter, controls decay of the weights, depends on the noise level
    double sigma = 10.0; // Standard deviation for Gaussian function

    // Apply NLM filter 
    nonLocalMeansFilter(image_data, nlm_filtered_image, patchSize, windowSize, h, sigma);

    // save the filtered images
    writeRawImage(nlmOutputFilename, nlm_filtered_image);

    std::cout << "NLM filtering completed." << std::endl;

    return 0;
}