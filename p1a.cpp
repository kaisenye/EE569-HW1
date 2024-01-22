#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>


// Function to save a raw image
void saveRawImage(const std::string& filename, 
                  const std::vector<unsigned char>& imageData, 
                  int width, 
                  int height) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Could not open the file for writing: " << filename << std::endl;
        return;
    }

    // write the data to image. 
    outFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());

    if (!outFile.good()) {
        std::cerr << "Error occurred at writing time!" << std::endl;
    }

    outFile.close();
}

// Function to perform bilinear interpolation on a single channel
unsigned char bilinearInterpolate(const std::vector<unsigned char>& rawData, 
                                  int x, 
                                  int y, 
                                  int width, 
                                  int height, 
                                  int channelOffset, 
                                  int channelStep) {
    int sum = 0;
    int count = 0;

    for (int dy = -1; dy <= 1; dy += 2) {
        for (int dx = -1; dx <= 1; dx += 2) {
            int newX = x + dx;
            int newY = y + dy;
            if (newX >= 0 && newX < width && newY >= 0 && newY < height) {
                sum += rawData[(newY * width + newX) * 3 + channelOffset];
                count++;
            }
        }
    }

    return count > 0 ? static_cast<unsigned char>(sum / count) : 0;
}

// Function for bilinear demosaicing
void bilinearDemosaicing(const std::vector<unsigned char>& rawData, 
                         std::vector<unsigned char>& outputData, 
                         int width, int height) {

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3; // Output index for BGR image

            if ((y % 2 == 0) && (x % 2 == 0)) {
                // Current pixel is red
                outputData[index + 2] = rawData[y * width + x]; // Red channel
                outputData[index + 1] = bilinearInterpolate(rawData, x, y, width, height, 1, 3); // Green channel
                outputData[index] = bilinearInterpolate(rawData, x, y, width, height, 0, 3); // Blue channel
            } else if ((y % 2 == 1) && (x % 2 == 1)) {
                // Current pixel is blue
                outputData[index] = rawData[y * width + x]; // Blue channel
                outputData[index + 1] = bilinearInterpolate(rawData, x, y, width, height, 1, 3); // Green channel
                outputData[index + 2] = bilinearInterpolate(rawData, x, y, width, height, 2, 3); // Red channel
            } else {
                // Current pixel is green
                outputData[index + 1] = rawData[y * width + x]; // Green channel
            }

             // Bilinearly interpolate red and blue based on row parity
            if (y % 2 == 0) {
                outputData[index + 2] = bilinearInterpolate(rawData, x, y, width, height, 2, 3); // Red channel
                outputData[index] = (x > 0) ? rawData[y * width + (x - 1)] : rawData[y * width + (x + 1)]; // Blue channel
            } else {
                outputData[index] = bilinearInterpolate(rawData, x, y, width, height, 0, 3); // Blue channel
                outputData[index + 2] = (x > 0) ? rawData[y * width + (x - 1)] : rawData[y * width + (x + 1)]; // Red channel
            }
        }
    }

    // save the output data as a raw image file
    saveRawImage("./outputs/demosaicisedHouseImage.raw", outputData, width, height);
}



// Function to compare two images
void compareImages(const std::vector<unsigned char>& image1, 
                   const std::vector<unsigned char>& image2, 
                   int width, 
                   int height) {
    // threshold for significant difference
    const unsigned char threshold = 10; 
    int count = 0;

    for (size_t i = 0; i < width * height * 3; i += 3) {
        // Compute absolute difference for each channel
        unsigned char diffB = std::abs(image1[i] - image2[i]);
        unsigned char diffG = std::abs(image1[i + 1] - image2[i + 1]);
        unsigned char diffR = std::abs(image1[i + 2] - image2[i + 2]);

        if (diffB > threshold || diffG > threshold || diffR > threshold) {
            size_t pixelIdx = i / 3;
            std::cout << "Significant difference at pixel " << pixelIdx << 
            ", with B:" << static_cast<int>(diffB) <<
            ", G:" << static_cast<int>(diffG) <<
            ", R:" << static_cast<int>(diffR) << " intensity differences." << std::endl;
            count++;
        }
    }

    std::cout << "Total number of pixels with significant differences: " << count << std::endl;
}

// main
int main() {
    // initialize variables
    const std::string houseFilename = "./images/House.raw";
    const int width = 420;
    const int height = 288;

    // open the raw file
    std::ifstream file(houseFilename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return 1; // Error code
    }

    // read the raw data
    std::vector<unsigned char> rawData(width * height);
    file.read(reinterpret_cast<char*>(rawData.data()), rawData.size());
    file.close();

    // output data
    std::vector<unsigned char> outputImage(width * height * 3); // Output image (BGR)

    // perform bilinear demosaicing
    bilinearDemosaicing(rawData, outputImage, width, height);

    // convert House_ori image into vector
    const std::string houseOriFilename = "./images/House_ori.raw";
    std::ifstream file2(houseOriFilename, std::ios::binary);
    if (!file2) {
        std::cerr << "Error opening file!" << std::endl;
        return 1; // Error code
    }
    std::vector<unsigned char> rawData2(width * height);
    file2.read(reinterpret_cast<char*>(rawData2.data()), rawData2.size());
    file2.close();

    // compare the images
    compareImages(outputImage, rawData2, width, height);

    return 0;
}