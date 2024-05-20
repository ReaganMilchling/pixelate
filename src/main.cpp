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

typedef struct {
    int r;
    int g;
    int b;
} pixel;

std::string toHex(int n)
{
    std::string hex = "0123456789ABCDEF";
    std::string ret = "";
    while (n) {
        ret.push_back(hex.find(n/16));
        n/=16;
    }
    return ret;
}

float redMean(pixel p1, pixel p2) 
{
    int red = (p1.r + p2.r) / 2;
    float delta = (2 + (red / 256.0)) * pow((p2.r-p1.r), 2.0) 
            + 4 * pow((p2.g-p1.g), 2.0) 
            + (2 + ((255-red)/256.0)) * pow((p2.b-p1.b), 2.0);
    return delta;
}

float euclidMean(pixel p1, pixel p2) 
{
    return pow((p2.r-p1.r), 2.0) 
           + pow((p2.g-p1.g), 2.0) 
           + pow((p2.b-p1.b), 2.0);
}


std::vector<pixel> round1x1 = {
    {0,143,155},
    {0,85,191},
    {35,120,65},
    {201,26,9},
    {200,112,160},
    {109,110,92},
    {75,159,74},
    {242,205,55},
    {165,169,180},
    {255,255,255},
    {228,205,158},
    {228,173,200},
    {0,32,160},
    {132,182,141},
    {201,26,9},
    {99,95,82},
    {174,239,236},
    {223,102,149},
    {252,252,252},
    {165,165,203},
    {255,128,13},
    {88,42,18},
    {160,165,169},
    {88,57,39},
    {247,133,177},
    {108,110,104},
    {165,169,180},
    {165,169,180},
    {63,54,145},
    {156,163,168},
    {207,226,247},
    {207,226,247},
    {223,238,165},
    {240,143,28},
    {187,233,11},
    {189,198,173},
    {170,127,46},
    {114,14,15},
    {54,174,191},
    {217,217,217},
    {170,125,85},
    {173,195,192},
    {200,112,160},
    {155,161,157},
    {201,26,9},
    {252,252,252},
    {174,239,236},
    {132,182,141},
    {223,102,149},
    {248,241,132},
    {255,128,13},
    {217,228,167},
    {0,32,160},
    {240,143,28},
    {35,120,65},
    {207,226,247},
    {224,224,224},
    {137,135,136},
    {155,154,90},
    {146,57,120},
    {217,228,167},
    {248,241,132},
    {109,110,92},
    {169,85,0},
    {96,116,161},
    {149,138,115},
    {10,52,99},
    {53,33,0},
    {255,240,58},
    {88,57,39},
    {35,120,65},
    {252,252,252},
    {10,52,99},
    {228,205,158},
    {0,32,160},
    {201,26,9},
    {245,205,47},
    {217,217,217},
    {132,182,141},
    {174,239,236},
    {255,128,13},
    {174,239,236},
    {248,241,132},
    {212,213,201},
    {5,19,29},
    {172,120,186},
    {208,145,104},
    {245,205,47},
    {245,205,47},
    {114,14,15},
    {254,138,24},
    {165,165,203},
    {35,120,65},
    {248,187,61},
    {217,217,217},
    {62,60,57},
    {218,176,0},
    {223,102,149},
    {88,57,39},
    {90,196,218},
    {252,252,252},
    {248,187,61},
    {165,169,180},
    {160,188,172},
    {187,165,61},
    {90,147,219},
    {100,90,76},
    {174,239,236},
    {160,188,172},
};

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
            outfile = outfile.substr(0, infile.find(".")) + "-red.jpg";
        } else {
            outfile = outfile.substr(0, infile.find(".")) + "-euclid.jpg";
        }
        pixelate(infile, outfile, block_size, std::stoi(argv[4]));
    }

    return 0;
}
