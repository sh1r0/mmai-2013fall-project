#include "boost/program_options.hpp"
#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "LogGabor.h"

#include <fstream>
#include <string>
#include <cmath>

//using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
	namespace po = boost::program_options;
	po::options_description desc("Options");
	desc.add_options()
		("help,h", "Print help messages")
		("input,i", po::value<string>()->required(),"Specify input file.")
		("output,o", po::value<string>()->default_value("."),"Specify output directory.");

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc),vm);
		if (vm.count("help")) {
			std::cout << "Basic Command Line Parameter App" << std::endl << desc << std::endl;
			return 0;
		}
		po::notify(vm);
	} catch(po::required_option& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}

	string inputfile = vm["input"].as<string>();
	unsigned slash = inputfile.find_last_of("/\\");
	unsigned dot = inputfile.find_last_of(".");
	string name = inputfile.substr(slash+1, dot);

	IplImage *img;
	try {
		img = cvLoadImage(inputfile.c_str(),CV_LOAD_IMAGE_COLOR);
	} catch(cv::Exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	IplImage *gray_img = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
	cvCvtColor(img, gray_img, CV_BGR2GRAY);

	CvLogGabor gaborfilter;
	const int nfilter = gaborfilter.createPlan(gray_img->width,gray_img->height);
	gaborfilter.executePlan(gray_img);

	float** pMag = new float* [nfilter];
    gaborfilter.exportGaborMagnitue(pMag);

	const int dimension = nfilter*2;
	real feature[dimension];
	for (int i = 0; i < nfilter; i++) {
		icvCalcMomentStat((real *)pMag[i], gray_img->width*gray_img->height, feature[2*i], feature[2*i+1]);
	}
	string outputfile = vm["output"].as<string>() + "/" + name + ".gb";
	std::ofstream output;
	output.open(outputfile.c_str());
	for (int i = 0; i < dimension; i++) {
		output << feature[i] << " ";
	}
	output.close();

	return 0;
}
