#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"
#include "color.hpp"

// decltype info comes from 
// https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
auto hash = [](const pixel& n) {
    //https://stackoverflow.com/questions/1646807/quick-and-simple-hash-code-combinations/1646913#1646913
    return ((17 * 31 + std::hash<int>()(n.r)) * 31 + std::hash<int>()(n.g)) * 31 + std::hash<int>()(n.b);
};
auto equal = [](const pixel& l, const pixel& r) {
    return l.r == r.r && l.g == r.g && l.b == r.b;
};
std::unordered_map<pixel, pixel, decltype(hash), decltype(equal)> pixelCache(8, hash, equal);

pixel findClosestVal(pixel val, int calc)
{
    std::vector<std::pair<float, pixel>> diff;

    for (pixel p : round1x1)
    {
        if (calc == 0) {
            diff.push_back({redMean(p, val), p});
        } else if (calc == 1) {
            diff.push_back({euclidMean(p, val), p});
        }
    }

    std::sort(diff.begin(), diff.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });

    return diff[0].second;
}

void resize(std::string infile, std::string outfile, int block)
{
    int width, height, channels;
    unsigned char* img = stbi_load(infile.c_str(), &width, &height, &channels, 3);
    if (img == NULL)
    {
        std::cout << "Failed to load image: " << stbi_failure_reason() << std::endl;
        return;
    }
    
    double aspect = width / (double)height;
    int n_width = block * aspect;
    int n_height = block;
    int n_size = n_width * n_height * channels;
    unsigned char *n_img = (unsigned char*) malloc(n_size);

    std::cout << "Old size: " << width << " x " << height << " x " << channels << std::endl;
    std::cout << "New size: " << n_width << " x " << n_height << " x " << channels << std::endl;


    stbir_resize_uint8_srgb(img, width, height, 0, n_img, n_width, n_height, 0, STBIR_RGB);
    stbir_resize_uint8_srgb(n_img, n_width, n_height, 0, img, width, height, 0, STBIR_RGB);

    int response = stbi_write_jpg(outfile.c_str(), n_width, n_height, channels, n_img, 100);
    if (response <= 0)
    {
        std::cout << "Failed to write image: " << stbi_failure_reason() << std::endl;
    }
    stbi_image_free(img);
}

void pixelate(std::string infile, std::string outfile, int block, int calc)
{
    int width, height, channels;
    unsigned char* img = stbi_load(infile.c_str(), &width, &height, &channels, 3);
    if (img == NULL)
    {
        std::cout << "Failed to load image: " << stbi_failure_reason() << std::endl;
        return;
    }

    double aspect = width / (double)height;
    int n_width = block * aspect;
    int n_height = block;
    int n_size = n_width * n_height * channels;
    unsigned char *n_img = (unsigned char*) malloc(n_size);

    stbir_resize_uint8_srgb(img, width, height, 0, n_img, n_width, n_height, 0, STBIR_RGB);
    int n = n_width * n_height * channels;
    for (int i = 0; i < n; i+=channels)
    {
        int r = (int)(n_img[i]);
        int g = (int)(n_img[i+1]);
        int b = (int)(n_img[i+2]);
        pixel current = {r, g, b};
        pixel nearest = pixelCache[current];
        if (nearest.r == 0 && nearest.g == 0 && nearest.b == 0)
        {
            nearest = findClosestVal(current, calc);
        }
        //std::cout << nearest.r << nearest.g << nearest.b << std::endl; 
        //std::stringstream ss;ss << std::hex << r << g << b;std::cout << ss.str() << std::endl;
        n_img[i] = (unsigned char)nearest.r;
        n_img[i+1] = (unsigned char)nearest.g;
        n_img[i+2] = (unsigned char)nearest.b;
    }

    int response = stbi_write_jpg(outfile.c_str(), n_width, n_height, channels, n_img, 100);
    if (response <= 0) std::cout << "Failed to write image: " << stbi_failure_reason() << std::endl;
    stbi_image_free(img);
}

int main(int argc, char *argv[])
{   
    if (argc < 3) {
        std::cout << "Second Arg must be int size of new picture." << std::endl;
        return -1;
    }

    if (argc < 4) {
        std::cout << "Third Arg must be 1 for resize or another int for resize and LEGOification." << std::endl;
        return -1;
    }

    std::string infile = argv[1];
    std::string outfile = infile.substr(0, infile.find(".")) + "-pixelated.jpg";
    int block_size = std::stoi(argv[2]);
    int r = std::stoi(argv[3]);

    if (r == 1) {
        resize(infile, outfile, block_size);
    } else {
        if (argc < 5) {
            std::cout << "Fourth Arg must be 1 for red mean or 0 for euclid mean." << std::endl;
            return -1;
        }
        int calc = std::stoi(argv[4]);
        if (calc == 1) {
            outfile = outfile.substr(0, outfile.find(".")) + "-red.jpg";
        } else {
            outfile = outfile.substr(0, outfile.find(".")) + "-euclid.jpg";
        }
        pixelate(infile, outfile, block_size, std::stoi(argv[4]));
    }

    return 0;
}
