// This file is not for active use in the finished build

// It consists of development utilities which have been taken
// out of the main build but are kept here for possible future need

// Using keywords
using namespace cv;
using namespace std;

// Include statements

// OpenCV libs
#include <opencv2/opencv.hpp>
//#include <opencv2/highgui.hpp>

namespace SetFinding {
    class devUtilities {
        public:
            // Writing a Mat out to be a .jpg image
            void printImage(Mat image) {
                // Write cards out to images for finding ideal erode/dilate parameters
                int randint = (rand()&300);
                string cardName = "card" + to_string(randint) + ".jpg";
                cv::imwrite(cardName, image); 
            }

            // Image is a binary image that has been thresholded to a given value
            // This function runs findContours() and then pretty-prints the results with random colors
            void drawAllContoursPrettyColors(Mat image) {
                // Extract contours from the card image, filtering for setShapes by size and size-ratio (rectangular)
                vector<vector<Point>> contours;
                vector<Vec4i> hierarchy; // This code based on https://docs.opencv.org/3.4/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc
                findContours(image, contours, hierarchy,
                    RETR_CCOMP, CHAIN_APPROX_SIMPLE );




                // Draw contours for testing purposes
                int ixd = 0;
                Mat dst = Mat::zeros(image.rows, image.cols, CV_8UC3);
                    for( ; ixd >= 0; ixd = hierarchy[ixd][0] )
                {
                    Scalar color( rand()&255, rand()&255, rand()&255 );
                    drawContours( dst, contours, ixd, color, FILLED, 8, hierarchy );
                }
            }

            // Displays all the images in seperate, numbered windows
            // Then stops execution until user enters a keypress
            void showImages(vector<Mat> images) {
                for (int i = 0; i < images.size(); i++) {
                    string label = "Card #" + to_string(i);
                    imshow(label, images[i]);
                }
                waitKey(0);
            }


        };
}

