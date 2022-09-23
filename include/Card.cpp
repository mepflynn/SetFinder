#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#include "Card.hpp"

namespace SetFinding {



class Card {
    public:

        Card(Mat sourceImage) {
            cardPeeper(sourceImage);
        }

    private:
        // Convert image to grayscale and run canny edge detection algorithm
        // return the binary image edges-only output
        Mat cannyEdgeDetection(Mat sourceImage) {

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
        void Erosion(Mat& src) {
            int erosion_type = MORPH_RECT; 
            int erosion_size = 2;


            Mat element = getStructuringElement( erosion_type,
                                Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                Point( erosion_size, erosion_size ) );
            erode( src, src, element );
            imshow( "Erosion Demo", src );
        }

        // run cv::dilate() with set parameters on src
        void Dilation(Mat& src) {
            int dilation_type = dilation_type = MORPH_RECT; 
            int dilation_size = 2;

            Mat element = getStructuringElement( dilation_type,
                                Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                Point( dilation_size, dilation_size ) );
            dilate( src, src, element );
            imshow( "Dilation Demo", src );
        }

        
        // Run the above fcns repeatedly to fill in edge gaps
        // and prepare the image for contour detection
        Mat edgeErosionDilation(Mat src) {
            for (int i = 0; i < 3; i++) {
                Dilation(src);
                Erosion(src);
            }

            return src;
        }

        

        // Given the extracted shapes, determine their
        // color, shape, shading, and number
        void categorizeShapes(vector<Mat> cardShapes) {
            if (cardShapes.empty() || cardShapes.size() > 3) {
                throw invalid_argument("Invalid argument size. Card must have 1-3 shapes detected. Check shape contouring.");
            }



            for (Mat shape : cardShapes) {
                ///TODO: Figure out how to process shape, color, shading, number
            }
        }

        // Toplevel fcn for the steps of parsing the card image
        void cardPeeper(Mat sourceImage) { // This name via Sidney W.

            // Apply Canny edge detection to create a clean binary image of edges
            Mat edgesImage = cannyEdgeDetection(sourceImage);

            // The above may have left gaps along the edges, causing problems with contour detection
            // Run a series of erosion/dilation cycles to fill in gaps


            // Extract contours from the card image, filtering for setShapes by size and size-ratio (rectangular)
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy; // This code based on https://docs.opencv.org/3.4/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc
            findContours(edgesImage, contours, hierarchy,
                RETR_CCOMP, CHAIN_APPROX_SIMPLE );

            Mat maskImage = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC3);

            int idx = 0;
            int imageArea = edgesImage.rows * edgesImage.cols;
            for( ; idx < contours.size(); idx++) {
                // first, filter by minimum contour area
                // Must be bigger than specks on the card, but smaller than the entire card.
                // Card shapes appear to make rectangles about 1/6 total card size, filter by 1/8th
                // Filter upper limit by 1/2 card size
                int shapeArea = contourArea(contours[idx]);
                
            ///TODO: contourArea is making small nums even though the shape is getting contoured
            // so we're skipping over drawing the shape
            // Card has gaps? I forgot to do fillingaps step
                int shapeAreaMinimum = 200; //imageArea / 12;
                int shapeAreaMaximum = imageArea / 2;
                if (shapeArea > shapeAreaMinimum && shapeArea < shapeAreaMaximum) {

                    // At this point, we're probably looking at a shape!
                    // Add it to the mask, for extraction and then processing
                    // (Draw it in color white, filled in shape via -1 for thickness)
                    drawContours(maskImage, contours, idx, Scalar(255,255,255), -1);
                    
                    // Then if the contour is large enough, create a bounding minSizeRectangle.
                    // RotatedRect rect = minAreaRect(contour);
                    // Lastly, verify that width >> height
                    // This is TBD for now, may not be needed, left unimplemented /////////////

                    
                    // Categorize this shape by its shape, shading, and color
                    // categorizeShape()
                }            
            }

            // Test display the mask itself
            imshow("Hopefully masked out shapes",maskImage);
            waitKey(0);

            // Having drawn the shapes onto the mask, mask shapes out of the source image
            Mat maskedShapes  = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC3);
            bitwise_and(maskImage, sourceImage, maskedShapes);

            // Test display the masked shapes
            imshow("Hopefully masked out shapes",maskedShapes);
            waitKey(0);

            // time for categorizeShapes
        }
    
        Mat sourceImage;
    
        int shape;     // Arbitrarily: 0 == oval, 1 == diamond, 2 == squiggle
        int shading;
        int color;
        int number;
};

}


