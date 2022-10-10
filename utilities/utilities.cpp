// This file is not for active use in the finished build

// It consists of development utilities which have been taken
// out of the main build but are kept here for possible future need

// Using keywords
using namespace cv;
using namespace std;

// Include statements

// OpenCV libs
#include <opencv2/opencv.hpp>
#include <fstream>
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
            // Other image-showing code for ease of use:
            // And shows the other relebant members, the source image and its mask components
            // imshow("src",sourceImage);
            // imshow("mask",maskImage);
            // imshow("mask Shapes",maskedShapes);
            ////////////////////////////////////////////////////////////


            // Take some images (binary images of shapes) and extract contours
            // that represent the shapes, for future use as a reference contour in matchShapes
            void contoursFromImages(vector<Mat> refShapes) {
                ///TODO: Not working for now, doesn't find any contours in the refShapes

                vector<vector<vector<Point>>> refContours(3);

                for (int j = 0; j < refShapes.size(); j++) {
                    vector<vector<Point>> contours;
                    vector<Vec4i> hierarchy; // This code based on https://docs.opencv.org/3.4/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc
                    findContours(refShapes[j], refContours[j], hierarchy,
                        RETR_CCOMP, CHAIN_APPROX_SIMPLE );
                }

                vector<vector<Point>> shapeContours;

                for (vector<vector<Point>> cntList : refContours) {
                    for (vector<Point> contour : cntList) {
                        if (contourArea(contour) > refShapes[0].rows * refShapes[0].cols / 2) {
                            shapeContours.push_back(contour);
                        }
                    }
                }
            
            }

            void saveContourToCSV(vector<Point> contour, string fileName) {
                ofstream file;
                
                file.open(fileName);

                // Column headers
                file << "x,y\n";

                for (auto itr = contour.begin(); itr < contour.end(); itr++) {
                    // Push one point (x,y) to a line, sep. by comma and terminated by newline
                    file << itr->x << "," << itr->y << "\n";
                }

            }


            Mat resizeMat(Mat source, Size2i newSize) {
                Mat resizedImage;

                resize(source, resizedImage, newSize, INTER_LINEAR);

                return resizedImage;
            }


            // Extract shape from image, save it out to an appropriately sized file 
            void saveShapeToJPG(Mat img, Rect boundRect, string fileName) {
                // Extract the shape into a separate Mat via
                // This overload of the Mat constructor
                Mat shape = Mat(img, boundRect);

                // shape still has a pointer to the original image
                // This is OK because we only need it briefly:

                // Save a copy of the image shape to jpg
                imwrite(fileName,shape);


                //////////////////////////////////////////////////////////////
                //Below is a more fleshed out and case-specific implementation

                vector<int> shapeIndices;
                // previously populated the above vector with indices of suspected shapes in contours[]
                // This needs implementation for the following to work

                string fileName;
                int fileNum = rand()&255;
                for(int ix : shapeIndices) { 
                    //Rect boundRect = boundingRect(contours[ix]);

                    
            
                    fileName = "card" + to_string(fileNum) + ".jpg";
                    fileNum++;

                    //Mat shape = Mat(maskImage, boundRect);

                    imwrite(fileName,shape);

                }
 
            }

        };
}

