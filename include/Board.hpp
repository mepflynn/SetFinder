#ifndef BOARD_HPP
#define BOARD_HPP

#include <opencv2/opencv.hpp>

#include "Card.hpp"

using namespace cv;
using namespace std;



namespace SetFinding{

    class Board {
        public:
            // Constructor takes the image of a board
            // Breaks it up into cards, and constructs
            // Card objects for each card image found

            Board(Mat sourceImg);

            // setFind() TBD once image detection works

        private:

            vector<Mat> extractCards(vector<vector<Point>> cardContours, Mat srcColor);

            vector<vector<Point>> findCardContours(Mat srcColor);


            Mat sourceImageColor;

            vector<Card> cards;

            vector<Mat> cardImages;
    };
}

#endif