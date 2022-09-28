#include <opencv2/opencv.hpp>

#include <algorithm>

using namespace cv;
using namespace std;

#include "Card.hpp"

namespace SetFinding {




        // This function will parse the card image,
        // extract the shape(s) from it, and then
        // categorize those shapes to set values for
        // shape, shading, color, and number.
        Card::Card(Mat sourceImg) {
            // Default values for now
            shape Shape = oval;
            shading Shading = open;
            color Color = red;
            number Number = one;

            sourceImage = sourceImg;

            cardPeeper(sourceImage);
        }

        // Convert image to grayscale and run canny edge detection algorithm
        // return the binary image edges-only output
        Mat Card::cannyEdgeDetection(Mat sourceImage) {

            // Convert source image into grayscale
            Mat grayImage = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC1);
            cvtColor(sourceImage, grayImage, COLOR_BGR2GRAY);

            /////////////////////////////////////////////////////////////////////////////////////////////////////
            // // Apply gaussian blur to denoise and and avoid detecting line-shading as separate internal shapes
            // Mat blurImage = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC3);
            // GaussianBlur(grayImage, blurImage, Size_(2,2), 0.5, 0.5);
            /////////////////////////// No gaussian blur for now as canny runs its own 5x5 gaussian /////////////

            // Run Canny edge detection to outline shapes
            Mat edgesImage = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC1);
            Canny(grayImage, edgesImage, 100,200);

            return edgesImage;
        }

        // run cv::erode() with set parameters on src
        void Card::Erosion(Mat& src) {
            int erosion_type = MORPH_RECT; 
            int erosion_size = 2;


            Mat element = getStructuringElement( erosion_type,
                                Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                Point( erosion_size, erosion_size ) );
            erode( src, src, element );
        }

        // run cv::dilate() with set parameters on src
        void Card::Dilation(Mat& src) {
            int dilation_type = dilation_type = MORPH_RECT; 
            int dilation_size = 2;

            Mat element = getStructuringElement( dilation_type,
                                Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                Point( dilation_size, dilation_size ) );
            dilate( src, src, element );
        }

        
        // Run the above fcns repeatedly to fill in edge gaps
        // and prepare the image for contour detection
        Mat Card::edgeErosionDilation(Mat src) {
            for (int i = 0; i < 3; i++) {
                Dilation(src);
                Erosion(src);
            }

            return src;
        }

        void Card::whatNumber(int numShapes) {
            // How many shapes are on the card?
            switch (numShapes) {
                case(1):
                    Number = one;
                    break;
                case (2):
                    Number = two;
                    break;
                case (3):
                    Number = three;
                    break;
            }
        }

        void Card::whatShape(vector<Mat> cardShapes) {
            // Retrieve each reference shape
            Mat refDiamond = imread("/shape_references/diamond.jpg");
            Mat refSquiggle = imread("/shape_references/squiggle.jpg");
            Mat refOval = imread("/shape_references/oval.jpg");

            vector<Mat> refShapes = {refDiamond, refSquiggle, refOval};

            vector<shape> shapeGuesses;
            for (Mat shape : cardShapes) {
                
                vector<double> comparisonReturns;

                for (Mat refShape : refShapes) {
                    comparisonReturns.push_back(matchShapes(shape,refShape,CONTOURS_MATCH_I2,0));
                }

                auto maxFinder = max_element(comparisonReturns.begin(), comparisonReturns.end());

                //int maxIndex = maxFinder.nextIndex

                ///TODO: Implement this index-finding behavior to determine the best matched shape.
                
                switch (*maxFinder) {
                    case((double)0):
                        shapeGuesses.push_back(diamond);
                        break;
                    case((double)1):
                        shapeGuesses.push_back(squiggle);
                        break;
                    case((double)2):
                        shapeGuesses.push_back(oval);
                        break;
                }
            }

            
        }
        

        

        // Given the extracted shapes, determine their
        // color, shape, shading, and number
        //
        // Function is void, and will set the card 
        // parameters from within 
        void Card::categorizeShapes(vector<Mat> cardShapes) {
            // Check for a valid number of shapes, 0 < S < 4
            if (cardShapes.empty() || cardShapes.size() > 3) {
                throw invalid_argument("Invalid argument size. Card must have 1-3 shapes detected. Check shape contouring.");
            }

            // Find the number of shapes, set 'Number'
            whatNumber(cardShapes.size());


        }

        // Toplevel fcn for the steps of parsing the card image
        void Card::cardPeeper(Mat sourceImage) { // This name via Sidney W.

            // Apply Canny edge detection to create a clean binary image of edges
            Mat edgesImage = cannyEdgeDetection(sourceImage);

            // The above may have left gaps along the edges, causing problems with contour detection
            // Run a series of erosion/dilation cycles to fill in gaps
            Mat erodeDilateImage = edgeErosionDilation(edgesImage);


            // Extract contours from the card image, filtering for setShapes by size and size-ratio (rectangular)
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy; // This code based on https://docs.opencv.org/3.4/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc
            findContours(erodeDilateImage, contours, hierarchy,
                RETR_CCOMP, CHAIN_APPROX_SIMPLE );

            Mat maskImage = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC3);

            vector<int> shapeIndices;

            int idx = 0;
            int imageArea = edgesImage.rows * edgesImage.cols;
            for( ; idx < contours.size(); idx++) {
                // filter by minimum/maximum contour area
                // Must be bigger than specks on the card, but smaller than the entire card.
                // Shape is about 1/6th, so: minimum 1/12th of card, max 1/2
                int shapeArea = contourArea(contours[idx]);
                int shapeAreaMinimum = imageArea / 12;
                int shapeAreaMaximum = imageArea / 2;
                if (shapeArea > shapeAreaMinimum && shapeArea < shapeAreaMaximum) {

                    // At this point, we're probably looking at a shape!
                    // Add it to the mask, for extraction and then processing
                    // (Draw it in color white, filled in shape via -1 for thickness)
                    drawContours(maskImage, contours, idx, Scalar(255,255,255), -1);
                    shapeIndices.push_back(idx);
                    
                    // Then if the contour is large enough, create a bounding minSizeRectangle.
                    // RotatedRect rect = minAreaRect(contour);
                    // Lastly, verify that width >> height
                    // This is TBD for now, may not be needed, left unimplemented /////////////

                    
                    // Categorize this shape by its shape, shading, and color
                    // categorizeShape()
                }            
            }

                        // Test display the mask itself
            imshow("Mask for image processing",maskImage);
            waitKey(0);

            // Also, draw these contours out into JPGs for shape comparison
            string fileName;
            int fileNum = rand()&255;
            for(int ix : shapeIndices) {
                Rect boundRect = boundingRect(contours[ix]);

                
        
                fileName = "card" + to_string(fileNum) + ".jpg";
                fileNum++;

                Mat shape = Mat(maskImage, boundRect);

                imwrite(fileName,shape);

            }

            // Test display the mask itself
            imshow("Mask for image processing",maskImage);

            // Having drawn the shapes onto the mask, mask shapes out of the source image
            Mat maskedShapes  = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC3);
            bitwise_and(maskImage, sourceImage, maskedShapes);

            // // Test display the masked shapes
            // imshow("Hopefully masked out shapes",maskedShapes);
            // waitKey(0);

            // time for categorizeShapes
        }


}


