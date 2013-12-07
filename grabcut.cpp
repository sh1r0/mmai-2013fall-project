#include "opencv2/opencv.hpp"
#include <iostream>
 
using namespace cv;
using namespace std;

Mat img0, img1;
Point anchor;
int drag = 0;

// return i in [a, b]
int bound(short i, short a, short b)
{
    return min(max(i,min(a,b)),max(a,b));
}

void mouseHandler(int event, int x, int y, int flags, void* param)
{
    x = bound(x, 0, img0.cols-1);
    y = bound(y, 0, img0.rows-1);

    /* user press left button */
    if (event == CV_EVENT_LBUTTONDOWN && !drag) {
        anchor = Point(x, y);
        drag  = 1;
    }

    /* user drag the mouse */
    if (event == CV_EVENT_MOUSEMOVE && drag) {
        img1 = img0.clone();

        rectangle(img1, anchor, Point(x, y), CV_RGB(255, 0, 0), 1, 8, 0);

        imshow("img", img1);
    }

    /* user release left button */
    if (event == CV_EVENT_LBUTTONUP && drag) {
        Point temp(anchor);
        anchor.x = min(x, temp.x);
        x = max(x, temp.x);
        anchor.y = min(y, temp.y);
        y = max(y, temp.y);
        Rect rect(anchor.x, anchor.y, x - anchor.x, y - anchor.y);

        // draw rectangle on original image
        img1 = img0.clone();
        rectangle(img1, rect, CV_RGB(0, 255, 0), 1);
        imshow("img", img1);

        Mat result; // segmentation result (4 possible values)
        Mat bgModel,fgModel; // the models (internally used)
     
        // GrabCut segmentation
        grabCut(img0,    // input image
            result,   // segmentation result
            rect,// rectangle containing foreground
            bgModel, fgModel, // models
            1,        // number of iterations
            GC_INIT_WITH_RECT); // use rectangle

        // Get the pixels marked as likely foreground
        compare(result, GC_PR_FGD, result, CMP_EQ);
        
        // Generate output image
        Mat foreground(img0.size(), CV_8UC3);
        img0.copyTo(foreground, result); // bg pixels not copied
     
        // display result
        namedWindow("Segmented Image");
        imshow("Segmented Image", foreground);
        imwrite("grabcut.png", foreground(rect));
        
        drag = 0;
    }

    /* user click right button: reset all */
    if (event == CV_EVENT_RBUTTONUP) {
        destroyWindow("Segmented Image");
        imshow("img", img0);
        drag = 0;
    }
}

int main(int argc, char* argv[])
{    
    if (argc < 2) {
        cerr << "[ERROR] no input image" << endl;
        return -1;
    }

    img0 = imread(argv[1]);

    if (!img0.data ) {
        cerr <<  "[ERROR] Could not open or find the image" << endl ;
        return -1;
    }
 
    namedWindow("img", 1);
    setMouseCallback("img", mouseHandler, NULL);
    imshow("img", img0);
    waitKey();
    return 0;
}