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
    
    // read the contents of the file into a vector
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

// Helper function: calculate the median of a small array
template <size_t size>
unsigned char median(std::array<unsigned char, size> &values) {
    std::nth_element(values.begin(), values.begin() + size / 2, values.end());
    return values[size / 2];
}

// Function: median filter for RGB image
std::vector<unsigned char> applyMedianFilter(const std::vector<unsigned char>& image, 
                                             int kernelSize) {
    std::vector<unsigned char> output(image.size());
    int edge = kernelSize / 2;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            std::array<std::array<unsigned char, 9>, 3> neighbors; // for each color channel

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

// Helper function: Gaussian function for Bilateral filter
double gaussianBF(double x, double sigma) {
    return std::exp(-(x * x) / (2 * sigma * sigma));
}

// Bilateral filter function for an RGB image
std::vector<unsigned char> applyBilateralFilter(const std::vector<unsigned char>& image, 
                                                int kernelSize, 
                                                double sigmaColor, 
                                                double sigmaSpace) {

    std::vector<unsigned char> output(image.size());
    int edge = kernelSize / 2;

    // pre-compute Gaussian space weights
    std::vector<double> spaceWeights(kernelSize * kernelSize);
    for (int i = -edge; i <= edge; ++i) {
        for (int j = -edge; j <= edge; ++j) {
            spaceWeights[(i + edge) * kernelSize + (j + edge)] = gaussianBF(std::sqrt(i * i + j * j), sigmaSpace);
        }
    }

    // bilateral filter
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            for (int channel = 0; channel < 3; ++channel) {
                double iFiltered = 0;
                double wP = 0;
                unsigned char centerPixel = image[3 * (y * WIDTH + x) + channel];

                for (int dy = -edge; dy <= edge; ++dy) {
                    for (int dx = -edge; dx <= edge; ++dx) {
                        int nx = std::min(std::max(x + dx, 0), WIDTH - 1);
                        int ny = std::min(std::max(y + dy, 0), HEIGHT - 1);
                        unsigned char neighborPixel = image[3 * (ny * WIDTH + nx) + channel];

                        double w = spaceWeights[(dy + edge) * kernelSize + (dx + edge)] * gaussianBF(centerPixel - neighborPixel, sigmaColor);
                        iFiltered += neighborPixel * w;
                        wP += w;
                    }
                }

                output[3 * (y * WIDTH + x) + channel] = static_cast<unsigned char>(iFiltered / wP);
            }
        }
    }
    
    return output;
}

// Helper Function: Gaussian function
double gaussian(double x, double sigma) {
    return std::exp(-(x * x) / (2 * sigma * sigma)) / (std::sqrt(2 * M_PI) * sigma);
}

// Helper function: to clamp pixel values
inline unsigned char clamp(int value, int low, int high) {
    return static_cast<unsigned char>(std::max(low, std::min(value, high)));
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
    for (size_t i = 0; i < kernel.size(); ++i) {
        double &value = kernel[i];
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

// Helper function: linear combination of two images
std::vector<unsigned char> linearCombine(const std::vector<unsigned char>& bilateralImage,
                                         const std::vector<unsigned char>& gaussianImage,
                                         double alpha, 
                                         double beta) {
    std::vector<unsigned char> output(bilateralImage.size());
    
    for (size_t i = 0; i < bilateralImage.size(); ++i) {
        // Apply the linear combination formula: output = alpha * bilateral + beta * gaussian
        int combinedValue = static_cast<int>(alpha * bilateralImage[i] - beta * gaussianImage[i]);
        output[i] = clamp(combinedValue, 0, 255);
    }

    return output;
}


int main() {
    std::string inputFilename = "./images/Flower_noisy.raw";
    std::string medianFilterdFilename = "./outputs/Flower_median_filtered.raw";
    std::string waterColoredFilename = "./outputs/Flower_water_colored.raw";

    std::vector<unsigned char> inputImage = readRawImage(inputFilename);

    // Apply median filter
    int medianKernelSize = 3; 
    std::vector<unsigned char> medianFiltered = applyMedianFilter(inputImage, medianKernelSize);

    // save the median filtered image
    writeRawImage(medianFilterdFilename, medianFiltered);

    // Apply bilateral filter
    int bilateralKernelSize = 5; 
    double sigmaColor = 20.0; 
    double sigmaSpace = 10.0;
    int K = 10;
    std::vector<unsigned char> bilateralFiltered = applyBilateralFilter(medianFiltered, bilateralKernelSize, sigmaColor, sigmaSpace);
    for (int i = 1; i < K; ++i) {
        bilateralFiltered = applyBilateralFilter(bilateralFiltered, bilateralKernelSize, sigmaColor, sigmaSpace);
    }

    // Apply Gaussian filter
    int gaussianKernelSize = 7; 
    double gaussianSigma = 2; 
    std::vector<unsigned char> gaussianFiltered = applyGaussianFilter(inputImage, gaussianKernelSize, gaussianSigma);

    // combine the two filtered images
    double alpha = 1.4;
    double beta = 0.4;
    std::vector<unsigned char> combinedImage = linearCombine(bilateralFiltered, gaussianFiltered, alpha, beta);

    // save the fianl combined image
    writeRawImage(waterColoredFilename, combinedImage);

    return 0;
}