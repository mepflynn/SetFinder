#ifndef CARD_HPP
#define CARD_HPP

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

namespace SetFinding{

    // Enums to form the card characteristics
    enum shape {oval, squiggle, diamond};
    enum shading {open, lines, shaded};
    enum color {red, green, purple};
    enum number {one = 1, two, three};

class Card {
    public: 
        // Constructor takes image of a card, and
        // populates the fields that describe a card
        // (shape, shading, color number) via image processing

        Card(Mat sourceImg);

    private:
        // Convert image to grayscale and run canny edge detection algorithm
        // return the binary image edges-only output
        Mat cannyEdgeDetection(Mat sourceImage);

        // run cv::erode() with set parameters on src
        void Erosion(Mat& src);

        // run cv::dilate() with set parameters on src
        void Dilation(Mat& src);

        // Run the above fcns repeatedly to fill in edge gaps
        // and prepare the image for contour detection
        Mat edgeErosionDilation(Mat src);

        // Run findContours, then sort the contours for those
        // representing shapes, and mask + isolate those shapes
        void maskAndIsolateShapes(Mat erodeDilateImage);

        // How many shapes are on the card?
        void whatNumber(int numShapes);

        void whatShape();

        // Given the extracted shapes, determine their
        // color, shape, shading, and number
        void categorizeShapes();

        // Toplevel fcn for the steps of parsing the card image
        void cardPeeper();


        // Image files and associated parameters
        Mat sourceImage;
        Mat maskImage;
        Mat maskedShapes;
        vector<Mat> binaryShapes; 
        vector<Rect> shapeLocations;
        vector<vector<Point>> shapeContours;


        // Instatiate copies of each card-defining enum
        shape Shape;
        shading Shading;
        color Color;
        number Number;
};

} // end namespace

#endif