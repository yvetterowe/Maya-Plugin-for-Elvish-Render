#pragma once

#include "DagNodeWriter.h"

#include <maya/MFnCamera.h>
#include <maya/MStringArray.h>

class CamaraWriter : public DagNodeWriter
{
public:
	CamaraWriter(MDagPath dagPath, MStatus status);
	virtual ~CamaraWriter();
	virtual MStatus			ExtractInfo();
	virtual MStatus			WriteToFile(ostream& os);
	virtual MStatus         render();

	//resolution config
	void					setResolution(int w,int h);

private:

	//helper methods
	void outputOutPutConfig(ostream& os);

	//render
	void render_configure();

	//camera info
	MFnCamera*				fCamara;
	double					fFocal;
	double					fAperture;
	double					fAspect;
	MStringArray            fImagerShaders;

	//resolution info
	int						fWidth,fHeight;
};