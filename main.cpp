#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include "SLIC.h"

using namespace cv;

int main(int argc, char* argv[])
{
	if (argc < 2) {
        cerr << "[ERROR] no input image" << endl;
        return -1;
    }

    Mat img = imread(argv[1], 1);

    if (!img.data) {
        cerr <<  "[ERROR] Could not open or find the image" << endl ;
        return -1;
    }

	int width = img.cols;
	int height = img.rows;
	// unsigned int (32 bits) to hold a pixel in ARGB format as follows:
	// from left to right,
	// the first 8 bits are for the alpha channel (and are ignored)
	// the next 8 bits are for the red channel
	// the next 8 bits are for the green channel
	// the last 8 bits are for the blue channel
	unsigned int *pbuff = new unsigned int[width*height];

	Vec3b intensity;
	unsigned int blue, green, red;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			intensity = img.at<Vec3b>(i, j);
			blue = intensity.val[0];
			green = intensity.val[1];
			red = intensity.val[2];
			pbuff[i*width+j] = (red << 16) + (green << 8) + blue;
		}
	}

	//----------------------------------
	// Initialize parameters
	//----------------------------------
	int k = 200; // Desired number of superpixels.
	double m = 20; // Compactness factor. use a value ranging from 10 to 40 depending on your needs. Default is 10
	int *klabels = new int[width*height];
	int numlabels = 0;
	string filename(argv[1]);
	string savepath("");
	//----------------------------------
	// Perform SLIC on the image buffer
	//----------------------------------
	SLIC segment;
	segment.PerformSLICO_ForGivenK(pbuff, width, height, klabels, numlabels, k, m);
	// Alternately one can also use the function PerformSLICO_ForGivenStepSize() for a desired superpixel size
	//----------------------------------
	// Save the labels to a text file
	//----------------------------------
	segment.SaveSuperpixelLabels(klabels, width, height, filename, savepath);
	//----------------------------------
	// Draw boundaries around segments
	//----------------------------------
	segment.DrawContoursAroundSegments(pbuff, klabels, width, height, 0xff0000);
	// segment.DrawContoursAroundSegmentsTwoColors(pbuff, klabels, width, height);
	//----------------------------------
	// Save the image with segment boundaries.
	//----------------------------------
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			uchar *color = (uchar*)(pbuff+i*width+j);
			img.at<Vec3b>(i, j)[0] = color[0];
			img.at<Vec3b>(i, j)[1] = color[1];
			img.at<Vec3b>(i, j)[2] = color[2];
		}
	}
	imwrite("slic.png", img);


	//----------------------------------
	// Clean up
	//----------------------------------
	if (pbuff) delete [] pbuff;
	if (klabels) delete [] klabels;

	return 0;
}
