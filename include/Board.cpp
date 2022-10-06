#include <opencv2/opencv.hpp>

#include "Board.hpp"

using namespace cv;
using namespace std;

namespace SetFinding{

    vector<Mat> Board::extractCards(vector<vector<Point>> cardContours, Mat srcColor) {
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

    vector<vector<Point>> Board::findCardContours(Mat srcColor) {
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
        // in future, implement adaptiveThreshold to be more 
        // flexible in changing lighting conditions


        // Use binary image to find contours and detect cards
        srcBin = srcBin > 1;
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy; // This code based on https://docs.opencv.org/3.4/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc
        findContours( srcBin, contours, hierarchy,
            RETR_CCOMP, CHAIN_APPROX_SIMPLE );


        // Parsing the vector contours to find which are the outlines of cards
        vector<vector<Point>> cardContours;

        // use 1/35th of total image size as benchmark for minimum card size
        // This is arbitrary, and is maybe a bit too small? TODO: Refine this constant
        int cardSizeThreshold = (int)((srcBin.rows * srcBin.cols) / 35);
        /////////////////////// TODO: ///////////////////////////////////
        // could also filter for cards based upon their shape ratio (width:height)
        // Set cards have approx 1:1.6 ratio of Width:Height
        // Would require running minSizeRect() on the contour to obtain these measurements
        // Implement this if card detection becomes faulty during testing
        /////////////////////////////////////////////////////////////////

        for (int i = 0; i < contours.size(); i++) {
            // If contour meets size threshold, add it to the list
            if (contourArea(contours[i]) > cardSizeThreshold) cardContours.push_back(contours[i]);
        }

        return cardContours;

    }


    Board::Board(Mat sourceImage) {
        sourceImageColor = sourceImage;

        // apply findContours() and filter for the contours that
        // represent the outlines of cards 
        vector<vector<Point>> cardContours = findCardContours(sourceImageColor);

        // TODO: /////////////////////////////////////////////////////
        // We expect to receive cards in multiples of 3, usually 12
        // Here, could check that cardContours contains a multiple of 3
        // and warn the user if not.
        // Or, would it be better to show the user the card-contour overlay
        // to confirm they're happy with the image? Then provide an option to
        // retake, roll with the taken image, or cancel the process
        //////////////////////////////////////////////////////////////

        cardImages = extractCards(cardContours, sourceImageColor);

        // Construct a Card object based upon each image
        // Push it back to the existing vector cards()
        int cardNum = 1;
        for (Mat image : cardImages) {
            cout << cardNum << endl;
            cardNum++;
            cards.push_back(SetFinding::Card(image));
        }
    }



}