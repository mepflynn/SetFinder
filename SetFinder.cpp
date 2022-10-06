#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

#include "Card.hpp"
#include "Board.hpp"

using namespace cv;
using namespace std;




int main( int argc, char** argv )
{
    
 
    // Retrieve the image into a Mat
    string img = "Set_Board_Shrunk.jpg";
    Mat srcColor = imread(img);
    ///TODO: Detect image size and shrink it to some standard value, to minimize processing and 
    /////////Allow the input of differently sized images

    // Check for successful image retrieval
    if (!srcColor.data) {
        return 1;
    }

    

    // Parse card images, construct Card objects
    SetFinding::Board theBoard(srcColor);

    // Write a Board::setFind() function, call it here
    

    
    return 0;
}

