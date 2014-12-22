#include "features.h"

#include <algorithm>
#include <array>


// Sort color elements
bool sortMajorColors(MajorColorElem elem1, MajorColorElem elem2)
{
    return elem1.weightColor > elem2.weightColor;
}

void Features::computeFeature(const string &id, FeaturesElement &featuresElemOut)
{
    // Read images
    Mat imgPers     = imread(id + ".png");
    Mat imgMaskPers = imread(id + "_mask.png");


    if (imgPers.empty() || imgMaskPers.empty())
    {
        cout << "Error: cannot loading images : " << endl;
        cout << id << endl;

        exit(0);
    }

    cvtColor(imgMaskPers, imgMaskPers, CV_BGR2GRAY);
    threshold(imgMaskPers, imgMaskPers, 254, 255, THRESH_BINARY);

    histRGB(imgPers, imgMaskPers, featuresElemOut.histogramChannels);
    majorColors(imgPers, imgMaskPers, featuresElemOut.majorColors);
}

void Features::histRGB(const Mat &frame, const Mat &fgMask, array<Mat, 3> &histogramChannels)
{
    // Conversion to the right color space ???
    // Size of the histogram
    int histSize = HIST_SIZE; // bin size
    float range[] = {0, 256}; // min max values
    const float *ranges[] = {range};
    // Extraction of the histograms
    std::vector<cv::Mat> sourceChannels;
    cv::split(frame, sourceChannels);
    cv::calcHist(&sourceChannels[0], 1, 0, fgMask, histogramChannels[0], 1, &histSize, ranges, true, false);
    cv::calcHist(&sourceChannels[1], 1, 0, fgMask, histogramChannels[1], 1, &histSize, ranges, true, false);
    cv::calcHist(&sourceChannels[2], 1, 0, fgMask, histogramChannels[2], 1, &histSize, ranges, true, false);
    // Normalize
    normalize(histogramChannels[0], histogramChannels[0]);
    normalize(histogramChannels[1], histogramChannels[1]);
    normalize(histogramChannels[2], histogramChannels[2]);
}

void Features::majorColors(const Mat &frame, const Mat &fgMask, array<MajorColorElem, NB_MAJOR_COLORS_EXTRACT> &listMajorColors)
{
    Mat src = frame.clone();

    // Step 1: Map the src to the samples
    Mat samples(cv::countNonZero(fgMask), 3, CV_32F); // We only cluster the "white" pixels

    int i = 0;
    for (int x = 0 ; x < fgMask.rows ; ++x)
    {
        for (int y = 0; y < fgMask.cols ; ++y)
        {
            if(fgMask.at<uchar>(x,y))
            {
                samples.at<float>(i,0) = src.at<Vec3b>(x,y)[0];
                samples.at<float>(i,1) = src.at<Vec3b>(x,y)[1];
                samples.at<float>(i,2) = src.at<Vec3b>(x,y)[2];
                ++i;
            }
        }
    }

    // Step 2: Apply kmeans to find labels and centers
    int clusterCount = NB_MAJOR_COLORS_EXTRACT;
    cv::Mat labels;
    int attempts = 5;
    cv::Mat centers;
    cv::kmeans(samples, clusterCount, labels,
               cv::TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS,
                                10, 0.01),
               attempts, cv::KMEANS_PP_CENTERS, centers);


    // Step 3: Fill information
    for(i = 0 ; i < centers.rows ; ++i)
    {
        listMajorColors.at(i).color = Vec3b(centers.at<float>(i, 0),
                                            centers.at<float>(i, 1),
                                            centers.at<float>(i, 2));

        // Conversion to a good color space (for distance computation)
        Mat imgToConvert(1, 1, CV_8UC3, Scalar(listMajorColors.at(i).color));

        cvtColor(imgToConvert, imgToConvert, CV_BGR2Lab);

        listMajorColors.at(i).color = imgToConvert.at<Vec3b>(0,0);

        // Add number of pixel of each major color
        listMajorColors.at(i).weightColor = 0;

        // TODO: Add Spacial information

    }

    // Step 4: Map the centers to the output
    i = 0;
    Mat dest(src.size(), src.type());
    for (int x = 0 ; x < dest.rows ; ++x)
    {
        for (int y = 0 ; y < dest.cols ; ++y)
        {
            if(fgMask.at<uchar>(x,y))
            {
                int cluster_idx = labels.at<int>(i,0);
                listMajorColors.at(cluster_idx).weightColor++ ;
                dest.at<Vec3b>(x,y)[0] = centers.at<float>(cluster_idx, 0);
                dest.at<Vec3b>(x,y)[1] = centers.at<float>(cluster_idx, 1);
                dest.at<Vec3b>(x,y)[2] = centers.at<float>(cluster_idx, 2);
                ++i;
            }
            else
            {
                dest.at<Vec3b>(x,y)[0] = 0;
                dest.at<Vec3b>(x,y)[1] = 0;
                dest.at<Vec3b>(x,y)[2] = 0;
            }
        }
    }

    // Step 5: Sort major colors (in number of weight size)
    std::sort(listMajorColors.begin(), listMajorColors.end(), sortMajorColors);

    // Debug

    /*for(MajorColorElem currentElem : listMajorColors)
    {
        cout << (int)currentElem.color[0] << " "
             << (int)currentElem.color[1] << " "
             << (int)currentElem.color[2] << endl;
        cout << currentElem.weightColor << endl;
    }
    imshow("src", src);
    imshow("mask", fgMask);
    imshow("dest", dest);
    waitKey( 0 );*/
}
