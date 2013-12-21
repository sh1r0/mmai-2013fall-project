/*!
*	@file		gabor.h
*	@brief		C++ class for general Gabor filters
*	@author		Jianguo Li (leeplus at gmail dot com)
*	@version	3.0 --- Dec, 2006 \n
				+) in-place transformation \n
				+) pre-computing all Gabor filters \n
				+) float type Gabor filters \n
*/

#ifndef __GABOR_HPP
#define __GABOR_HPP

#include <vector>
#include "opencv/cv.h"
#include "fftw3.h"
#include "gaborUtil.h"

// #pragma comment(lib, "fftw3f")

typedef struct tagGaborParam
{
	int nScale;			/// Number of wavelet scales, default 3 scales
	int nOrient;		/// Number of filter orientations, default 8 orientations
	real nMinWavLen;	/// Wavelength of smallest scale filter, at least 4, default 4 pixels

	real rScale;		/// scale between successive filter banks, default sqrt(2)
	real rOverlap;		/// the overlap coefficient between successive filters, default 0.618
}cvGaborParam;

/// set default parameters for Gabor filters
inline void setGaborDefaultParam(cvGaborParam& gabor_param)
{
	gabor_param.nScale = 3;
	gabor_param.nOrient = 8;
	gabor_param.nMinWavLen = 4;		// at least 4 pixels

	gabor_param.rScale = 1.4142;	// sqrt(2)
	gabor_param.rOverlap = 0.618;	// should be in the range of (0,1)
}

class CvGabor
{
public:
	/// constructor
	CvGabor();

	/// destructor
	~CvGabor();

public:
	/*!
		create a shared plan for Gabor filter \n
		the input 2D signals or images of the same size can use one plan forever

		@param nImgWidth -- width of the image
		@param nImgHeight -- width of the image
		@param gab_param -- parameters of the Gabor filters
		@return number of filter-banks
	*/
	int createPlan(int nImgWidth, int nImgHeight, cvGaborParam* gab_param = NULL);

	/*!
		execute the plan for a given image
		@param img -- input of the OpenCV IplImage format 
	*/
	void executPlan(IplImage* img);

	/*!
		execute the plan for a given matrix
		@param img -- input of the OpenCV CvMat format 
	*/
	void executPlan(CvMat* img);

	/// destroy the plan
	void destroyPlan();

	/// export pointer of gaborMagnitude (i.e., pointer of each row of m_gabMag)
	void exportGaborMagnitue(float** gaborMag);

private:
	/// pre-computing kernel of Gabor filters
	void PreComputGaborKernel(int nw, int nh);

private:
	/// Gabor parameters
	real m_kapa;	/// a constant related to the bandwidth
	real m_rDC;	/// DC response compensating term
	cvGaborParam m_gabor_param;

	/// Gabor magnitude and phase
	std::vector<rarray>  m_gabMag;		/// magnitude
	std::vector<rarray>  m_gabPha;		/// phase

	//////////////////////////////////////////////////////////////////////////
	/// the following parameters are for shared plan execution
	//////////////////////////////////////////////////////////////////////////
	int m_nh;	/// height
	int m_nw;	/// width
	int m_nFB;	/// number of filter banks

	/// kernels of Gabor filters in freq-domain
	std::vector<rarray> m_gaborKernel;

	/// convolution by FFT in the freq-domain
	/// in-place transformation, both before and after FFT
	fftwf_complex *m_imgfftConv;

	/// the FFTW plan for inverse FFT
	fftwf_plan m_invplan;

	// image FFT variables
	real *m_imgin;
	fftwf_complex *m_imgfft;
	fftwf_complex *m_fftwOut;
	fftwf_plan m_fwplan;
};

#endif
