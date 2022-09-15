#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


#include <iostream>

using namespace cv;
using namespace std;


int main( int argc, char** argv )
{
    cout << "aa" << endl;
 
    // Retrieve the image into a Mat
    string img = "Set_Board_Shrunk.jpg";
    Mat srcColor = imread(img);

    // Check for successful image retrieval
    if (!srcColor.data) {
        return 1;
    }

    // Convert the source image into grayscale
    Mat srcGray;

    cvtColor(srcColor, srcGray, COLOR_BGR2GRAY);

    // Parse this grayscale image into a binary image

    // First, create a blank image Mat of the same bit-depth (8), size (row,col), type (U = unsigned int?) and numColorChannels (3)
    Mat srcBin = Mat::zeros(srcGray.rows, srcGray.cols, CV_8UC1);

    // Convert into a binary image, by turning all pixles valued above 120 white
    // and all below it black
    threshold(srcGray, srcBin, 120, 100, THRESH_BINARY);
    // 120,100 works well for this example image
    // in future, implement adaptiveThreshold to be more flexible
    // in changing lighting conditions


    // Use binary image to find contours and detect cards
    srcBin = srcBin > 1;
    namedWindow( "Source", 1 );
    imshow( "Source", srcBin );
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy; // This code based on https://docs.opencv.org/3.4/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc
    findContours( srcBin, contours, hierarchy,
        RETR_CCOMP, CHAIN_APPROX_SIMPLE );


    // Parsing the vector contours to find which are the outlines of cards
    vector<vector<Point>> biggerContours;

    // use 1/30th of total image size as benchmark for minimum card size
    int cardSizeThreshold = (int)((srcBin.rows * srcBin.cols) / 35);

    for (int i = 0; i < contours.size(); i++) {
        // If contour meets size threshold, add it to the list
        if (contourArea(contours[i]) > cardSizeThreshold) biggerContours.push_back(contours[i]);
    }

    // Create a destination mat for the results
    Mat dst = Mat::zeros(srcGray.rows, srcGray.cols, CV_8UC3);

    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    int idx = 0;
    // for( ; idx >= 0; idx = hierarchy[idx][0] )
    // {
    //     Scalar color( rand()&255, rand()&255, rand()&255 );
    //     drawContours( dst, contours, idx, color, FILLED, 8, hierarchy );
    // }
    // Instead, draw only the bigger contours
    Scalar color( rand()&255, rand()&255, rand()&255 );
    drawContours(dst, biggerContours, -1, color, 3);
    namedWindow( "Components", 1 );
    imshow( "Components", dst );
    waitKey(0);

    return 0;
}