/*!
*	@file		LogGabor.h
*	@brief		C++ class for Log-Gabor filters
*	@author		Jianguo Li (leeplus at gmail dot com)
*	@version	3.0 --- Dec, 2006 \n
		+) in-place transformation \n
		+) pre-computing all Gabor filters \n
		+) float type Gabor filters \n
*	@version	4.0 --- Jan, 2007 \n
				+) SIMD plus MKL based implementation (removed)
*/

#ifndef __LOG_GABOR_HPP
#define __LOG_GABOR_HPP

#include <vector>
#include "opencv/cv.h"
#include "fftw3.h"
#include "gaborUtil.h"

//#define  FFTW_IN_PLACE
#pragma comment(lib, "fftw3f")

typedef struct tagLogGaborParam
{
	int nScale;			/// Number of wavelet scales, default 3 scales
	int nOrient;		/// Number of filter orientations, default 8 orientations
	int nMinWavLen;		/// Wavelength of smallest scale filter, at least 3, default 4 pixels

	// kapa = exp(-0.25 beta sqrt(2 ln2) ),
	// beta: bandwidth in octave
	// kapa = 0.745 for beta = 1, kapa=0.555 for beta = 2
	real rOctave;		/// band width in octave, in the range of 0.5~4 octave, default 1 octave
	real rThetaSigma;	/// Ratio of angular interval between filter orientations, default 1.7
}cvLogGaborParam;

/// set default parameters for Log-Gabor filters
inline void setLogGaborDefaultParam(cvLogGaborParam& gabor_param)
{
	gabor_param.nScale = 4;
	gabor_param.nOrient = 6;
	gabor_param.nMinWavLen = 4;	// at least 3 pixel, 4 is better

	gabor_param.rOctave = 1.0f;	//
	gabor_param.rThetaSigma = 1.7f;	// best value
}

class CvLogGabor
{
public:
	/// constructor
	CvLogGabor();

	/// destructor
	~CvLogGabor();

public:
	/*!
		create a shared plan for Log-Gabor filter \n
		the input 2D signals or images of the same size can use one plan forever

		@param nImgWidth -- width of the image
		@param nImgHeight -- width of the image
		@param gab_param -- parameters of the Log-Gabor filters
		@return number of filter-banks
	*/
	int createPlan(int nImgWidth, int nImgHeight, cvLogGaborParam* gab_param = NULL);

	/*!
		execute the plan for a given image
		@param img -- input of the OpenCV IplImage format
	*/
	void executePlan(IplImage* img);

	/*!
		execute the plan for a given matrix
		@param img -- input of the OpenCV CvMat format
	*/
	void executePlan(CvMat* img);

	/// destroy the plan
	void destroyPlan();

	/// export pointer of gaborMagnitude (i.e., pointer of each row of m_gabMag)
	void exportGaborMagnitue(float** gaborMag);
private:
	/// pre-computing kernel of Gabor filters
	void PreComputGaborKernel(int nw, int nh);

private:
	/// Gabor parameters
	real m_rMultCoeff;  /// Scaling factor between successive filters
	real m_rBandSigma;  /// Gaussian sigma for octave band, smaller for larger band width

	/// parameters for the Log-Gabor filters
	cvLogGaborParam m_gabor_param;

	/// Gabor magnitude and phase
	std::vector<rarray>  m_gabMag;		/// magnitude
	std::vector<rarray>  m_gabPha;		/// phase

	//////////////////////////////////////////////////////////////////////////
	/// the following parameters are for shared plan execution
	//////////////////////////////////////////////////////////////////////////
	int m_nh;	/// height
	int m_nw;	/// width
	int m_nFB;	/// number of filter bank
	int m_nType;

	/// kernels of Gabor filters
	std::vector<rarray> m_gaborKernel;

	/// convolution by FFT in the freq-domain
	/// in-place transformation, both before and after FFT
	fftwf_complex *m_imgfftConv;

#ifndef FFTW_IN_PLACE
	/// output of Log-Gabor transformation for out-of-place transformation
	fftwf_complex *m_gaborOut;
#endif

	/// the FFTW plan for inverse FFT
	fftwf_plan m_invplan;

	// image FFT variables
	real *m_imgin;
	fftwf_complex *m_fftwOut;
	fftwf_complex *m_imgfft;
	fftwf_plan m_fwplan;
};

#endif
