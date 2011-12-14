#pragma once

/*enum OptionType{OP_CONTRAST,     //0
				OP_SAMPLE,       //1
				OP_FILTER,       //2
				OP_TRACEDEPTH,   //3
				OP_ILLUMINATION, //4
				OP_FINALGATHER,  //5
				OP_CNT };*/

#include <maya/MString.h>

struct OpContrast
{
	double r,g,b,a;
};

struct OpSample
{
	int sMin,sMax;
};

struct OpFilter
{
	int fTypeId,fSize;
	MString fName;
};

struct OpTraceDepth 
{
	int reflect,refrect,sum;
};

struct OpResolution 
{
	int width,height;
};