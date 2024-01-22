#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <functional>

// Function: transfer function
void transferFunction(const std::string &inputFile, 
                      const std::string &outputFile,
                      int width, 
                      int height) {

    // open the input file
    std::ifstream file(inputFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Unable to open file " << inputFile << std::endl;
        return;
    }

    // read the image data
    std::vector<unsigned char> image(width * height);
    file.read(reinterpret_cast<char*>(image.data()), image.size());
    file.close();


    // count the frequency of pixels for each grayscale value
    int frequency[256] = {0};
    for (unsigned char pixel : image) {
        frequency[pixel]++;
    }

    // calculate probability of each grayscale value
    int totalPixels = width * height;
    double probability[256] = {0};
    for (int i = 0; i < 256; ++i) {
        probability[i] = static_cast<double>(frequency[i]) / totalPixels;
    }

    // calculate cumulative probability
    double cumulativeProbability[256] = {0};
    cumulativeProbability[0] = probability[0];
    for (int i = 1; i < 256; ++i) {
        cumulativeProbability[i] = cumulativeProbability[i - 1] + probability[i];
    }

    // calculate transfer function
    int scale = 255;
    int mapping[256] = {0};
    for (int i = 0; i < 256; ++i) {
        int mappedValue = static_cast<int>(cumulativeProbability[i] * scale);
        mapping[i] = (mappedValue > 255) ? 255 : mappedValue;
    }

    // apply mapping to get enhanced image
    std::vector<unsigned char> enhancedImage(width * height);
    for (int i = 0; i < width * height; ++i) {
        enhancedImage[i] = mapping[image[i]];
    }

    // write the enhanced image to a file
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Unable to open file for enhanced image: " << outputFile << std::endl;
        return;
    }
    outFile.write(reinterpret_cast<char*>(enhancedImage.data()), enhancedImage.size());
    outFile.close();

}


// Function: bucket filling
void bucketFilling(const std::string &inputFile, 
                    const std::string &outputFile,
                    int width, 
                    int height) {
    // read the image data from file
    std::ifstream file(inputFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Unable to open file " << inputFile << std::endl;
        return;
    }
    std::vector<unsigned char> image(width * height);
    file.read(reinterpret_cast<char*>(image.data()), image.size());
    file.close();

    // calculate the histogram
    int histogram[256] = {0};
    for (unsigned char pixel : image) {
        histogram[pixel]++;
    }

    // total number of pixels
    int num_pixels = width * height;

    // bumber of buckets
    // for 8-bit grayscale images
    int num_buckets = 256; 

    // calculate the ideal number of pixels per bucket
    int pixels_per_bucket = num_pixels / num_buckets;

    // create the mapping for new pixel values
    std::vector<unsigned char> new_values(256);
    int accumulated_pixels = 0;
    int current_bucket = 0;

    // distribute pixels into new buckets
    for (int i = 0; i < 256; ++i) {
        accumulated_pixels += histogram[i];
        while (accumulated_pixels >= pixels_per_bucket && current_bucket < 255) {
            accumulated_pixels -= pixels_per_bucket;
            current_bucket++;
        }
        new_values[i] = current_bucket;
    }

    // apply the new values to the image
    for (unsigned char &pixel : image) {
        pixel = new_values[pixel];
    }

    // write the enhanced image to a file
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Unable to open file for enhanced image: " << outputFile << std::endl;
        return;
    }
    outFile.write(reinterpret_cast<char*>(image.data()), image.size());
    outFile.close();
}



int main() {
    // paths
    std::string inputFile = "./images/DimLight.raw";  // original image
    std::string outputFileTf = "./outputs/tfDimLight.raw"; // enhanced image by transfer function
    std::string outputFileBf = "./outputs/bfDimLight.raw"; // enhanced image by bucket filling

    int width = 596;
    int height = 340;

    // apply transfer function
    transferFunction(inputFile, outputFileTf, width, height);

    // apply bucket filling
    bucketFilling(inputFile, outputFileBf, width, height);

    
    return 0;
}
