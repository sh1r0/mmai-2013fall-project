#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		cerr << "[ERROR] no input image" << endl;
		return -1;
	}

	Mat im_gray = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	if (!im_gray.data) {
		cerr <<  "[ERROR] Could not open or find the image" << endl;
		return -1;
	}

	Mat im_bw;
	im_bw = im_gray < 253;
	medianBlur(im_bw, im_bw, 5);
	imshow("BW", im_bw);
	waitKey();

	imwrite("bw.jpg", im_bw);

	return 0;
}
