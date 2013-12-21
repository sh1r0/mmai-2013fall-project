#include <algorithm>
#include <math.h>

#include "gabor.h"

CvGabor::CvGabor()
{
	m_nFB = 0;
	m_nh = 0;
	m_nw = 0;
}

CvGabor::~CvGabor()
{
	destroyPlan();
}

// Pre-compute of Gabor filter banks: non anti-aliasing implementation
void CvGabor::PreComputGaborKernel(int nw, int nh)
{
	int i, j, k, offset;
	int sz = nw * nh;
	int nScl = m_gabor_param.nScale;
	int nOrt = m_gabor_param.nOrient;

	real* spread = (real*)cvAlloc(sz * sizeof(real));
	memset(spread, 0, sizeof(real) * sz);

	// temporal variable
	real u, v, u1, v1;
	real *pKernel;

	int negnw2 = -nw/2;
	int negnh2 = -nh/2;
	// for numeric stableness, people can change 1.0 to 2.0 or 0.5
	real invnw = 1.0f/nw;
	real invnh = 1.0f/nh;

	real scale_k = m_gabor_param.rScale;
	// filter-overlap ratio (p=exp(- ( gamma*pi * (k-1)/(k+1))^2))
	real cross_ratio = m_gabor_param.rOverlap;
	if( cross_ratio <=0 || cross_ratio >= 1.0 )
		cross_ratio = 0.618f;

	// gamma, eta: the sharpness of the filter along two axes
	// gamma and eta are determined by Ville Kyrki's thesis
	// please refer to Ville Kyrki's thesis for details		
	real gamma = (1.0f/g_pi) * sqrt( -log(cross_ratio) ) * (scale_k+1)/(scale_k-1);
	real eta = (1.0f/g_pi) * sqrt( -log(cross_ratio) ) / tan(0.5*g_pi/nOrt );
	real coeff = g_pi/(gamma * eta);

	// in-place computing of Gabor filters in freq-domain
	real wavLen, fi;
	real sqr_pi = g_pi * g_pi;

	real theta, cosAng, sinAng;
	m_gaborKernel.resize(m_nFB);

	for(i=0; i<nOrt; ++i) // orientation
	{
		wavLen = m_gabor_param.nMinWavLen;
		// when theta = pi/2; filters in this orientation will be of very small value
		theta = (i * g_pi)/nOrt;

		cosAng = cos(theta);
		sinAng = cos(theta);

		// final kernels
		for(j=0; j<nScl; ++j) // scale
		{
			fi = 1.0f/wavLen;

			real alpha = (gamma * gamma)/(fi * fi);
			real beta = (eta * eta)/(fi * fi);

			// computing spread
			for(int y=0; y<nh; ++y) // rows
			{
				offset = y * nw;
				u = (negnh2 + y) * invnh;

				for(int x=0; x<nw; ++x) // cols
				{
					v = (negnw2 + x) * invnw;

					u1 = u*cosAng + v * sinAng;
					v1 = -u*sinAng + v * cosAng;

					// the critical point
					spread[offset + x] = alpha * (u1 - fi)*(u1 - fi) + v1*v1 * beta;
				}
			} // end y

			m_gaborKernel[i*nScl + j].resize(sz);
			pKernel = &(m_gaborKernel[i*nScl + j][0]);

			// Gabor filter in freq-domain
			for(k=0; k<sz; k++)
				pKernel[k] = coeff * exp( -sqr_pi * spread[k] );

			// calculate Wavelength of next filter
			wavLen = wavLen * m_gabor_param.rScale;
		} // end j
	} // end i 
	cvFree(&spread);
}

// create plan for shared size image, allocate some memory
int CvGabor::createPlan(int nImgWidth, int nImgHeight, cvGaborParam* gab_param /* =NULL */)
{
	m_nw = nImgWidth;
	m_nh = nImgHeight;
	int sz = m_nw * m_nh;

	if( gab_param != NULL )
		m_gabor_param = *gab_param;
	else
		setGaborDefaultParam(m_gabor_param);
	m_nFB = m_gabor_param.nOrient * m_gabor_param.nScale;
	
	real rOctave = 1.0f;
	m_kapa = pow(2.0f, rOctave);
	m_kapa = sqrt( 2 * log(2.0)) * (m_kapa +1)/(m_kapa - 1);

	// Direct component for Gabor: kapa * omiga0
	m_rDC = exp( -0.5 * m_kapa * m_kapa );

	// pre-computing some parameters and allocate some memory for shared computing
	// the polar angle of all pixels	
	PreComputGaborKernel(m_nw, m_nh);

	// memory for inverse 2D FFT
	m_imgfftConv = (fftwf_complex *)fftwf_malloc( sizeof(fftwf_complex) * sz);
	// in-place transformation
	m_invplan = fftwf_plan_dft_2d(m_nh, m_nw, m_imgfftConv, m_imgfftConv, FFTW_BACKWARD, FFTW_ESTIMATE);
	
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

void CvGabor::destroyPlan()
{
	if ( m_nh * m_nw > 0 )
	{
		// release memory
		fftwf_free(m_imgfftConv);
		fftwf_destroy_plan(m_invplan);

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

		fftwf_destroy_plan(m_fwplan);		
		fftwf_free(m_fftwOut);
		fftwf_free(m_imgfft);

		cvFree(&m_imgin);

		m_nFB = 0;
		m_nh = 0;
		m_nw = 0;
	}
}

void CvGabor::executPlan(IplImage* img)
{
	CvMat* matImg = cvCreateMat(m_nh, m_nw, CV_32F);
	cvScale(img, matImg);

	executPlan(matImg);
	cvReleaseMat(&matImg);
}

// now full compatible with the MATLAB version Gabor filter
void CvGabor::executPlan(CvMat* img)
{
	int nw = img->width;
	int nh = img->height;
	int sz = nw * nh;

	if( nw != m_nw || nh != m_nh )
		return;

	int type = cvGetElemType(img);
	if( type != CV_32F && type != CV_32FC1 )
	{
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

	// S2: start computing Gabor filter in frequency domain
	real *pKernel;
	for(int i=0; i<m_nFB; i++)
	{
		pKernel = &(m_gaborKernel[i][0]);

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
		pKernel = &(m_gabMag[i][0]);
		for(int k=0; k<sz; k++)
		{
			pKernel[k] = sqrt(m_imgfftConv[k][0] * m_imgfftConv[k][0] + m_imgfftConv[k][1] * m_imgfftConv[k][1]);
		}
	}
}

void CvGabor::exportGaborMagnitue(float** gaborMag)
{
	for(int i=0; i<m_nFB; i++)
		gaborMag[i] = (float *)(&(m_gabMag[i][0]));
}
