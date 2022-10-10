#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

namespace SetFinding{

        void printImage(Mat image);

        void drawAllContoursPrettyColors(Mat image);

        void showImages(vector<Mat> images);

        void contoursFromImages(vector<Mat> refShapes);

        void saveContourToCSV(vector<Point> contour, string fileName);

        vector<Point> contourFromCSV(string fileName);

        Mat resizeMat(Mat source, Size2i newSize);

        void saveShapeToJPG(Mat img, Rect boundRect, string fileName);

        

}

#endif