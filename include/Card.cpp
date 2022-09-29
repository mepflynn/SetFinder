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
        Card::Card(Mat src) {
            // Default values for now
            shape Shape = oval;
            shading Shading = open;
            color Color = red;
            number Number = one;

            this->sourceImage = src;

            // Initialize related Mats (images) for future use
            maskImage = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC1); ///TODO: Channel error issue; maskImage needs to stay single-channel, or be converted
            maskedShapes  = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC3);



            cardPeeper();
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

        void Card::maskAndIsolateShapes(Mat erodeDilateImage) {
            // Extract contours from the card image, filtering for setShapes by size and size-ratio (rectangular)
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy; // This code based on https://docs.opencv.org/3.4/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc
            findContours(erodeDilateImage, contours, hierarchy,
                RETR_CCOMP, CHAIN_APPROX_SIMPLE );

            
            

            int idx = 0;
            int imageArea = sourceImage.rows * sourceImage.cols;
            for( ; idx < contours.size(); idx++) {
                // filter by minimum/maximum contour area
                // Must be bigger than specks on the card, but smaller than the entire card.
                // Shape is about 1/6th, so: minimum 1/12th of card, max 1/2
                int shapeArea = contourArea(contours[idx]);
                int shapeAreaMinimum = imageArea / 12;
                int shapeAreaMaximum = imageArea / 2;
                if (shapeArea > shapeAreaMinimum && shapeArea < shapeAreaMaximum) {

                    // At this point, we're probably looking at a shape!
                    // Run one last test, check that width >> height (shape oughtta be horizontal)
                    // Start by creating bounding rectangle
                    Rect rect = boundingRect(contours[idx]);

                    // if taller than wide, skip to next contour
                    if (rect.height > rect.width) continue;

                    // Shape status is confirmed.
                    // Add it to the mask, for extraction and then processing
                    // (Draw it in color white, filled in shape via -1 for thickness)
                    drawContours(maskImage, contours, idx, Scalar(255,255,255), -1);
                    
                    // Also, draw this contour into a separate image and normalize it
                    // This image will be compared to references to determine the shape's identity
                    // WARNING: This new image is simply a partial pointer to the original maskImage
                    // if maskImage gets changes or goes out of scope, the information is lost.
                    binaryCardShapes.push_back(Mat(maskImage,rect));
                    shapeLocations.push_back(rect);


                    
                    // Categorize this shape by its shape, shading, and color
                    // categorizeShape()
                }            
            }
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

        void Card::whatShape() {
            // Retrieve each reference shape
            Mat refDiamond = imread("/shape_references/diamond.jpg", CV_8UC1);
            Mat refSquiggle = imread("/shape_references/squiggle.jpg", CV_8UC1);
            Mat refOval = imread("/shape_references/oval.jpg", CV_8UC1);

            vector<Mat> refShapes = {refDiamond, refSquiggle, refOval};

            vector<shape> shapeGuesses;
            for (Mat shape : binaryCardShapes) {
                
                vector<double> comparisonReturns;

                for (Mat refShape : refShapes) {
                    comparisonReturns.push_back(matchShapes(shape,refShape,CONTOURS_MATCH_I2,0));
                }

                double min = comparisonReturns[0];
                int minIndex = 0;
                for (int index = 1; index < comparisonReturns.size(); index++) {
                    if (min > comparisonReturns[index]) {
                        min = comparisonReturns[index];
                        min = index;
                    }
                }

                switch(minIndex) {
                    case(0):
                        shapeGuesses.push_back(diamond);
                        break;
                    case(1):
                        shapeGuesses.push_back(squiggle);
                        break;
                    case(2):
                        shapeGuesses.push_back(oval);
                        break;
                }
            }


            // Shapes found and decided; print the findings:
            for (shape guess : shapeGuesses ) {
                switch (guess) {
                    case(diamond):
                        cout << "Found diamond" << endl;
                        break;
                    case (squiggle):
                        cout << "Found squiggle" << endl;
                        break;
                    case (oval):
                        cout << "Found oval" << endl;
                        break;
                }
            }

            
        }
        

        

        // Given the extracted shapes, determine their
        // color, shape, shading, and number
        // This information exists in the various image member variables
        //
        // Function returns void, and will set the card parameters from within 
        void Card::categorizeShapes() {
            // Check for a valid number of shapes, 0 < S < 4
            if (binaryCardShapes.empty() || binaryCardShapes.size() > 3) {
                throw invalid_argument("Invalid argument size. Card must have 1-3 shapes detected. Check shape contouring.");
            }

            // Find the number of shapes, set 'Number'
            whatNumber(binaryCardShapes.size());

            // Find the shape of the shapes, set 'Shape'
            whatShape();


        }

        // Toplevel fcn for the steps of parsing the card image
        void Card::cardPeeper() { // This name via Sidney W.

            // Apply Canny edge detection to create a clean binary image of edges
            Mat edgesImage = cannyEdgeDetection(sourceImage);

            // The above may have left gaps along the edges, causing problems with contour detection
            // Run a series of erosion/dilation cycles to fill in gaps
            Mat erodeDilateImage = edgeErosionDilation(edgesImage);
    
            // Mask out shapes, and obtain binary shape outlines cropped out from the card image
            // This function populates maskImage, binaryCardShapes, and shapeLocations
            maskAndIsolateShapes(erodeDilateImage);


            // Having drawn the shapes onto the mask, mask shapes out of the source image
            // and deposit that result in maskedShapes
            bitwise_and(maskImage, sourceImage, maskedShapes);

            // time for categorizeShapes
            categorizeShapes();
        }

        


}
