//
// Copyright (C) 
// 
// File: MayaExporterForERCmd.cpp
//
// MEL Command: MayaExporterForER
//
// Author: Maya Plug-in Wizard 2.0
//

// Includes everything needed to register a simple MEL command with Maya.
// 

#include <maya/MSimple.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MIOStream.h>
#include <maya/MFileObject.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MGlobal.h>
#include <maya/MRenderView.h>
#include <maya/M3dView.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

#include <eiAPI\ei.h>

#include <stdio.h>
#include <math.h>
#include <fstream>
#include <Windows.h>

#include "MayaExporterForER.h"
#include "Render.h"

//#define SAMPLE

#ifdef SAMPLE
char* PicPathName = "er.frame.0001.bmp";
#else
char* PicPathName = "test.bmp";
#endif

static const char * kDoNotClearBackground		= "-b";
static const char * kDoNotClearBackgroundLong	= "-background";
const unsigned int num_side_tiles = 8;

// Use helper macro to register a command with Maya.  It creates and
// registers a command that does not support undo or redo.  The 
// created class derives off of MPxCommand.
//
//DeclareSimpleCommand( MayaExporterForER, "", "2012");

class ExportMayaScene : public MPxCommand
{
public:
	MStatus					doIt( const MArgList& args );
	static void*			creator();

private:
	void					parseArglist(const MArgList& args);

	//Functions for Render View of Step 3
	virtual MStatus			Exec(char*  PathName);
	static MSyntax			newSyntax();
	MStatus					parseSyntax (MArgDatabase &argData);
    bool					readBmp();
	RV_PIXEL				evaluate(int x, int y);

private:
	//Members for Render View of Step 3
	char*					BmpName;
	unsigned char*			pBmpBuf;
	int						width;
	int						height;
	RGBQUAD*				pColorTable;
	int						biBitCount;
	int						lineByte;
	bool					doNotClearBackground;
};

///////////////////////////////ER_Render_View_Fuc////////////////////////////////////////////////
bool ExportMayaScene::readBmp( )
{
	FILE* fp = fopen( BmpName, "rb" );
	if( fp == 0 ) return 0;
	fseek( fp, sizeof(BITMAPFILEHEADER), 0 );
	BITMAPINFOHEADER infoHead;
	fread( &infoHead, sizeof( BITMAPINFOHEADER), 1, fp );
	width = infoHead.biWidth;
	height = infoHead.biHeight;
	biBitCount = infoHead.biBitCount;
	lineByte = (width*biBitCount/8 + 3)/4*4;
	pBmpBuf = new unsigned char[lineByte*height];
	fread( pBmpBuf, 1, lineByte*height, fp );
	fclose(fp);
	return 1;
}

RV_PIXEL ExportMayaScene::evaluate(int x, int y)
{
	RV_PIXEL pixel;
	pixel.r = float(*(pBmpBuf+y*lineByte+x*3+2));
	pixel.g = float(*(pBmpBuf+y*lineByte+x*3+1));
	pixel.b = float(*(pBmpBuf+y*lineByte+x*3));
	pixel.a = 255.0f;
	return pixel;
}

MSyntax ExportMayaScene::newSyntax()
{
	MStatus status;
	MSyntax syntax;
	syntax.addFlag( kDoNotClearBackground, kDoNotClearBackgroundLong );
	CHECK_MSTATUS_AND_RETURN(status, syntax);
	return syntax;
}

MStatus ExportMayaScene::parseSyntax (MArgDatabase &argData)
{
	// Get the flag values, otherwise the default values are used.
	doNotClearBackground = argData.isFlagSet( kDoNotClearBackground );
	
	return MS::kSuccess;
}

MStatus ExportMayaScene::Exec(char* PathName)
{
	MGlobal::displayInfo("Exec() begin!\n");
	MStatus stat = MS::kSuccess;
	BmpName = PathName;
	readBmp();
	// Check if the render view exists. It should always exist, unless
	// Maya is running in batch mode.
	//
	if (!MRenderView::doesRenderEditorExist())
	{
		cout<< "Cannot ER_RenderView in batch render mode. "
				   "Please run in interactive mode, "
				   "so that the render editor exists." << endl;
		return MS::kFailure;
	}
	else
	{
		cout<<"Past doesRenderEditorExist()"<<endl;
	}

	if (MRenderView::startRender( width, height, doNotClearBackground) != MS::kSuccess)
	{
		cout<<"ER_RenderView: error occured in startRender." << endl;
		return MS::kFailure;
	}
	
	int lineByte = (width*biBitCount/8 + 3 )/4*4;
	unsigned int average_tiles_width = width / num_side_tiles;
	unsigned int average_tiles_height = height / num_side_tiles;

	// Draw each tile
	for (unsigned int tile_y = 0; tile_y < num_side_tiles; tile_y++)
	{
		for (unsigned int tile_x = 0; tile_x < num_side_tiles; tile_x++)
		{
			int min_x = tile_x * average_tiles_width;
			int max_x;
			if ((tile_x+1) == num_side_tiles)
				max_x = width-1;
			else
				max_x = (tile_x + 1) * average_tiles_width - 1;				
			int min_y = tile_y * average_tiles_height;
			int max_y;
			if ((tile_y+1) == num_side_tiles)
				max_y = height-1;
			else
				max_y = (tile_y + 1) * average_tiles_height - 1;
			
			unsigned int tile_width = max_x - min_x + 1; 
			unsigned int tile_height = max_y - min_y + 1;

			RV_PIXEL* pixels = new RV_PIXEL[tile_width * tile_height];
			unsigned int index = 0;
			for (unsigned int j = min_y; j < max_y+1; j++ )
			{
				for (unsigned int i = min_x; i < max_x+1; i++)
				{
					pixels[index] = evaluate(i , j);
					index++;
				}
			}
			if (MRenderView::updatePixels(min_x, max_x, min_y, max_y, pixels) != MS::kSuccess)
			{
				cout<< "ER_RenderView: error occured in updatePixels." << endl;
				delete [] pixels;
				return MS::kFailure;
			}

			delete [] pixels;

			if (MRenderView::refresh(min_x, max_x, min_y, max_y) != MS::kSuccess)
			{
				cout<<"ER_RenderView: error occured in refresh." << endl;
				return MS::kFailure;
			}
		}
	}

	if (MRenderView::endRender() != MS::kSuccess)
	{
		cout <<"ER_RenderView: error occured in endRender." << endl;
		return MS::kFailure;
	}

	cout<<"ER_RenderView completed." << endl;
	return stat;
}
//////END///////////////////////////ER_Render_View//////////////////////////////////////////////


MStatus ExportMayaScene::doIt( const MArgList& args )
//
//	Description:
//		implements the MEL MayaExporterForER command.
//       Includes exporting maya file for ER and filp the output rendering
//                 picture on the Maya Render View Window.
//
//	Arguments:
//		args - the argument list that was passes to the command from MEL
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - command failed (returning this value will cause the 
//                     MEL script that is being run to terminate unless the
//                     error is caught using a "catch" statement.
//
{
	MStatus stat = MS::kSuccess;

	// Since this class is derived off of MPxCommand, you can use the 
	// inherited methods to return values and set error messages
	//
	
	MayaExporterForER* exporter = new MayaExporterForER;
	MString filename("mytest.ess");
	MFileObject file;
	file.setRawName(filename);

	exporter->parseArglist(args);
	exporter->writer(file,"none",MPxFileTranslator::kExportAccessMode);

	setResult( "MayaExporterForER command executed!\n" );

	//render it into image
	Render render;

#ifdef SAMPLE
	render.parse("sample.ess");
	render.overrideOptions(exporter);
	ei_render("world","caminst1","opt");
#else
	render.parse("mytest.ess");
	//render.overrideOptions(exporter);
	ei_render("world","instperspShape","opt");
#endif

	delete exporter;

	//flip the rendering output onto the Maya RenderViewWindow;
	Exec(PicPathName);

	return stat;
}

void* ExportMayaScene::creator()
{
	return new ExportMayaScene;
}

MStatus initializePlugin( MObject obj ) {
	MFnPlugin plugin( obj, "Autodesk", "1.0", "Any" );
	plugin.registerCommand( "ExportMayaScene", ExportMayaScene::creator );
	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj ) {
	MFnPlugin plugin( obj );
	plugin.deregisterCommand( "ExportMayaScene" );
	return MS::kSuccess;
}