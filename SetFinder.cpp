#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


#include <iostream>

using namespace cv;
using namespace std;


vector<Mat> extractCards(vector<vector<Point>> cardContours, Mat srcColor) {
    vector<Mat> cardImages;
    
    for (vector<Point> contour : cardContours) {
        // Obtain simplified rectangular coordinates for each card
        RotatedRect rect = minAreaRect(contour);

        vector<Point2f> box(4);

        rect.points(box.data());
        // Not doing the conversion step here the Python does?
        // See if this causes issues ////////// TODO /////////


        float width = rect.size.width;
        float height = rect.size.height;


        vector<Point2f> dstPoints = {{0,height-1},{0,0},{width-1,0},{width-1,height-1}};

        Mat M = getPerspectiveTransform(box, dstPoints);

        Mat warped;

        warpPerspective(srcColor, warped, M, rect.size);

        

        if (warped.rows < warped.cols) {
            // This card is laid out horizontal
            // Transform the mat 90 degree to be vertical
            rotate(warped, warped, ROTATE_90_CLOCKWISE);
        } 

        cardImages.push_back(warped);
        
    }

    return cardImages;

}


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
    vector<vector<Point>> cardContours;

    // use 1/35th of total image size as benchmark for minimum card size
    // This is arbitrary, and is maybe a bit too small? TODO: Refine this constant
    int cardSizeThreshold = (int)((srcBin.rows * srcBin.cols) / 35);

    for (int i = 0; i < contours.size(); i++) {
        // If contour meets size threshold, add it to the list
        if (contourArea(contours[i]) > cardSizeThreshold) cardContours.push_back(contours[i]);
    }

    vector<Mat> cardImages = extractCards(cardContours, srcColor);


    for (int i = 0; i < cardImages.size(); i++) {
        string label = "Card #" + to_string(i);
        imshow(label, cardImages[i]);
    }

    waitKey(0);

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

    // Instead, draw only the card's contours
    Scalar color( rand()&255, rand()&255, rand()&255 );
    drawContours(dst, cardContours, FILLED, color, 3);
    namedWindow( "Components", 1 );
    imshow( "Components", dst );
    waitKey(0);

    return 0;
}