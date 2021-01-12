//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

//OpenCL Kernel for rx_AlignFilter

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void HistogramFull( __global uchar4 imagein[], __global uint HistoRowList[], uint Width)
{
	//Should be run for every Row
	//HistoRowList contains 256 * Width bins
	//get_global_id(0) => vertical position of this instance

	//Line Start Position
	const uint LineStart = get_global_id(0) * Width;		//the row this instance is checking starts here in the image

	//HistoList Start Position
	const uint HistoPos = get_global_id(0) * 256;			//this instances 256 Histogram bins start here in HistoRowList

	__private uint4 Pixel;
	__private uint Gray;

	//Checking every Pixel in this Row (horizontally)
	for(__private uint i = 0; i < Width; i++)
	{
		//Read Pixel
		Pixel.s0 = (uint)imagein[LineStart + i].s0;
		Pixel.s1 = (uint)imagein[LineStart + i].s1;
		Pixel.s2 = (uint)imagein[LineStart + i].s2;

		//Grayscale Value (0-255)
		Gray = (uint)Pixel.s0 + (uint)Pixel.s1 + (uint)Pixel.s2;
		Gray /= 3;

		//Increment this bin
		HistoRowList[HistoPos + Gray]++;
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void HistogramBlock ( __global uchar4 imagein[], __global uint HistoRowList[], uint Width)
{ 
	//Should be run for every 8th Row
	//HistoRowList contains 256 * Width bins
	//get_global_id(0) => vertical position of this instance

	// getting these 8 values out of a 8x8 matrix
	// - - - - - - - X 
	// - - - - - - X -
	// - - - - - X - -
	// - - - - X - - -
	// - - - X - - - -
	// - - X - - - - -
	// - X - - - - - -
	// X - - - - - - -


	//Line Start Position
	const uint LineStart = get_global_id(0) * Width * 8;		//the row this instance is checking starts here in the image

	//HistoList Start Position
	const uint HistoPos = get_global_id(0) * 256;			//this instances 256 Histogram bins start here in HistoRowList

	__private uint4 Pixel;
	__private uint Gray;

	//Checking every 8th Pixel in this Row (horizontally)
	for(__private uint i = 0; i < Width; i += 8)
	{
		//Read Pixel
		Pixel.s0 = (uint)imagein[LineStart + i].s0 + 
			(uint)imagein[LineStart + i + 1 + (1 * Width)].s0 + 
			(uint)imagein[LineStart + i + 2 + (2 * Width)].s0 + 
			(uint)imagein[LineStart + i + 3 + (3 * Width)].s0 + 
			(uint)imagein[LineStart + i + 4 + (4 * Width)].s0 + 
			(uint)imagein[LineStart + i + 5 + (5 * Width)].s0 + 
			(uint)imagein[LineStart + i + 6 + (6 * Width)].s0 +
			(uint)imagein[LineStart + i + 7 + (7 * Width)].s0; 
		Pixel.s1 = (uint)imagein[LineStart + i].s1 + 
			(uint)imagein[LineStart + i + 1 + (1 * Width)].s1 + 
			(uint)imagein[LineStart + i + 2 + (2 * Width)].s1 + 
			(uint)imagein[LineStart + i + 3 + (3 * Width)].s1 + 
			(uint)imagein[LineStart + i + 4 + (4 * Width)].s1 + 
			(uint)imagein[LineStart + i + 5 + (5 * Width)].s1 + 
			(uint)imagein[LineStart + i + 6 + (6 * Width)].s1 +
			(uint)imagein[LineStart + i + 7 + (7 * Width)].s1; 
		Pixel.s2 = (uint)imagein[LineStart + i].s2 + 
			(uint)imagein[LineStart + i + 1 + (1 * Width)].s2 + 
			(uint)imagein[LineStart + i + 2 + (2 * Width)].s2 + 
			(uint)imagein[LineStart + i + 3 + (3 * Width)].s2 + 
			(uint)imagein[LineStart + i + 4 + (4 * Width)].s2 + 
			(uint)imagein[LineStart + i + 5 + (5 * Width)].s2 + 
			(uint)imagein[LineStart + i + 6 + (6 * Width)].s2 +
			(uint)imagein[LineStart + i + 7 + (7 * Width)].s2; 

		//Grayscale Value (0-255)
		Gray = (uint)Pixel.s0 + (uint)Pixel.s1 + (uint)Pixel.s2;
		Gray /= 24;

		//Increment this bin
		HistoRowList[HistoPos + Gray]++;
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void HistogramRow ( __global uchar4 imagein[], __global uint HistoRowList[], uint Width)
{
	//Should be run for every Row
	//HistoRowList contains 256 * Height bins
	//get_global_id(0) => vertical position of this instance

	//Line Start Position
	const uint LineStart = get_global_id(0) * Width;		//the row this instance is checking starts here in the image

	//HistoList Start Position
	const uint HistoPos = get_global_id(0) * 256;			//this instances 256 Histogram bins start here in HistoRowList

	__private uchar4 Pixel;
	__private uint Gray;

	//Checking every Pixel in this Row (horizontally)
	for(__private uint i = 0; i < Width; i++)
	{
		//Read Pixel
		Pixel = imagein[LineStart + i];

		//Grayscale Value (0-255)
		Gray = (uint)Pixel.s0 + (uint)Pixel.s1 + (uint)Pixel.s2;
		Gray /= 3;

		//Increment this bin
		HistoRowList[HistoPos + Gray]++;
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void JoinHistogram ( __global uint HistoRowList[], __global uint HistoList[], int Height)
{
	//Should be run for each of the 256 bins in HistoRowList
	//HistoRowList contains 256 * Height bins
	//HistoList contains 256 bins
	//get_global_id(0) => bin of this instance

	//The bin this instance processes
	const uint BinPos = get_global_id(0);

	__private uint Sum = 0;

	//summing bins for every position
	for(__private uint i = 0; i < Height; i++)
	{
		//Read Row Value
		__private uint Value = HistoRowList[(i * 256) + BinPos];
		Sum += Value;
	}

	HistoList[BinPos] = Sum;
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void SmoothenHistogram ( __global uint HistoIn[], __global uint HistoOut[], int Width)
{
	//Should be run for each of the 256 bins of Histogram

	//bin this instance is processing
	const int Pos = get_global_id(0);
	//how many bins are we integrating
	const int x = Width;

	//Average +/- Values
	__private uint Value = 0;

	//bins to average not leaving the list
	const uint Istart = (uint)(max(0, (Pos - x)));
	const uint Iend = (uint)(min(255, (Pos + x)));

	//Numbers of bins averaged
	const uint NumBins = Iend - Istart;

	for(__private uint i = Istart; i < Iend; i++)
	{
		Value += HistoIn[i];
	}

	HistoOut[Pos] = Value / NumBins;
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void ShowHistogram ( __global uchar4 ImageIn[], __global uint HistoList[],
								uint MaxVal,
								int Peak_1_Val, int Peak_1_Pos, int Peak_2_Val, int Peak_2_Pos,
								int Threshold, int Width, int Height)
{
	//Should be run for each of the 256 bins of Histogram

	//Active Histogram bin
	const int Pos = get_global_id(0);

	//X-Factor for ImageWidth (so many columns display one Histogram Value)
	const float XFactor = Width / 256;
	//X-Start Position in Image to overlay vertical line
	const uint HistoXPos = convert_uint_sat(XFactor * convert_float(Pos));

	//Y-Factor for ImageHeight (maximum Histogram Value => top of Image)
	__private const float MaxFloat = convert_float(MaxVal);
	__private const float HeightFloat = convert_float_rtp(Height);
	const float YFactor = HeightFloat / MaxFloat;

	//Histogram Value of this position
	uint HistoYValue = convert_uint_sat(YFactor * convert_float_rtp(HistoList[Pos]));
	if(HistoYValue >= Height) HistoYValue = Height - 1;

	//Pixel and overlay Values
	__private uchar4 Pixel = (uchar4)(0, 0, 0, 0);
	__private uchar4 NewPixel = (uchar4)(0, 0, 0, 0);
	__private uchar4 OverlayPixel = (uchar4)(0, 0, 127, 0);

	//Marker for Positions with Threshold and Peak lines
	__private int DrawLine = 0;

	//Draw vertical Lines
	for(__private uint i = 0; i < XFactor; i++)
	{
		//Overlay Histogram Value from Bottom
		for(__private uint j = 0; j < HistoYValue; j++)
		{
			int PixelPos = (HistoXPos + i) + (j * Width);

			Pixel = ImageIn[PixelPos];
			NewPixel = (Pixel / (uchar4)(2,2,2,1)) + OverlayPixel;
			ImageIn[PixelPos] = NewPixel;
		}
		//Rest of the Column
		for(__private int j = HistoYValue; j < Height; j++)
		{
			int PixelPos = (HistoXPos + i) + (j * Width);

			Pixel = ImageIn[PixelPos];
			ImageIn[PixelPos] = Pixel;
		}

		//Overlay Lines for Peak Values
		if(Pos == Peak_1_Pos || Pos == Peak_2_Pos)
		{
			OverlayPixel = (uchar4)(127, 0, 0, 0);
			DrawLine = 1;
		}

		//Overlay Lines for Threshold
		if(Pos == Threshold)
		{
			OverlayPixel = (uchar4)(0, 127, 0, 0);
			DrawLine = 1;
		}

		//Draw Peak and Threshold Lines
		if(DrawLine == 1)
		{
			for(__private int j = 0; j < Height; j++)
			{
				int PixelPos = (HistoXPos + i) + (j * Width);

				Pixel = ImageIn[PixelPos];
				NewPixel = (Pixel / (uchar4)(2,2,2,1)) + OverlayPixel;
				ImageIn[PixelPos] = NewPixel;
			}
		}
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void Binarize ( __global uchar4 imagein[],  __global uchar4 imageout[], const uint Threshold, const uint InverseImage)
{
	//Should be run for every Pixel

	//Get Pixel at Position
	const int Pos = get_global_id(0);
	const uchar4 Pixel = imagein[Pos];

	//Grayscale Value
	const uchar Grey = (uchar)(((uint)Pixel.s0 + (uint)Pixel.s1 + (uint)Pixel.s2) / 3);

	if(InverseImage == 0)
	{
		if(Grey > Threshold)
		{
			imageout[Pos] = 0;
		}
		else
		{
			imageout[Pos] = 255;
		}
	}
	else
	{
		if(Grey > Threshold)
		{
			imageout[Pos] = 255;
		}
		else
		{
			imageout[Pos] = 0;
		}
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void ColorHistogram ( __global uchar4 imagein[], __global uint HistogramArrayBuffer[], uint Height, uint Width, uint Horizontal)
{
	//Should be run for every Column if Lines are vertical, every Row if Lines are horizontal
	//Length is Height if Lines are verical, Width if horizontal (how many Pixels are processed in this Instance)
	//HistogramArrayBuffer contains a Histogram for every Row/Column
	//	organized as Length * 256 bins
	//Horizontal = 1 for horizontal, 0 for vertical

	//Start Position, indicates the first Pixel this Instance handles
	const uint LineStart = get_global_id(0) + (Horizontal * ((get_global_id(0) * Width) - get_global_id(0)));
	
	//HistoArray Start Position, this instances 256 Histogram bins start here in HistogramArrayBuffer
	const uint HistoPos = get_global_id(0) * 256;

	//Step is 1 if horizontal, width if vertical (How far are processed Pixels separated in the array)
	const uint Step = (Horizontal == 1) ? 1 : Width;

	uchar4 Pixel; //__private 
	uint Gray; //__private 

	//Check every Pixel in this Row/Column
	for(uint i = 0; i < (Horizontal == 1 ? Width : Height); i++) //__private uint i
	{
		//Read Pixel
		Pixel = imagein[LineStart + (i * Step)];

		//Grayscale Value (0-255)
		Gray = (uint)Pixel.s0 + (uint)Pixel.s1 + (uint)Pixel.s2;
		Gray /= 3;

		if(Gray >= 256)
		{
			printf("Gray = %u\n", Gray);
		}

		//Increment this bin
		HistogramArrayBuffer[HistoPos + Gray]++;
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void ColorThresholdLines ( __global uint HistogramArrayBuffer[], __global uchar ThresholdLineBuffer[], uint BufferLength)
{
	//Should be run for every Column if Lines are vertical, every Row if Lines are horizontal
	//Length is Width if Lines are vertical, Height if horizontal (how many Pixels are processed in this Instance)
	//HistogramArrayBuffer contains a Histogram for every Row/Column
	//	organized as Length * 256 bins
	//ThresholdLineBuffer contains a Threshold for every Row/Column

	//HistoArray Start Position, this instances 256 Histogram bins start here in HistogramArrayBuffer
	const uint HistoPos = get_global_id(0) * 256;
	//This is also the Position inside ThresholdLineBuffer 
	const uint ThresholdPos = get_global_id(0);

	//Threshold is set in the valley between the two largest peaks
	//A peak is the max Value between a positive and a negative Slope, a Valley the min Value between a negative and a positive Slope

	int Band = 3;
	int SlopeList[256] = { 0 };
	bool SlopeRising = false;
	bool SlopeFalling = false;
	int SlopeRiseCount = 0;
	int SlopeFallCount = 0;
	uint WeightList[256];
	uint Summe = 0;
	bool GotPeak = false;
	int LastPeak = 0;
	int WeightedPeaks = 0;
	bool PeakList[256] = { 0 };
	bool ValleyList[256] = { 0 };
	int NumPeaks = 0;
	int NumValleys = 0;
	uint HalfPeak = 0;


	//First item
	for (int j = 0; j < Band; j++)
	{
		SlopeList[0] += (HistogramArrayBuffer[HistoPos + j + 1] - HistogramArrayBuffer[HistoPos + j]);
	}
	//If first item has a falling slope, it could be a peak
	if(SlopeList[0] < 0) SlopeRising = true;
	//If first item has a rising slope, it could be a valley
	else if (SlopeList[0] > 0) SlopeFalling = true;

	//find all slope, peaks and valleys
	for (int i = 0; i < 256; i++)
	{
		//Slope averaged over +/- Band
		for (int j = max(0, i - Band); j < min(i + Band, 254 - Band); j++)
		{
			SlopeList[i] += (HistogramArrayBuffer[HistoPos + j + 1] - HistogramArrayBuffer[HistoPos + j]);
		}

		//Count rising / falling steps
		if (SlopeList[i] > 0)
		{
			SlopeRiseCount++;
			SlopeFallCount = 0;
		}
		else if (SlopeList[i] < 0)
		{
			SlopeFallCount++;
			SlopeRiseCount = 0;
		}

		//Check for valid rising slope
		if (SlopeRiseCount >= Band)
		{
			SlopeRising = true;
			//Check for change of direction => valley
			if (SlopeFalling)
			{
				SlopeFalling = false;
				ValleyList[max(0, i - Band)] = true;
				NumValleys++;
			}
		}
		//Check for valid falling slope
		else if (SlopeFallCount >= Band)
		{
			SlopeFalling = true;
			//check for change of direction => peak
			if (SlopeRising)
			{
				SlopeRising = false;
				PeakList[max(0, i - Band)] = true;
				NumPeaks++;
			}
		}
	}

	//if last item has a rising slope, it could be a peak
	if ((SlopeList[255] > 0) & SlopeRising)
	{
		PeakList[255] = true;
		NumPeaks++;
	}
	//if last item has a falling slope, it could be a valley
	if ((SlopeList[255] < 0) & SlopeFalling)
	{
		ValleyList[255] = true;
		NumValleys++;
	}

	//Weight of a Peak is the sum of Values between Valleys

	//Find Weight of Peaks
	for (int i = 0; i < 256; i++)
	{
		WeightList[i] = 0;
		Summe += HistogramArrayBuffer[HistoPos + i];

		//A valley is the "End" of a peak
		if (ValleyList[i])
		{
			if (GotPeak)
			{
				WeightList[LastPeak] = Summe;
				WeightedPeaks++;
			}
			Summe = 0;
			GotPeak = false;
		}
		if (PeakList[i])
		{
			GotPeak = true;
			LastPeak = i;
		}
	}
	//Last point could be a Peak
	if (GotPeak)
	{
		WeightList[LastPeak] = Summe;
		WeightedPeaks++;
	}

	//We need at least 2 Peaks
	if (WeightedPeaks == 0)
	{
		//No Peak found, keep old threshold
		return;
	}
	else if (WeightedPeaks < 2)
	{
		//only 1 Peak found, Threshold to middle of rising Slope
		HalfPeak = HistogramArrayBuffer[HistoPos + LastPeak] / 6;
		for (int i = 0; i < 256; i++)
		{
			if (HistogramArrayBuffer[HistoPos + i] >= HalfPeak)
			{
				ThresholdLineBuffer[ThresholdPos] = (uchar)i;
				return;
			}
		}
		//That should not happen, keep old threshold
		return;
	}

	//Find 2 heaviest peaks
	uint Peak_1_Val = 0;
	uint Peak_2_Val = 0;
	uint Peak_1_Pos = 0;
	uint Peak_2_Pos = 0;

	for (int i = 0; i < 256; i++)
	{
		//New biggest peak
		if (PeakList[i] && WeightList[i] > Peak_1_Val)
		{
			if (Peak_1_Val > Peak_2_Val)
			{
				Peak_2_Val = Peak_1_Val;
				Peak_2_Pos = Peak_1_Pos;
			}
			//last biggest becomes second
			Peak_1_Val = WeightList[i];
			Peak_1_Pos = i;
		}
		//New second biggest peak
		else if (PeakList[i] && WeightList[i] > Peak_2_Val)
		{
			Peak_2_Val = WeightList[i];
			Peak_2_Pos = i;
		}
	}

	//If Peak values are zero we have no valid Peaks
	if (Peak_1_Val == 0 || Peak_2_Val == 0)
	{
		//keep old threshold
		return;
	}

	//Threshold is the deepest valley between the two heaviest Peaks
	uint ValleyVal = Peak_1_Val;
	int ValleyFound = -1;

	for (int i = Peak_1_Pos; i != Peak_2_Pos; i += ((Peak_1_Pos < Peak_2_Pos) ? 1 : -1))
	{
		if (ValleyList[i])
		{
			if (HistogramArrayBuffer[HistoPos + i] < ValleyVal) ValleyVal = HistogramArrayBuffer[HistoPos + i];
			ThresholdLineBuffer[ThresholdPos] = (uchar)i;
			ValleyFound = i;
		}
	}

	if (ValleyFound < 0)
	{
		//No Valley between peaks found, use middle bewtween the peaks
		ThresholdLineBuffer[ThresholdPos] = (uchar)((Peak_1_Pos + Peak_2_Pos) / 2);
		return;
	}
	else
	{
		ThresholdLineBuffer[ThresholdPos] = (uchar)ValleyFound;
		return;
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void ColorThresholdAverage ( __global uchar ThresholdLineBuffer[], __global uchar ThresholdArrayBuffer[], 
										uint Height, uint Width, uint Horizontal, int AvgSize)
{
	//Should be run for every Column if Lines are vertical, every Row if Lines are horizontal
	//ThresholdLineBuffer contains a Threshold for every Row/Column
	//ThresholdArrayBuffer contains a Threshold for every Pixel organized as ImageHeight * ImageWidth
	//AvgSize is the withd/height of the averaging
	//Length is Width if Lines are vertical, Height if horizontal

	//The Column/Row this instance is calculating 
	const int Pos = get_global_id(0);

	//ThresholdArray Start Position, this instances Row/Column starts here in ThresholdArrayBuffer
	const uint ArrayPos = get_global_id(0) * ((Horizontal == 1) ? Width : 1);

	//Step is Witdh if Lines are horizontal, 1 if vertical (How far are processed Pixels separated in the array)
	const uint Step = ((Horizontal == 1) ? 1 : Width);

	//how many values are we integrating
	const int x = AvgSize;

	//Average +/- Values
	uint Value = 0; //__private 

	//Values to average not leaving the Buffer
	const uint Istart = (uint)(max((int)0, (Pos - (int)x)));
	const uint Iend = (uint)(min((int)((Horizontal == 1) ? Height : Width), (Pos + (int)x)));

	//Summing up
	for(uint i = Istart; i < Iend; i++) //__private uint i
	{
		Value += ThresholdLineBuffer[i];
	}

	//Numbers of values averaged
	const uint NumVals = Iend - Istart;
	//Average Value
	Value = Value / NumVals;

	//Storing Value for all Rows if Lines are vertical, Columns if Lines are horizontal
	for(int i = 0; i < ((Horizontal == 1) ? Width : Height ); i++)
	{
		ThresholdArrayBuffer[ArrayPos + (i * Step)] = Value;
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void ColorShowHistogram ( __global uchar4 imagein[],
								   __global uchar ThresholdLineBuffer[], __global uchar ThresholdArrayBuffer[],
								   uint Height, uint Width, uint Horizontal)
{
	//Should be run for every Column if Lines are vertical, every Row if Lines are horizontal
	//Overlays Threshold- and averaged Threshold-line (yellow, green)
	//If Lines are vertical overlay is horizontal and vice versa

	//The Row/Column this instance handles
	const uint Pos = get_global_id(0);

	//Threshold of 255 should cover full Window
	const float ThresholdFactor = (Horizontal == 1) ? ((float)Width / (float)256) : ((float)Height / (float)256);

	//Normalized Threshold value at this position
	const uint NormThreshold = (uint)((float)ThresholdLineBuffer[Pos] * ThresholdFactor);
	//Normalized Averaged Threshold value at this position
	const uint NormAvgThreshold = (uint)((float)ThresholdArrayBuffer[Pos * ((Horizontal == 1) ? Width : 1)] * ThresholdFactor);

	if(Horizontal == 0)
	{
		//Overlay Threshold point
		imagein[(NormThreshold * Width) + Pos] = (uchar4)(0, 255, 255, 0);
		if(NormThreshold > 0) imagein[((NormThreshold - 1) * Width) + Pos] = (uchar4)(0, 255, 255, 0);	//yellow
		if(NormThreshold > 1) imagein[((NormThreshold - 2) * Width) + Pos] = (uchar4)(0, 255, 255, 0);

		//Overlay averaged Threshold point
		imagein[(NormAvgThreshold * Width) + Pos] = (uchar4)(0, 255, 255, 0);
		if(NormAvgThreshold > 0) imagein[((NormAvgThreshold - 1) * Width) + Pos] = (uchar4)(255, 255, 0, 0);	//cyan
		if(NormAvgThreshold > 1) imagein[((NormAvgThreshold - 2) * Width) + Pos] = (uchar4)(255, 255, 0, 0);
	}
	else
	{
		//Overlay Threshold point
		imagein[NormThreshold + (Pos * Width)] = (uchar4)(0, 255, 255, 0);

		if(NormThreshold > 0) imagein[NormThreshold - 1 + (Pos * Width)] = (uchar4)(0, 255, 255, 0);	//yellow
		if(NormThreshold > 1) imagein[NormThreshold - 2 + (Pos * Width)] = (uchar4)(0, 255, 255, 0);

		//Overlay averaged Threshold point
		imagein[NormAvgThreshold + (Pos * Width)] = (uchar4)(0, 255, 255, 0);

		if(NormAvgThreshold > 0) imagein[NormAvgThreshold - 1 + (Pos * Width)] = (uchar4)(255, 255, 0, 0);	//cyan
		if(NormAvgThreshold > 1) imagein[NormAvgThreshold - 2 + (Pos * Width)] = (uchar4)(255, 255, 0, 0);
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void ColorBinarize ( __global uchar4 imagein[],  __global uchar4 imageout[], __global uchar ThresholdArrayBuffer[])
{
	//Should be run for every Pixel

	//ThresholdArrayBuffer contains a Threshold for every Pixel
	// organized as ImageHeight * ImageWidth

	//Get Pixel at Position
	const int Pos = get_global_id(0);
	const uchar4 Pixel = imagein[Pos];
	const uchar Threshold = ThresholdArrayBuffer[Pos];

	//Grayscale Value
	const uchar Grey = (uchar)(((uint)Pixel.s0 + (uint)Pixel.s1 + (uint)Pixel.s2) / 3);

	//Binarization
	if(Grey > Threshold)
	{
		imageout[Pos] = 0;
	}
	else
	{
		imageout[Pos] = 255;
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void RGBBlockHistogram ( __global uchar4 imagein[], __global uint HistoRowList[], uint Width)
{
	//Should be run for every 8th Row
	//HistoRowList contains 256 * 3 * Width bins
	//get_global_id(0) => vertical position of this instance

	// getting these 8 values out of a 8x8 matrix
	// - - - - - - - X 
	// - - - - - - X -
	// - - - - - X - -
	// - - - - X - - -
	// - - - X - - - -
	// - - X - - - - -
	// - X - - - - - -
	// X - - - - - - -


	//Line Start Position
	const uint LineStart = get_global_id(0) * Width * 8;		//the row this instance is checking starts here in the image

	//HistoList Start Position
	const uint HistoPos = get_global_id(0) * 256 * 3;			//this instances 3 * 256 Histogram bins start here in HistoRowList

	__private uint4 Pixel;

	//Checking every 8th Pixel in this Row (horizontally)
	for(__private uint i = 0; i < Width; i += 8)
	{
		//Read Pixel
		Pixel.s0 = (uint)imagein[LineStart + i].s0 + 
			(uint)imagein[LineStart + i + 1 + (1 * Width)].s0 + 
			(uint)imagein[LineStart + i + 2 + (2 * Width)].s0 + 
			(uint)imagein[LineStart + i + 3 + (3 * Width)].s0 + 
			(uint)imagein[LineStart + i + 4 + (4 * Width)].s0 + 
			(uint)imagein[LineStart + i + 5 + (5 * Width)].s0 + 
			(uint)imagein[LineStart + i + 6 + (6 * Width)].s0 +
			(uint)imagein[LineStart + i + 7 + (7 * Width)].s0; 
		Pixel.s1 = (uint)imagein[LineStart + i].s1 + 
			(uint)imagein[LineStart + i + 1 + (1 * Width)].s1 + 
			(uint)imagein[LineStart + i + 2 + (2 * Width)].s1 + 
			(uint)imagein[LineStart + i + 3 + (3 * Width)].s1 + 
			(uint)imagein[LineStart + i + 4 + (4 * Width)].s1 + 
			(uint)imagein[LineStart + i + 5 + (5 * Width)].s1 + 
			(uint)imagein[LineStart + i + 6 + (6 * Width)].s1 +
			(uint)imagein[LineStart + i + 7 + (7 * Width)].s1; 
		Pixel.s2 = (uint)imagein[LineStart + i].s2 + 
			(uint)imagein[LineStart + i + 1 + (1 * Width)].s2 + 
			(uint)imagein[LineStart + i + 2 + (2 * Width)].s2 + 
			(uint)imagein[LineStart + i + 3 + (3 * Width)].s2 + 
			(uint)imagein[LineStart + i + 4 + (4 * Width)].s2 + 
			(uint)imagein[LineStart + i + 5 + (5 * Width)].s2 + 
			(uint)imagein[LineStart + i + 6 + (6 * Width)].s2 +
			(uint)imagein[LineStart + i + 7 + (7 * Width)].s2; 

		Pixel.s0 /= 8;
		Pixel.s1 /= 8;
		Pixel.s2 /= 8;

		//Increment this bins
		HistoRowList[HistoPos + (uint)Pixel.s0]++;
		HistoRowList[HistoPos + 256 + (uint)Pixel.s1]++;
		HistoRowList[HistoPos + 512 + (uint)Pixel.s2]++;
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void JoinRGBHistogram ( __global uint HistoRowList[], __global uint HistoList[], int Height)
{
	//Should be run for each of the 256 RGB-bins in HistoRowList
	//HistoRowList contains 256 * 3 * Height bins
	//HistoList contains 256 * 3 bins
	//get_global_id(0) => bin of this instance

	//The bin this instance processes
	const uint BinPos = get_global_id(0);

	__private uint SumB = 0;
	__private uint SumG = 0;
	__private uint SumR = 0;

	//summing bins for every position
	for(__private uint i = 0; i < Height; i++)
	{
		//Read Row Value
		__private uint ValueB = HistoRowList[(i * 256 * 3) + BinPos];
		SumB += ValueB;
		__private uint ValueG = HistoRowList[(i * 256 * 3) + 256 + BinPos];
		SumG += ValueG;
		__private uint ValueR = HistoRowList[(i * 256 * 3) + 512 + BinPos];
		SumR += ValueR;
	}

	HistoList[BinPos] = SumB;
	HistoList[BinPos + 256] = SumG;
	HistoList[BinPos + 512] = SumR;
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void SmoothenRGBHistogram ( __global uint HistoIn[], __global uint HistoOut[], int Width)
{
	//Should be run for each of the 256 RGB-bins of Histogram
	//HistoIn and HistoOut contain 256 * 3 bins

	//bin this instance is processing
	const int Pos = get_global_id(0);
	//how many bins are we integrating
	const int x = Width;

	//Average +/- Values
	__private uint ValueB = 0;
	__private uint ValueG = 0;
	__private uint ValueR = 0;

	//bins to average not leaving the list
	const uint Istart = (uint)(max(0, (Pos - x)));
	const uint Iend = (uint)(min(255, (Pos + x)));

	//Numbers of bins averaged
	const uint NumBins = Iend - Istart;

	for(__private uint i = Istart; i < Iend; i++)
	{
		ValueB += HistoIn[i];
		ValueG += HistoIn[256 + i];
		ValueR += HistoIn[512 + i];
	}

	HistoOut[Pos] = ValueB / NumBins;
	HistoOut[256 + Pos] = ValueG / NumBins;
	HistoOut[512 + Pos] = ValueR / NumBins;
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************


__kernel void ShowRGBHistogram ( __global uchar4 ImageIn[], __global uint HistoList[],
								uint MaxVal,
								__constant uint Peak_1_Val[], __constant uint Peak_1_Pos[],
								__constant uint Peak_2_Val[], __constant uint Peak_2_Pos[],
								__constant uint Threshold[], int Width, int Height, 
								uint LineHeight, uint LineWidth)
{
	//Should be run for each of the 256 bins of Histogram

	//Active Histogram bin
	const int Pos = get_global_id(0);

	//X-Factor for ImageWidth (so many columns display one Histogram Value)
	const float XFactor = Width / 256;
	//X-Start Position in Image to overlay vertical line
	const uint HistoXPos = convert_uint_sat(XFactor * convert_float(Pos));

	//Y-Factor for ImageHeight (maximum Histogram Value => top of Image)
	__private const float MaxFloat = convert_float(MaxVal);
	__private const float HeightFloat = convert_float_rtp(Height);
	const float YFactor = HeightFloat / MaxFloat;

	//Histogram Values RGB of this position
	uint HistoYValueB = convert_uint_sat(YFactor * convert_float_rtp(HistoList[Pos]));
	if(HistoYValueB >= Height) HistoYValueB = Height - 1;
	uint HistoYValueG = convert_uint_sat(YFactor * convert_float_rtp(HistoList[Pos + 256]));
	if(HistoYValueG >= Height) HistoYValueG = Height - 1;
	uint HistoYValueR = convert_uint_sat(YFactor * convert_float_rtp(HistoList[Pos + 512]));
	if(HistoYValueR >= Height) HistoYValueR = Height - 1;

	//Pixels and overlay Values
	__private uchar4 PixelB = (uchar4)(0, 0, 0, 0);
	__private uchar4 NewPixelB = (uchar4)(0, 0, 0, 0);
	__private uchar4 OverlayPixelB = (uchar4)(127, 0, 0, 0);
	__private uchar4 PixelG = (uchar4)(0, 0, 0, 0);
	__private uchar4 NewPixelG = (uchar4)(0, 0, 0, 0);
	__private uchar4 OverlayPixelG = (uchar4)(0, 127, 0, 0);
	__private uchar4 PixelR = (uchar4)(0, 0, 0, 0);
	__private uchar4 NewPixelR = (uchar4)(0, 0, 0, 0);
	__private uchar4 OverlayPixelR = (uchar4)(0, 0, 127, 0);

	//Draw Threshold Lines
	if(Pos == Threshold[0] || Pos == Threshold[1] || Pos == Threshold[2])
	{
		__private int j = 0;
		while(j < Height)
		{
			if(Pos == Threshold[0] && j < Height)
			{
				for(__private uint k = 0; k < LineWidth; k++)
				{
					int PixelPos = HistoXPos + (j * Width) + k;
					PixelB = ImageIn[PixelPos];
					NewPixelB = (PixelB / (uchar4)(2,2,2,1)) + OverlayPixelB;
					ImageIn[PixelPos] = NewPixelB;
				}
				j++;
			}
			if(Pos == Threshold[1] && j < Height)
			{
				for(__private uint k = 0; k < LineWidth; k++)
				{
					int PixelPos = HistoXPos + (j * Width) + k;
					PixelG = ImageIn[PixelPos];
					NewPixelG = (PixelG / (uchar4)(2,2,2,1)) + OverlayPixelG;
					ImageIn[PixelPos] = NewPixelG;
				}
				j++;
			}
			if(Pos == Threshold[2] && j < Height)
			{
				for(__private uint k = 0; k < LineWidth; k++)
				{
					int PixelPos = HistoXPos + (j * Width) + k;
					PixelR = ImageIn[PixelPos];
					NewPixelR = (PixelR / (uchar4)(2,2,2,1)) + OverlayPixelR;
					ImageIn[PixelPos] = NewPixelR;
				}
				j++;
			}
		}
	}

	//Draw Pixels
	for(__private uint i = 0; i < XFactor; i++)
	{
		for(__private uint j = 0; j < LineHeight; j++)
		{
			for(__private uint k = 0; k < LineWidth; k++)
			{
				int PixelPos = HistoXPos + ((HistoYValueB + j) * Width) + k;
				PixelB = ImageIn[PixelPos];
				NewPixelB = (PixelB / (uchar4)(2,2,2,1)) + OverlayPixelB;
				ImageIn[PixelPos] = NewPixelB;

				PixelPos = HistoXPos + ((HistoYValueG + j) * Width) + k;
				PixelG = ImageIn[PixelPos];
				NewPixelG = (PixelG / (uchar4)(2,2,2,1)) + OverlayPixelG;
				ImageIn[PixelPos] = NewPixelG;

				PixelPos = HistoXPos + ((HistoYValueR + j) * Width) + k;
				PixelR = ImageIn[PixelPos];
				NewPixelR = (PixelR / (uchar4)(2,2,2,1)) + OverlayPixelR;
				ImageIn[PixelPos] = NewPixelR;
			}
		}
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

__kernel void ColorizeRGB ( __global uchar4 imagein[],  __global uchar4 imageout[], 
								   __constant uint RGBThreshold[], 
								   const uint InverseImage)
{
	//Should be run for every Pixel

	//Get Pixel at Position
	const int Pos = get_global_id(0);
	const uchar4 Pixel = imagein[Pos];

	if(InverseImage == 0)
	{
		if((uint)Pixel.s0 > RGBThreshold[0]){ imageout[Pos].s0 = 0; }
		else{ imageout[Pos].s0 = 255; }

		if((uint)Pixel.s1 > RGBThreshold[1]){ imageout[Pos].s1 = 0; }
		else{ imageout[Pos].s1 = 255; }

		if((uint)Pixel.s2 > RGBThreshold[2]){ imageout[Pos].s2 = 0; }
		else{ imageout[Pos].s2 = 255; }
	}
	else
	{
		if((uint)Pixel.s0 > RGBThreshold[0]){ imageout[Pos].s0 = 255; }
		else{ imageout[Pos].s0 = 0; }

		if((uint)Pixel.s1 > RGBThreshold[1]){ imageout[Pos].s1 = 255; }
		else{ imageout[Pos].s1 = 0; }

		if((uint)Pixel.s2 > RGBThreshold[2]){ imageout[Pos].s2 = 255; }
		else{ imageout[Pos].s2 = 0; }
	}
}

//**************************************************************************************************************************************************************************************
//**************************************************************************************************************************************************************************************

