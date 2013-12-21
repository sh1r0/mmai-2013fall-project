#include <algorithm>
#include "LogGabor.h"
#include "opencv/highgui.h"

CvLogGabor::CvLogGabor()
{
	m_nFB = 0;
	m_nh = 0;
	m_nw = 0;
}

CvLogGabor::~CvLogGabor()
{
	destroyPlan();
}

// Pre-compute Gabor Kernels
void CvLogGabor::PreComputGaborKernel(int nw, int nh)
{
	int i, j, idx;
	int nw2 = nw/2 + 1;
	int nh2 = nh/2 + 1;
	int sz = nw * nh;

	real *sinTheta = (real *)cvAlloc(sizeof(real) * sz);
	real *cosTheta = (real *)cvAlloc(sizeof(real) * sz);
	real *radius = (real *)cvAlloc(sizeof(real) * sz);
	real *spread = (real *)cvAlloc(sizeof(real) * sz);

	// temporal variable
	real x, y, theta;
	int negnw2 = -nw/2;
	int negnh2 = -nh/2;
	real invnw = 1.0/nw;
	real invnh = 1.0/nh;

	// in-place computing
	int offset;
	for(j=0; j<nh; j++) // rows	
	{
		offset = j*nw;
		y = (negnh2 + j) * invnh;
		for(i=0; i<nw; i++) // cols
		{
			x = (negnw2 + i) * invnw;			
			theta = atan2(-y, x);

			idx = i + offset;			
			sinTheta[idx] = sin(theta);
			cosTheta[idx] = cos(theta);
			radius[idx] = 0.5 * log(x*x + y*y);
		}
	}
	// setting the center to be 1, log(1) = 0
	radius[nw/2 + (nh/2) * nw] = 0;

	//////////////////////////////////////////////////////////////////////////
	real angl, ds, dc, dtheta, cosAngl, sinAngl;
	real val, logWav, wavLength;

	real invThetaSigma2 = g_pi/(m_gabor_param.nOrient * m_gabor_param.rThetaSigma);
	invThetaSigma2 = -0.5/(invThetaSigma2 * invThetaSigma2);
	real invLogSigmaOn2 = log(m_rBandSigma);
	invLogSigmaOn2 = -0.5/(invLogSigmaOn2 * invLogSigmaOn2);

	real* pKernel;
	m_gaborKernel.resize(m_nFB);
	for(int i=0; i<m_gabor_param.nOrient; i++) // orientation
	{
		wavLength = m_gabor_param.nMinWavLen;

		// computing spread
		angl = (i*g_pi)/m_gabor_param.nOrient;
		cosAngl = cos(angl);
		sinAngl = sin(angl);
		for(int k=0; k<sz; k++)
		{
			ds = sinTheta[k] * cosAngl - cosTheta[k] * sinAngl;	// Difference in sine.
			dc = cosTheta[k] * cosAngl + sinTheta[k] * sinAngl; // Difference in cosine
			dtheta = atan2(ds, dc);	 // Absolute angular distance.

			spread[k] = exp( dtheta * dtheta * invThetaSigma2 );
		}

		for(int j=0; j<m_gabor_param.nScale; j++) // scale
		{
			m_gaborKernel[i*m_gabor_param.nScale + j].resize(sz);
			pKernel = &m_gaborKernel[i*m_gabor_param.nScale + j][0];

			// logGabor
			logWav = log(wavLength);
			for(int k=0; k<sz; k++)
			{
				// most time consuming point
				val = radius[k] + logWav;
				pKernel[k] = exp( val * val * invLogSigmaOn2 );
			}
			pKernel[(nw2-1) + (nh2-1) * nw] = 0;

			// logGabor filter
			for(int k=0; k<sz; k++)
			{
				pKernel[k] = pKernel[k] * spread[k];
			}

			// calculate Wavelength of the next level filter
			wavLength = wavLength * m_rMultCoeff;
		}
	}

	cvFree(&sinTheta);
	cvFree(&cosTheta);
	cvFree(&radius);
	cvFree(&spread);
}

// create plan for shared size image, allocate some memory
int CvLogGabor::createPlan(int nImgWidth, int nImgHeight, cvLogGaborParam* gab_param /* =NULL */)
{
	m_nw = nImgWidth;
	m_nh = nImgHeight;
	int sz = m_nw * m_nh;

	if( gab_param != NULL )
		m_gabor_param = *gab_param;
	else
		setLogGaborDefaultParam(m_gabor_param);
	m_nFB = m_gabor_param.nOrient * m_gabor_param.nScale;

	// within 1~4 octave
	if( m_gabor_param.rOctave < 0.5 || m_gabor_param.rOctave > 4 )
	{		
		m_rBandSigma = 0.745;
		m_rMultCoeff = 0.5 * g_pi;
	}
	else
	{
		m_rBandSigma = exp(-0.25 * m_gabor_param.rOctave * sqrt( 2 * log(2.0)));
		if( m_gabor_param.rOctave >= 1.0 )
			m_rMultCoeff = 0.5 * g_pi * m_gabor_param.rOctave;
		else
			m_rMultCoeff = 0.5 * g_pi * (0.745/m_rBandSigma);
	}

	// pre-computing radius and the polar angle of all pixels: m_radius, m_spread
	PreComputGaborKernel(m_nw, m_nh);

	// memory for inverse 2D FFT
	m_imgfftConv = (fftwf_complex *)fftwf_malloc( sizeof(fftwf_complex) * sz);

#ifndef FFTW_IN_PLACE
	m_gaborOut = (fftwf_complex *)fftwf_malloc( sizeof(fftwf_complex) * sz);
	m_invplan = fftwf_plan_dft_2d(m_nh, m_nw, m_imgfftConv, m_gaborOut, FFTW_BACKWARD, FFTW_ESTIMATE);
#else
	m_invplan = fftwf_plan_dft_2d(m_nh, m_nw, m_imgfftConv, m_imgfftConv, FFTW_BACKWARD, FFTW_ESTIMATE);
#endif

	// output feature memory
	m_gabMag.resize(m_nFB);
	for(int i=0; i<m_nFB; i++)
	{
		m_gabMag[i].resize(sz);
	}

	// variables for image FFT
	// the most important is the FFTW output memory layout
	int nw2 = m_nw/2 + 1;
	int fftwSz = m_nh * nw2;
	m_imgin = (real *)cvAlloc( sizeof(real) * sz);
	
	m_fftwOut = (fftwf_complex *)fftwf_malloc( sizeof(fftwf_complex) * fftwSz );
	m_imgfft = (fftwf_complex *)fftwf_malloc( sizeof(fftwf_complex) * sz );	

	// special note: the input should be row major
	m_fwplan = fftwf_plan_dft_r2c_2d(m_nh, m_nw, m_imgin, m_fftwOut, FFTW_ESTIMATE);

	return m_nFB;
}

void CvLogGabor::destroyPlan()
{
	if ( m_nh * m_nw > 0 )
	{
		// release memory
		// free the gabor feature memory
		for(int i=0; i<m_nFB; i++)
		{
			rarray tmp;
			m_gabMag[i].swap(tmp);
		}
		// free the gabor kernel memory
		for(int i=0; i<m_nFB; i++)
		{
			rarray tmp;
			m_gaborKernel[i].swap(tmp);
		}

#ifndef FFTW_IN_PLACE
		fftwf_free(m_gaborOut);
#endif

		fftwf_free(m_fftwOut);
		fftwf_free(m_imgfft);
		fftwf_free(m_imgfftConv);

		cvFree(&m_imgin);
		fftwf_destroy_plan(m_invplan);
		fftwf_destroy_plan(m_fwplan);

		m_nFB = 0;
		m_nh = 0;
		m_nw = 0;
	}
}

void CvLogGabor::executePlan(IplImage* img)
{
	CvMat* matImg = cvCreateMat(m_nh, m_nw, CV_32F);
	cvConvert(img, matImg);
	m_nType = CV_32F;

	executePlan(matImg);
	cvReleaseMat(&matImg);
}

// now full compatible with the MATLAB version log-Gabor filter
void CvLogGabor::executePlan(CvMat* img)
{	
	int nw = img->width;
	int nh = img->height;
	int sz = nw * nh;

	if( nw != m_nw || nh != m_nh )
		return;

	m_nType = cvGetElemType(img);
	if( m_nType != CV_32F && m_nType != CV_32FC1 )
	{
		m_nType = CV_32F;
		CvMat* matImg = cvCreateMat(nh, nw, CV_32F);
		cvConvert(img, matImg);

		cvReleaseMat(&img);		
		img = cvCloneMat(matImg);
	}

	//S1: 2D FFT transform the input image to frequency domain
	// initialize the input data after creating the plan
	icvImag2RealArray(img, m_imgin);

	// execute the FFT plan
	fftwf_execute(m_fwplan);

	// transform from FFTW Hermitian symmetry to real two dimensional format
	icvHermitOut2Array(m_fftwOut, m_imgfft, m_nw, m_nh);

	// shift FFT image with the same phase of the Gabor filter
	icvFFT2Shift(m_imgfft, m_nw, m_nh);

	// S2: start computing Log-Gabor filter in frequency domain
	real *pKernel;
	for(int i=0; i<m_nFB; i++)
	{
		pKernel = &m_gaborKernel[i][0];

		// do the convolution:
		// note in the filter-bank, the order is orientation x scale (continued store same orientation)
		for(int k=0; k<sz; k++)
		{
			m_imgfftConv[k][0] = m_imgfft[k][0] * pKernel[k];
			m_imgfftConv[k][1] = m_imgfft[k][1] * pKernel[k];
		}

		// back FFT transform, and save the result in m_gaborOut			
		fftwf_execute(m_invplan);

		// computing the magnitude
		pKernel = &m_gabMag[i][0];

		for(int k=0; k<sz; k++)
		{
#ifndef FFTW_IN_PLACE
			pKernel[k] = sqrt(m_gaborOut[k][0] * m_gaborOut[k][0] + m_gaborOut[k][1] * m_gaborOut[k][1]);
#else
			pKernel[k] = sqrt(m_imgfftConv[k][0] * m_imgfftConv[k][0] + m_imgfftConv[k][1] * m_imgfftConv[k][1]);
#endif
		}
	} // end i
}

void CvLogGabor::exportGaborMagnitue(float** gaborMag)
{
	for(int i=0; i<m_nFB; i++)
		gaborMag[i] = (float *)(&(m_gabMag[i][0]));
}

