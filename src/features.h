#ifndef FEATURES_H
#define FEATURES_H


#include <iostream>
#include <vector>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

// Independent features

// Keep has to be lower or equal to extract
#define NB_MAJOR_COLORS_EXTRACT 7
#define NB_MAJOR_COLORS_KEEP 5

struct MajorColorElem
{
    Vec3b color;
    float position;
    int weightColor; // Nb of element of this color
};

// Global structs

struct FeaturesElement
{
    array<Mat, 3> histogramChannels;
    array<MajorColorElem, NB_MAJOR_COLORS_EXTRACT> majorColors;
};

class Features
{
public:
    static void computeFeature(const string &id, FeaturesElement &featuresElemOut);
    static void computeArray(float* &array, // Out: final array
                             size_t &sizeArray, // In: Size of config (offset) /Out: Total size
                             const vector<FeaturesElement> &listFeatures);

private:
    static void histRGB(const Mat &frame, const Mat &fgMask, array<Mat, 3> &histogramChannels);
    static void majorColors(const Mat &frame, const Mat &fgMask, array<MajorColorElem, NB_MAJOR_COLORS_EXTRACT> &listMajorColors);

    static unsigned int sizeElementArray;
};

#endif // FEATURES_H
