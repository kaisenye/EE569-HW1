#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>

const int WIDTH = 768; // To be adjusted according to your image's width
const int HEIGHT = 512; // To be adjusted according to your image's height

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

// Helper function to clamp pixel values
inline unsigned char clamp(int value, int low, int high) {
    return static_cast<unsigned char>(std::max(low, std::min(value, high)));
}

// Helper function to calculate the median of a small array
template <size_t size>
unsigned char median(std::array<unsigned char, size> &values) {
    std::nth_element(values.begin(), values.begin() + size / 2, values.end());
    return values[size / 2];
}

// Function: median filter for RGB image
std::vector<unsigned char> applyMedianFilter(const std::vector<unsigned char>& image, int kernelSize) {
    std::vector<unsigned char> output(image.size());
    int edge = kernelSize / 2;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            std::array<std::array<unsigned char, 9>, 3> neighbors; // For each color channel

            // collect neighbors for R, G, B channels
            for (int dy = -edge; dy <= edge; ++dy) {
                for (int dx = -edge; dx <= edge; ++dx) {
                    int nx = std::min(std::max(x + dx, 0), WIDTH - 1);
                    int ny = std::min(std::max(y + dy, 0), HEIGHT - 1);
                    for (int channel = 0; channel < 3; ++channel) {
                        neighbors[channel][3 * (dy + edge) + (dx + edge)] = image[3 * (ny * WIDTH + nx) + channel];
                    }
                }
            }

            // compute median for each channel
            for (int channel = 0; channel < 3; ++channel) {
                output[3 * (y * WIDTH + x) + channel] = median(neighbors[channel]);
            }
        }
    }

    return output;
}

// Helper Function: Gaussian function
double gaussian(double x, double sigma) {
    return std::exp(-(x * x) / (2 * sigma * sigma)) / (std::sqrt(2 * M_PI) * sigma);
}

// Function: apply Gaussian filter for RGB image
std::vector<unsigned char> applyGaussianFilter(const std::vector<unsigned char>& image, int kernelSize, double sigma) {
    std::vector<unsigned char> output(image.size());
    int edge = kernelSize / 2;
    std::vector<double> kernel(kernelSize * kernelSize);
    double sum = 0.0;
    double twoSigmaSquare = 2.0 * sigma * sigma;

    // generate Gaussian kernel
    for (int i = -edge; i <= edge; ++i) {
        for (int j = -edge; j <= edge; ++j) {
            int index = (i + edge) * kernelSize + (j + edge);
            kernel[index] = gaussian(std::sqrt(i * i + j * j), sigma);
            sum += kernel[index];
        }
    }

    // normalize the kernel
    for (double &value : kernel) {
        value /= sum;
    }

    // apply Gaussian filter
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            for (int channel = 0; channel < 3; ++channel) {
                double weightedSum = 0.0;

                for (int dy = -edge; dy <= edge; ++dy) {
                    for (int dx = -edge; dx <= edge; ++dx) {
                        int nx = std::min(std::max(x + dx, 0), WIDTH - 1);
                        int ny = std::min(std::max(y + dy, 0), HEIGHT - 1);
                        int index = (dy + edge) * kernelSize + (dx + edge);
                        weightedSum += image[3 * (ny * WIDTH + nx) + channel] * kernel[index];
                    }
                }

                output[3 * (y * WIDTH + x) + channel] = clamp(static_cast<int>(weightedSum), 0, 255);
            }
        }
    }

    return output;
}

int main() {
    std::string inputFilename = "./images/Flower_noisy.raw";
    std::string outputFilename = "./outputs/Flower_color_filterd.raw";

    std::vector<unsigned char> inputImage = readRawImage(inputFilename);

    // Apply median filter
    int medianKernelSize = 5; 
    std::vector<unsigned char> medianFiltered = applyMedianFilter(inputImage, medianKernelSize);

    // Apply Gaussian filter
    int gaussianKernelSize = 5; 
    double gaussianSigma = 3; 
    std::vector<unsigned char> gaussianFiltered = applyGaussianFilter(medianFiltered, gaussianKernelSize, gaussianSigma);

    // save the filtered images
    writeRawImage(outputFilename, gaussianFiltered);

    return 0;
}