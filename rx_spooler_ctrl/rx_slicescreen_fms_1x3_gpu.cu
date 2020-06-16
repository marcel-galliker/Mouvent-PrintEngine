// ****************************************************************************
//
//	rx_slicescreen_fms_1x3.c
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

//--- includes ---------------------------------------------------------------------
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#ifdef __cplusplus
extern "C"{
#endif

#include "rx_error.h"
#include "rx_trace.h"
#include "rx_threads.h"
#include "rx_mem.h"
#include "rx_slicescreen.h"
#include "rx_slicescreen_fms_1x3_gpu.h"

#ifdef linux
	#include "errno.h"
#endif

//--- defines ----------------------------------------------------------
#define TA_WIDTH	256
#define TA_HEIGHT	256

//	time per color and scan of "Tiger-Woman" image (~full width) on "GeForce GT 1030"
//	int		sectors=1;		// 300 ms
//	int		sectors=4;		// 130 ms
//	int		sectors=8;		// 83 ms
//	int		sectors=16;		// 60 ms
#define	SECTORS 32			// 30 ms

#define STREAM_CNT	4

typedef struct
{
	cudaStream_t	stream;

	UINT32			in_size;
	BYTE*			in;

	UINT32			out_size;
	BYTE*			out;

	UINT16*			ta;
	UINT16*			df;	// density_factor

	int				time[4];
	int				Time[3];
} SStreamPar;

//--- static variables -------------------------------------------------
static int		_Init=FALSE;

static cudaDeviceProp	_GpuProp;
static int				_GPU_Present=-1;

static int				_Time[3];

static SStreamPar _GPU_Stream[STREAM_CNT];

//--- prototypes ----------------------------------------------------
static int _gpu_malloc(SSLiceInfo *inplane, SSLiceInfo *outplane, int bitsPerPixel, SStreamPar *par);

//--- gpu_is_board_present -----------------------------
int gpu_is_board_present(void)
{
	if (_GPU_Present<0)
	{
		cudaGetDeviceProperties(&_GpuProp, 0);
		if (_GpuProp.name[0]) TrPrintf(TRUE, "GPU: %s", _GpuProp.name);
		else				  TrPrintf(TRUE, "GPU: not present");	
		_GPU_Present = (_GpuProp.name[0]!=0);
	}
	return _GPU_Present;
}

//--- gpu_init -------------------------------
int gpu_init(void)
{
	if (!_Init)
	{
		_Init = TRUE;
		memset(_GPU_Stream, 0, sizeof(_GPU_Stream));
		_GPU_Present = gpu_is_board_present();
		if (FALSE && _GPU_Present)
		{
			_GPU_Present = FALSE;
			Error(WARN, 0, "Disable GPU for tests");
		}
	}
	Error(LOG, 0, "Screening: GPU=>>%s<<", _GpuProp.name);

	if (_GPU_Present) return REPLY_OK;
	else return REPLY_ERROR;
}

//--- gpu_stream_cnt -------------------
int gpu_stream_cnt(void)
{
	return STREAM_CNT;
}

//--- _gpu_malloc -----------------------------------------------------------
static int _gpu_malloc(SSLiceInfo *inplane, SSLiceInfo *outplane, int bitsPerPixel, SStreamPar *par)
{
	int ret=REPLY_OK;

	if (_GPU_Present)
	{
		if (par->stream==NULL) 
		{
			ret=cudaStreamCreate(&par->stream);
			if (ret!=cudaSuccess) Error(ERR_ABORT, 0, "could not create CUDA stream, cudaError=%d", ret);
		}

		if (inplane->dataSize>par->in_size)
		{
			par->in_size = (UINT32)inplane->dataSize;
			if (par->in) ret=cudaFree(par->in);
			ret=cudaMalloc(&par->in, par->in_size);
		}

		if (outplane->dataSize>par->out_size)
		{
			par->out_size = (UINT32)outplane->dataSize;
			if (par->out) ret=cudaFree(par->out);
			ret=cudaMalloc(&par->out, par->out_size);
		}

		if (!par->ta) cudaMalloc(&par->ta, TA_WIDTH*TA_HEIGHT*sizeof(UINT16));
		if (!par->df) cudaMalloc(&par->df, JET_CNT*TA_HEIGHT*sizeof(UINT16));
	}
	return ret;
}

//--- _screen_fms_sml_kernel ----------------------
__global__ void _screen_fms_sml_kernel(UINT8 *in, UINT8 *out, UINT16 *pta, UINT16 *densityFactor, UINT32 lineLen, UINT32 height, UINT32 sectorWidth, UINT32 limitM, UINT32 limitL)
{
	// creates 2 bit/pixel
	// if limitL == 0: output = (s,m) else output = (s,m,l)

	int y = (blockDim.x * blockIdx.x + threadIdx.x)*sectorWidth;
	int x;
	UINT8 *pSrc=&in[y];
	UINT8 *pDst=&out[y/4];
	x = y%lineLen;
	y = y/lineLen;
	UINT16* taLine=&pta[(y%TA_HEIGHT)*TA_WIDTH];
	UINT8  dst;
	UINT32 src;	// need 32 bits for compensating disabled jets
	UINT16  ta;

	if (x+sectorWidth<lineLen) sectorWidth=x+sectorWidth;
	else                       sectorWidth=lineLen;

	if (y<height)
	{
		while (x<sectorWidth)
		{
			src = *pSrc++ * densityFactor[x];
			ta  = taLine[x%TA_WIDTH];
			dst <<= 2;
			if (limitL && src > limitL + ta)	dst |= 0x03;
			else if (src > limitM + ta)			dst |= 0x02;
			else if (src > ta)					dst |= 0x01;

			if (!(++x & 3)) *pDst++=dst;
		}
	}
}

//--- _screen_fms_sml ----------------------
static void _screen_fms_sml(UINT32 y, UINT8 *in, UINT8 *out, UINT16 *pta, UINT16 *densityFactor, UINT32 lineLen, UINT32 height, UINT32 sectorWidth, UINT32 limitM, UINT32 limitL)
{
	// creates 2 bit/pixel
	// if limitL == 0: output = (s,m) else output = (s,m,l)

	UINT32 x;
	UINT8 *pSrc=&in[y];
	UINT8 *pDst=&out[y/4];
	x = y%lineLen;
	y = y/lineLen;
	UINT16* taLine=&pta[(y%TA_HEIGHT)*TA_WIDTH];
	UINT8  dst=0;
	UINT32 src;	// need 32 bits for compensating disabled jets
	UINT16  ta;

	if (x+sectorWidth<lineLen) sectorWidth=x+sectorWidth;
	else					   sectorWidth=lineLen;

	if (y<height)
	{
		while (x<sectorWidth)
		{
			src = *pSrc++ * densityFactor[x];
			ta  = taLine[x%TA_WIDTH];
			dst <<= 2;
			if (limitL && src > limitL + ta)	dst |= 0x03;
			else if (src > limitM + ta)			dst |= 0x02;
			else if (src > ta)					dst |= 0x01;

			if (!(++x & 3)) *pDst++=dst;
		}
	}
}

//--- gpu_blk_size ---------------------------
int gpu_blk_size(void)
{
	return MAX_DENSITY_FACTORS / SECTORS;
}

//--- gpu_screen_FMS_1x3g ----------------------------------------------------------------------
int gpu_screen_FMS_1x3g(SSLiceInfo *inplane, SSLiceInfo *outplane, void *epplaneScreenConfig, const char *dots, int threadNo)
{
	SPlaneScreenConfig* pplaneScreenConfig = (SPlaneScreenConfig*)epplaneScreenConfig;
	UINT16 limitM, limitL;

	SStreamPar	*pstream = &_GPU_Stream[threadNo];

	if (strchr(dots, 'L'))
	{
		limitM = 0x10000 * 33 / 100;
		limitL = 0x10000 * 66 / 100;
	}
	else
	{
		limitM = 0x10000 * 50 / 100;
		limitL = 0;
	}

	if (pplaneScreenConfig->limit[0]) limitM = 65536 * pplaneScreenConfig->limit[0] / 100;
	if (pplaneScreenConfig->limit[1]) limitL = 65536 * pplaneScreenConfig->limit[1] / 100;
	
	if (pplaneScreenConfig->TA->width!=TA_WIDTH || pplaneScreenConfig->TA->heigth!=TA_HEIGHT) return Error(ERR_ABORT, 0, "TA-ARRAY must be 256*256");

	if (_GPU_Present)
	{
	//	TrPrintfL(TRUE, "GPU[%d].malloc start", threadNo);
		_gpu_malloc(inplane, outplane, pplaneScreenConfig->outputbitsPerPixel, pstream);
	//	TrPrintfL(TRUE, "GPU[%d].malloc done", threadNo);
		pstream->time[0]=rx_get_ticks();
		cudaMemcpyAsync(pstream->in, inplane->buffer,				   inplane->lengthPx * inplane->lineLen,		cudaMemcpyHostToDevice, pstream->stream);
		cudaMemcpyAsync(pstream->ta, pplaneScreenConfig->TA->ta16,	   TA_WIDTH*TA_HEIGHT*sizeof(UINT16),			cudaMemcpyHostToDevice, pstream->stream);
		cudaMemcpyAsync(pstream->df, pplaneScreenConfig->densityFactor, sizeof(pplaneScreenConfig->densityFactor),	cudaMemcpyHostToDevice, pstream->stream);
		cudaStreamSynchronize(pstream->stream);
		pstream->time[1]=rx_get_ticks();
	//	TrPrintfL(TRUE, "GPU[%d].memcpy(in) done", threadNo);
//		cudaMemset(_gpu_out, 0x01, outplane->lengthPx * outplane->lineLen);

		// Each Grid consists of X * Y blocks that each contain 512 * 512 threads. Unused threads will just do nothing. (check inside kernel, see code above)
		int sectorWidth		= gpu_blk_size();
		int threadsPerBlock = 32; //_GpuProp.maxThreadsPerBlock;
		int blocksPerGrid   = (((inplane->lengthPx*inplane->lineLen)/sectorWidth) + threadsPerBlock - 1) / threadsPerBlock;

		_screen_fms_sml_kernel <<<blocksPerGrid, threadsPerBlock, 0, pstream->stream >>> (pstream->in, pstream->out, pstream->ta, pstream->df, inplane->lineLen, inplane->lengthPx, sectorWidth, limitM, limitL);

		cudaStreamSynchronize(pstream->stream);
		pstream->time[2]=rx_get_ticks();
	//	TrPrintfL(TRUE, "GPU[%d].process done", threadNo);

		cudaMemcpyAsync(outplane->buffer, pstream->out, outplane->lengthPx * outplane->lineLen, cudaMemcpyDeviceToHost, pstream->stream);

		cudaStreamSynchronize(pstream->stream);
		pstream->time[3]=rx_get_ticks();
	//	TrPrintfL(TRUE, "GPU[%d].memcpy(out) done", threadNo);

		pstream->Time[0] = pstream->time[1]-pstream->time[0];
		pstream->Time[1] = pstream->time[2]-pstream->time[1];
		pstream->Time[2] = pstream->time[3]-pstream->time[2];
	}
	else
	{
		int time0=rx_get_ticks();
		for (UINT32 y=0; y<inplane->lengthPx; y++)
		{
			_screen_fms_sml(y*inplane->lineLen, inplane->buffer, outplane->buffer, pplaneScreenConfig->TA->ta16, pplaneScreenConfig->densityFactor, inplane->lineLen, inplane->lengthPx, inplane->lineLen, limitM, limitL);
		}
		_Time[0]=0;
		_Time[1]=rx_get_ticks()-time0;
		_Time[2]=0;
	}
	return REPLY_OK;
}

//--- gpu_time ----------------------
int  gpu_time(int no)
{
	if (_GPU_Present)
	{
		return (int)(_GPU_Stream[0].Time[no]+0.5);
	}
	return (int)(_Time[no]+0.5);
}

#ifdef __cplusplus
}
#endif