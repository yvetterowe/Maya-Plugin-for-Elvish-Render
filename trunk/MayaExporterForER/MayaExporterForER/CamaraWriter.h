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

private:

	//helper methods
	void outputOutPutConfig(ostream& os);

	//render
	void render_configure();

	MFnCamera*				fCamara;
	double					fFocal;
	double					fAperture;
	double					fAspect;
	MStringArray            fImagerShaders;

};