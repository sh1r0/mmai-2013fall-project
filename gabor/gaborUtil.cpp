#include <algorithm>
#include "gaborUtil.h"

// input store by row-major order
// in should be outside initialized
void icvImag2RealArray(CvMat *imga, real* in)
{
	int nw = imga->width;
	int nh = imga->height;
	int offset;

	for(int y=0; y < nh; y++) // rows
	{
		offset = y*nw;
		for(int x=0; x < nw; x++) // cols
		{
			in[x + offset] = cvmGet(imga, y, x);
		}
	}
}

// transform a reduced 2d hermit symmetry array to a full 2d array
void icvHermitOut2Array(fftwf_complex* out, fftwf_complex* ret, int nw, int nh)
{
	int nw2, nh2;
	int conjx, conjy;

	nw2 = nw/2 + 1;
	nh2 = nh/2 + 1;

	real invsz = 1.0/(nw * nh);
	int offset, offsetnw2;
	for(int y=0; y < nh; y++)
	{
		offset = y * nw;
		offsetnw2 = y * nw2;
		for(int x=0; x < nw2; x++)
		{
			// the zero col is 
			if( x == 0 )
			{
				conjx = 0;
				conjy = 2 * (nh2-1) - y;
			}

			if( y == 0 )
			{
				conjy = 0;
				conjx = 2 * (nw2-1) - x;
			}

			if( x != 0 && y != 0 )
			{
				conjx = 2 * (nw2-1) - x;
				conjy = 2 * (nh2-1) - y;
			}

			// first half
			ret[x + offset ][0] = out[x + offsetnw2 ][0] * invsz;
			ret[x + offset ][1] = out[x + offsetnw2 ][1] * invsz;

			// 2nd half
			ret[conjx + conjy * nw ][0] =  out[x + offsetnw2 ][0] * invsz;
			ret[conjx + conjy * nw ][1] = -out[x + offsetnw2 ][1] * invsz;
		}
	}
}

// 2D fftshift for complex data
void icvFFT2Shift(fftwf_complex* out, int nw, int nh)
{
	int x, y, newx, newy;
	int nh2, nw2;
	nh2 = nh/2;
	nw2 = nw/2;

	int idx1, idx2;
	int offset, offsetnew;
	for(y = 0; y< nh2; y++)
	{
		newy = y + nh2;
		offset = y * nw;
		offsetnew = newy * nw;

		// the top-left block to bottom-right
		for(x = 0; x<nw2; x++)
		{
			newx = x + nw2;

			idx1 = x + offset;
			idx2 = newx + offsetnew;

			std::swap(out[idx1][0], out[idx2][0]);
			std::swap(out[idx1][1], out[idx2][1]);
		}

		// top-right to bottom-left block		
		for(x = nw2; x<nw; x++)
		{
			newx = x - nw2;

			idx1 = x + offset;
			idx2 = newx + offsetnew;

			std::swap(out[idx1][0], out[idx2][0]);
			std::swap(out[idx1][1], out[idx2][1]);
		}
	}
}

void icvCalcMomentStat(real* points, int sz, real& vmu, real& vstd)
{
	vmu = 0;
	vstd = 0;
	real a;
	for(int i=0; i<sz; i++)
	{
		a = points[i];
		vmu += a;
		vstd += a * a;
	}
	vmu = vmu/sz;
	a = vmu*vmu;

	vstd = fabs(vstd/sz - a);
	vstd = sqrt(vstd);
}
