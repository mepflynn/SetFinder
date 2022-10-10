#include <opencv2/opencv.hpp>

#include <algorithm>
#include <fstream>

using namespace cv;
using namespace std;

#include "Card.hpp"

namespace SetFinding {
    
        int contoursIndex = 1;

            void saveContourToCSV(vector<Point> contour, string fileName) {
                ofstream file;

                file.open(fileName);

                // Column headers
                file << "x,y\n";

                for (auto itr = contour.begin(); itr < contour.end(); itr++) {
                    // Push one point (x,y) to a line, sep. by comma and terminated by newline
                    file << to_string(itr->x) << "," << to_string(itr->y) << "\n";
                }

            }

            vector<Point> contourFromCSV(string fileName) {
                ifstream file;
                vector<Point> contour;

                file.open(fileName);

                string line;
                int x,y,i;

                // Run on one line of the CSV at a time (one Point obj)
                while (!file.eof()) {
                    getline(file, line, '\n');

                    // Find the separator ','
                    i = line.find(',');

                    // X is all the nums up until the comma
                    x = stoi(line.substr(0,i));
                    // Y is after the comma, til EOS minus the '\n'
                    y = stoi(line.substr(i + 1,line.length() - i - 2));

                    contour.push_back(Point(x,y));
                }
                
                return contour;
            }




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
            maskImage = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC3);
            maskedShapes  = Mat::zeros(sourceImage.rows, sourceImage.cols, CV_8UC3);


            // Parse, separate, and analyze the card's shapes
            // Then set 
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

                    // Write this contour out to a file.
                    saveContourToCSV(contours[idx], "contour" + to_string(contoursIndex) + ".csv");
                    contoursIndex++;
                    
                    // Also, draw this contour into a separate image and normalize it
                    // This image will be compared to references to determine the shape's identity
                    // WARNING: This new image is simply a partial pointer to the original maskImage
                    // if maskImage gets changes or goes out of scope, the information is lost.
                    
                    // Push the selected shape image into binaryShapes for shape analysis
                    // Make a temp Mat to turn this image into singlechannel for future call to matchShape()
                    Mat binaryCard;
                    cvtColor(Mat(maskImage,rect), binaryCard, COLOR_BGR2GRAY);

                    imshow("bin card #" + to_string(contoursIndex), binaryCard);
                    waitKey(0);

                    binaryShapes.push_back(binaryCard);
                    shapeLocations.push_back(rect);
                    shapeContours.push_back(contours[idx]);
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
                default:
                    cout << "Greater than or less than 3 shapes, oopsie" << endl;
            }
        }

        void Card::whatShape() {
            // Retrieve each reference shape
            Mat refDiamond = imread("/shape_references/diamond.jpg", CV_8UC1);
            Mat refSquiggle = imread("/shape_references/squiggle.jpg", CV_8UC1);
            Mat refOval = imread("/shape_references/oval.jpg", CV_8UC1);



            vector<Mat> refShapes = {refDiamond, refSquiggle, refOval};

            vector<vector<vector<Point>>> refContours;
            vector<vector<Point>> refContour;

            for (int j = 0; j < refShapes.size(); j++) {
                vector<vector<Point>> contours;
                vector<Vec4i> hierarchy; // This code based on https://docs.opencv.org/3.4/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc
                findContours(refShapes[j], refContour, hierarchy,
                    RETR_CCOMP, CHAIN_APPROX_SIMPLE );
                refContours.push_back(refContour);
            }

            for (vector<vector<Point>> cntList : refContours) {
                for (vector<Point> contour : cntList) {
                    if (contourArea(contour) > refDiamond.rows * refDiamond.cols / 2) {
                        shapeContours.push_back(contour);
                    }
                }
            }

            vector<shape> shapeGuesses;
            for (Mat shape : binaryShapes) {
                
                vector<double> comparisonReturns;

                for (Mat refShape : refShapes) {
                    comparisonReturns.push_back(matchShapes(shape,refShape,CONTOURS_MATCH_I2,0.0));
                }

                double min = comparisonReturns[0];
                int minIndex = 0;
                for (int index = 1; index < comparisonReturns.size(); index++) {
                    if (min > comparisonReturns[index]) {
                        min = comparisonReturns[index];
                        minIndex = index;
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
            try {
                if (binaryShapes.empty() || binaryShapes.size() > 3) {
                    throw invalid_argument("Invalid argument size. Card must have 1-3 shapes detected. Check shape contouring.");
                }
            } catch(invalid_argument) {
                // cout << "Found " <<  binaryShapes.size() << " shapes." << endl;

                // for (Mat oneShape : binaryShapes) {
                //     cout << "Area of shape:" << oneShape.rows << " by " << oneShape.cols << endl;
                // }

                if (binaryShapes.empty()) {
                    cout << "No shapes found." << endl;
                    return;
                }

                // int shapeNum = 1;
                // for (Mat oneShape : binaryShapes) {
                //     imshow("shape number" + to_string(shapeNum),oneShape);
                //     shapeNum++;
                // }

                // imshow("sourceImage",sourceImage);
                // imshow("maskImage",maskImage);
                // imshow("maskedShapes",maskedShapes);
                // waitKey(0);

                // return;

                

                // The maskAndIsolateShapes function currently over-detects squiggles and ovals,
                // it finds them each twice
                // The bandaid solution here is to delete half the shapes when there are too many
                cout << "Too many shapes found, halving" << endl;
                int halfSize = binaryShapes.size() / 2;
                while (binaryShapes.size() > halfSize) {
                    binaryShapes.pop_back();
                }
            }

            // Find the number of shapes, set 'Number'
            whatNumber(binaryShapes.size());

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
            // This function populates maskImage, binaryShapes, and shapeLocations
            maskAndIsolateShapes(erodeDilateImage);


            // Having drawn the shapes onto the mask, mask shapes out of the source image
            // and deposit that result in maskedShapes
            bitwise_and(maskImage, sourceImage, maskedShapes);

            // imshow("Mask before thresh", maskImage);

            // Convert maskImage down to a CV_8UC1 (single-channel image) for future 
            // disassembly into individual CV_8UC1 shapes
            // for (int i = 0; i < binaryShapes.size(); i++) {
            //     Mat singleChannelShape = Mat::zeros(maskImage.rows, maskImage.cols, CV_8UC1);

            //     threshold(maskImage, binaryShapes[i], 1, 255, THRESH_BINARY);
            // }


            // imshow("Mask after thresh", maskImage);
            // waitKey(0);


            // time for categorizeShapes
            categorizeShapes();
        }

        


}
