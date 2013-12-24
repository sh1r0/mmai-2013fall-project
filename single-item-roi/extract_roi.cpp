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

	Mat im_bw = im_gray < 254;
	medianBlur(im_bw, im_bw, 7);

	unsigned char *mask = im_bw.data;
	int height = im_bw.rows;
	int width = im_bw.cols;

	int *dp_table = new int[height*width];
	for (int i = 0; i < width; ++i) {
		dp_table[i] = mask[i] > 127;
		for (int j = 1; j < height; j++) {
			dp_table[j*width+i] = (mask[j*width+i]) ? dp_table[(j-1)*width+i]+1 : 0;
		}
	}

	stack<elem> rect_stack;
	int max_area = 0, max_x, max_y, max_w, max_h;
	elem left;

	for (int i = height-1; i >= height/2; i--) {
		for (int j = 0; j < width; j++) {
			int h = dp_table[i*width+j];
			if (rect_stack.empty()) {
				if (h > 0) {
					rect_stack.push(elem(j, h));
				}
			} else if (h > rect_stack.top().height) {
				rect_stack.push(elem(j, h));
			} else {
				while (h < rect_stack.top().height) {
					left = rect_stack.top();
					int _w = j - left.position;
					int _h = left.height;
					int area = _w*_h;

					if (area > max_area && left.position < (width>>1) && (_w > (width>>1) || (_h > (height>>1)))) {
						max_area = area;
						max_x = left.position;
						max_y = i - _h + 1;
						max_w = _w;
						max_h = _h;
					}

					rect_stack.pop();

					if (rect_stack.empty()) {
						if (h > 0) {
							rect_stack.push(elem(left.position, h));
						}
						break;
					} else if (h > rect_stack.top().height) {
						rect_stack.push(elem(left.position, h));
						break;
					}
				}
			}
		}

		while (!rect_stack.empty()) {
			left = rect_stack.top();
			int _w = width - left.position;
			int _h = left.height;
			int area = _w*_h;

			if (area > max_area && left.position < (width>>1) && (_w > (width>>1) || (_h > (height>>1)))) {
				max_area = area;
				max_x = left.position;
				max_y = i - _h + 1;
				max_w = _w;
				max_h = _h;
			}

			rect_stack.pop();
		}
	}

	int x, y, h, w;
	if (max_w*3 > max_h) {
		w = 0.8 * min(max_w, max_h);
		h = w;
		x = max_x + 0.5 * max_w - 0.5 * w;
		y = max_y + 0.5 * max_h - 0.5 * h;
	} else {
		x = max_x + 0.05 * max_w;
		y = max_y + 0.1 * max_h;
		w = 0.9 * max_w;
		h = 0.4 * max_h;
	}
	Rect roi = Rect(x, y, w, h);
	imwrite(output_prefix+"_patch.jpg", im_rgb(roi));

	// output images for debugging
	// Mat temp;
	// im_rgb.copyTo(temp, im_bw);
	// rectangle(temp, Point(x, y), Point(x + w - 1, y + h - 1), CV_RGB(0, 255, 0), 1, 8, 0);
	// rectangle(temp, Point(max_x, max_y), Point(max_x+max_w-1, max_y+max_h-1), CV_RGB(255, 0, 0), 1, 8, 0);
	// imwrite(output_prefix+"_roi.jpg", temp);

	return 0;
}
