#include <vector>
#include <iostream>
#include <fstream>
#include <string>


struct RGB {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct YUV {
    unsigned char y;
    unsigned char u;
    unsigned char v;
};

YUV rgbToYuv(const RGB& rgb) {
    YUV yuv;
    yuv.y = static_cast<unsigned char>(0.257 * rgb.r + 0.504 * rgb.g + 0.098 * rgb.b + 16);
    yuv.u = static_cast<unsigned char>(-0.148 * rgb.r - 0.291 * rgb.g + 0.439 * rgb.b + 128);
    yuv.v = static_cast<unsigned char>(0.439 * rgb.r - 0.368 * rgb.g - 0.071 * rgb.b + 128);
    return yuv;
}

RGB yuvToRgb(const YUV& yuv) {
    RGB rgb;
    rgb.r = static_cast<unsigned char>(yuv.y + 1.13983 * (yuv.v - 128));
    rgb.g = static_cast<unsigned char>(yuv.y - 0.39465 * (yuv.u - 128) - 0.58060 * (yuv.v - 128));
    rgb.b = static_cast<unsigned char>(yuv.y + 2.03211 * (yuv.u - 128));
    return rgb;
}


// Function: transform RGB to YUV
void transformRGBToYUV(std::vector<YUV>& yuvImage,
                       const std::string &inputFile,
                       int width,
                       int height) {
    // open the input file
    std::ifstream file(inputFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Unable to open file " << inputFile << std::endl;
        return;
    }

    // read the RGB data from the file
    std::vector<RGB> rgbImage(width * height);
    file.read(reinterpret_cast<char*>(rgbImage.data()), rgbImage.size() * sizeof(RGB));
    file.close();

    for (const auto& pixel : rgbImage) {
        yuvImage.push_back(rgbToYuv(pixel));
    }
}

// Function: transform YUV to RGB and store in a raw file
void transformYUVToRGB(const std::vector<YUV>& yuvImage, 
                      const std::string &outputFile) {
    
    // create a vector to store the RGB values
    std::vector<RGB> rgbImage;

    // transform YUV to RGB and store in rgbImage
    for (const auto& pixel : yuvImage) {
        rgbImage.push_back(yuvToRgb(pixel));
    }

    // store the RGB values in a output raw file
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Unable to open file for enhanced image: " << outputFile << std::endl;
        return;
    }
    outFile.write(reinterpret_cast<char*>(rgbImage.data()), rgbImage.size() * sizeof(RGB));
}


// Function: transfer function for Y channel ============================================================================
void transferFunctionYChannel(std::vector<YUV>& yuvImage, 
                              int width, 
                              int height) {
    // count the frequency of pixels for each grayscale value in the Y channel
    int frequency[256] = {0};
    for (const auto& pixel : yuvImage) {
        frequency[pixel.y]++;
    }

    // calculate probability of each grayscale value in the Y channel
    int totalPixels = width * height;
    double probability[256] = {0};
    for (int i = 0; i < 256; ++i) {
        probability[i] = static_cast<double>(frequency[i]) / totalPixels;
    }

    // calculate cumulative probability for the Y channel
    double cumulativeProbability[256] = {0};
    cumulativeProbability[0] = probability[0];
    for (int i = 1; i < 256; ++i) {
        cumulativeProbability[i] = cumulativeProbability[i - 1] + probability[i];
    }

    // calculate transfer function for the Y channel
    int scale = 255;
    int mapping[256] = {0};
    for (int i = 0; i < 256; ++i) {
        int mappedValue = static_cast<int>(cumulativeProbability[i] * scale);
        mapping[i] = (mappedValue > 255) ? 255 : mappedValue;
    }

    // apply mapping to get enhanced Y channel
    for (auto& pixel : yuvImage) {
        pixel.y = mapping[pixel.y];
    }
}


// Function: bucket filling for Y channel ==============================================================================
void bucketFillingYChannel(std::vector<YUV>& yuvImage, 
                           int width, 
                           int height) {
    // calculate the histogram for the Y channel
    int histogram[256] = {0};
    for (const auto& pixel : yuvImage) {
        histogram[pixel.y]++;
    }

    // total number of pixels
    int num_pixels = width * height;

    // number of buckets for 8-bit grayscale images
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

    // apply the new values to the Y channel
    for (auto& pixel : yuvImage) {
        pixel.y = new_values[pixel.y];
    }
}


// CLAHE ==============================================================================================================

// Sub-function: clip the histogram
void clipHistogram(std::vector<int>& histogram, int clipLimit) {
    int excess = 0;
    for (auto& h : histogram) {
        if (h > clipLimit) {
            excess += h - clipLimit;
            h = clipLimit;
        }
    }

    int increment = excess / histogram.size();
    int residual = excess % histogram.size();

    for (auto& h : histogram) {
        h += increment;
        if (residual > 0) {
            h++;
            residual--;
        }
    }
}

// Sub-function: perform histogram equalization on a tile
void equalizeHistogramTile(std::vector<YUV>& image, int width, int startX, int startY, int tileSizeX, int tileSizeY, int clipLimit) {
    int endX = std::min(startX + tileSizeX, width);
    int endY = std::min(startY + tileSizeY, static_cast<int>(image.size() / width));

    std::vector<int> histogram(256, 0);
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            histogram[image[y * width + x].y]++;
        }
    }

    if (clipLimit > 0) {
        clipHistogram(histogram, clipLimit);
    }

    // calculate CDF
    std::vector<int> cdf(256, 0);
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    // normalize the CDF
    int total = cdf[255];
    for (int i = 0; i < 256; ++i) {
        cdf[i] = (cdf[i] * 255) / total;
    }

    // apply the equalized histogram to the pixels
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            image[y * width + x].y = static_cast<unsigned char>(cdf[image[y * width + x].y]);
        }
    }
}

// Main CLAHE function with tile number
void applyCLAHE(std::vector<YUV>& image, int width, int height, int numTilesX, int numTilesY, int clipLimit) {
    // calculate tile size based on the number of tiles
    int tileSizeX = width / numTilesX;
    int tileSizeY = height / numTilesY;

    // apply histogram equalization to each tile
    for (int y = 0; y < height; y += tileSizeY) {
        for (int x = 0; x < width; x += tileSizeX) {
            equalizeHistogramTile(image, width, x, y, tileSizeX, tileSizeY, clipLimit);
        }
    }
}


int main() {
    // image dimensions
    int width = 750;  
    int height = 422; 

    // input file path
    std::string inputFile = "./images/City.raw";  

    // create a vector to store the YUV values
    std::vector<YUV> yuvImage;

    // transform RGB to YUV, and store in yuvImage
    transformRGBToYUV(yuvImage, inputFile, width, height);

    // apply transfer function to Y channel
    transferFunctionYChannel(yuvImage, width, height);

    // transform YUV to RGB, and store in rgbImage
    transformYUVToRGB(yuvImage, "./outputs/CityDefogged_TF.raw");

    // apply bucket filling to Y channel
    bucketFillingYChannel(yuvImage, width, height);

    // transform YUV to RGB, and store in rgbImage
    transformYUVToRGB(yuvImage, "./outputs/CityDefogged_BF.raw");

    // apply CLAHE
    int numTilesX = 4; // number of tiles in X direction
    int numTilesY = 4; // number of tiles in Y direction
    int clipLimit = 20; // contrast limit for histogram clipping
    applyCLAHE(yuvImage, width, height, numTilesX, numTilesY, clipLimit);

    // transform YUV to RGB, and store in rgbImage
    transformYUVToRGB(yuvImage, "./outputs/CityDefogged_CLAHE.raw");

    return 0;
}
