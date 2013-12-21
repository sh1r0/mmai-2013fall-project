/*!
*	@file		gaborUtil.h
*	@brief		utility function for Gabor transformation
*	@author		Jianguo Li (leeplus at gmail dot com)
*	@date		Oct, 2006
*/
#ifndef __GABOR_UTIL_HPP
#define __GABOR_UTIL_HPP

#include <vector>
#include "opencv/cv.h"
#include "fftw3.h"

#pragma warning (disable: 4018 4244 4305)

typedef float real;

typedef std::vector<int>	intArray;
typedef std::vector<real>	rarray;
typedef unsigned char	BYTE;

const double g_pi = 3.14159265358979;

#ifdef _MSC_VER
	#define NOMINMAX
	#include <minmax.h>
#endif

//////////////////////////////////////////////////////////////////////////
///		auxiliary functions for 2D FFT
//////////////////////////////////////////////////////////////////////////

/*!
	transform an 2D mat to a 1D array \n
	input store by row-major order, in should be outside initialized
*/
void icvImag2RealArray(CvMat *imga, real* in);

/// transform a reduced hermit symmetry matrix to a full 2D array, for FFTW
void icvHermitOut2Array(fftwf_complex* out, fftwf_complex* ret, int nw, int nh);

/// 2D FFTShift on complex data
void icvFFT2Shift(fftwf_complex* out, int nw, int nh);

//////////////////////////////////////////////////////////////////////////
///		statistics feature for given Gabor response
//////////////////////////////////////////////////////////////////////////

/*!
	calculating the first two order moment for a given array, i.e, mean and standard deviation
	@param points -- input array
	@param sz -- the size of the input array
	@param vmu -- first statistics, mean value
	@param vstd -- second statistics, std deviation value
*/
void icvCalcMomentStat(real* points, int sz, real& vmu, real& vstd);

#endif
