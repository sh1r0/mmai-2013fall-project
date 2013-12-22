#include <iostream>
#include <string>
#include <stack>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class elem {
public:
	int height;
	int position;

	elem() {};

	elem(int p, int h) {
		height = h;
		position = p;
	};
};

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		cerr << "[ERROR] no input image" << endl;
		return -1;
	}

	string file_path = string(argv[1]);
	unsigned dot = file_path.find_last_of(".");
	string output_prefix = file_path.substr(0, dot);

	Mat im_rgb  = imread(argv[1], 1);

	if (!im_rgb.data) {
		cerr <<  "[ERROR] Could not open or find the image" << endl;
		return -1;
	}

	Mat im_gray;
	cvtColor(im_rgb, im_gray, CV_RGB2GRAY);

	Mat im_bw = im_gray < 253;
	medianBlur(im_bw, im_bw, 5);

	unsigned char *mask = im_bw.data;
	int height = im_bw.rows;
	int width = im_bw.cols;

	int *dp_table = new int[height*width];
	for (int i = 0; i < width; ++i) {
		dp_table[i] = mask[i];
		for (int j = 1; j < height; j++) {
			dp_table[j*width+i] = (mask[j*width+i]) ? dp_table[(j-1)*width+i]+1 : 0;
		}
	}

	stack<elem> rect_stack;
	int max_area = 0, max_x, max_y, max_w, max_h;
	for (int i = height-1; i >= 0; i--) {
		int last_zero = 0;
		elem left;

		for (int j = 0; j < width; j++) {
			int h = dp_table[i*width+j];
			if (rect_stack.empty()) {
				if (h > 0) {
					rect_stack.push(elem(j, h));
				} else {
					last_zero = j;
				}
			} else if (h > rect_stack.top().height) {
				rect_stack.push(elem(j, h));
			} else {
				while (h < rect_stack.top().height) {
					left = rect_stack.top();
					int w = j - left.position;
					int area = w*left.height;

					if (area > max_area) {
						max_area = area;
						max_x = left.position;
						max_y = i - left.height + 1;
						max_w = w;
						max_h = left.height;
					}

					rect_stack.pop();

					if (rect_stack.empty()) {
						if (h == 0) {
							last_zero = j;
						} else {
							rect_stack.push(elem(last_zero+1, h));
						}
						break;
					}
				}
			}
		}

		while (!rect_stack.empty()) {
			left = rect_stack.top();
			int w = width - left.position;
			int area = w*left.height;
			if (area > max_area) {
				max_area = area;
				max_x = left.position;
				max_y = i - left.height + 1;
				max_w = w;
				max_h = left.height;
			}
			rect_stack.pop();
		}
	}

	int center_x = max_x + (max_w >> 1);
	int center_y = max_y + (max_h >> 1);
	int radius = (int)(max_w * 0.4);
	Rect roi = Rect(center_x - radius, center_y - radius, 2*radius, 2*radius);
	imwrite(output_prefix+"_patch.jpg", im_rgb(roi));
/*
	// show images for parameter tuning
	imshow("roi", im_rgb(roi));
	rectangle(im_rgb, Point(center_x - radius, center_y - radius),
		Point(center_x + radius, center_y + radius), CV_RGB(0, 255, 0), 1, 8, 0);
	rectangle(im_rgb, Point(max_x, max_y), Point(max_x+max_w-1, max_y+max_h-1),
		CV_RGB(255, 0, 0), 1, 8, 0);
	imshow("img", im_rgb);
	waitKey();
	imwrite(output_prefix+"_roi.jpg", im_rgb);
*/
	return 0;
}
