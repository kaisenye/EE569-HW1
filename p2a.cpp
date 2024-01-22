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

// Function: uniform weight filter to the image
void applyUniformFilter(const std::vector<unsigned char>& input, 
                        std::vector<unsigned char>& output, 
                        int kernelSize) {
    int offset = kernelSize / 2;
    for (int y = offset; y < HEIGHT - offset; ++y) {
        for (int x = offset; x < WIDTH - offset; ++x) {
            int sum = 0;
            for (int dy = -offset; dy <= offset; ++dy) {
                for (int dx = -offset; dx <= offset; ++dx) {
                    sum += input[(y + dy) * WIDTH + (x + dx)];
                }
            }
            output[y * WIDTH + x] = sum / (kernelSize * kernelSize);
        }
    }
}

// Helper function: calculate Gaussian weight
double gaussian(double x, double y, double sigma) {
    return exp(-(x * x + y * y) / (2 * sigma * sigma)) / (2 * M_PI * sigma * sigma);
}

// Function: Gaussian filter to the image
void applyGaussianFilter(const std::vector<unsigned char>& input, 
                         std::vector<unsigned char>& output, 
                         int kernelSize, 
                         double sigma) {
    int offset = kernelSize / 2;
    std::vector<double> kernel(kernelSize * kernelSize);
    double sumKernel = 0;

    // create the Gaussian kernel
    for (int y = -offset; y <= offset; ++y) {
        for (int x = -offset; x <= offset; ++x) {
            kernel[(y + offset) * kernelSize + (x + offset)] = gaussian(x, y, sigma);
            sumKernel += kernel[(y + offset) * kernelSize + (x + offset)];
        }
    }

    // normalize the kernel
    for (double &value : kernel) {
        value /= sumKernel;
    }

    // apply the Gaussian kernel to the image
    for (int y = offset; y < HEIGHT - offset; ++y) {
        for (int x = offset; x < WIDTH - offset; ++x) {
            double sum = 0;
            for (int dy = -offset; dy <= offset; ++dy) {
                for (int dx = -offset; dx <= offset; ++dx) {
                    sum += input[(y + dy) * WIDTH + (x + dx)] * kernel[(dy + offset) * kernelSize + (dx + offset)];
                }
            }
            output[y * WIDTH + x] = static_cast<unsigned char>(sum);
        }
    }
}


int main() {
    std::string inputFilename = "./images/Flower_gray_noisy.raw";
    std::string uniformOutputFilename = "./outputs/Flower_gray_uniform.raw";
    std::string gaussianOutputFilename = "./outputs/Flower_gray_gaussian.raw";

    std::vector<unsigned char> image_data = readRawImage(inputFilename);
    std::vector<unsigned char> uniform_filtered_image(WIDTH * HEIGHT);
    std::vector<unsigned char> gaussian_filtered_image(WIDTH * HEIGHT);

    // Apply uniform filter 
    // with a kernel size of 3
    applyUniformFilter(image_data, uniform_filtered_image, 3);

    // Apply Gaussian filter 
    // with a kernel size of 3 and sigma of 1.0
    applyGaussianFilter(image_data, gaussian_filtered_image, 3, 1.0);

    // save the filtered images
    writeRawImage(uniformOutputFilename, uniform_filtered_image);
    writeRawImage(gaussianOutputFilename, gaussian_filtered_image);

    std::cout << "Filtering completed." << std::endl;

    return 0;
}
