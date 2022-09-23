#ifndef CARD_HPP
#define CARD_HPP

#include <opencv2/opencv.hpp>

namespace SetFinding{

class Card {
    public: 
        // Constructor takes image of a card, and
        // populates the fields that describe a card
        // (shape, shading, color number) via image processing
        Card(cv::Mat sourceImage);

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

        // Given the extracted shapes, determine their
        // color, shape, shading, and number
        void categorizeShapes(vector<Mat> cardShapes);

        // Toplevel fcn for the steps of parsing the card image
        void cardPeeper(Mat sourceImage);
};

} // end namespace

#endif