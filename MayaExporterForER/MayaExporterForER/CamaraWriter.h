#pragma once

#include "DagNodeWriter.h"

#include <maya/MFnCamera.h>

class CamaraWriter : public DagNodeWriter
{
public:
	CamaraWriter(MDagPath dagPath, MStatus status);
	virtual ~CamaraWriter();
	virtual MStatus			ExtractInfo();
	virtual MStatus			WriteToFile(ostream& os);

private:

	//helper methods
	void outputOutPutConfig(ostream& os);
	MFnCamera*				fCamara;
	double					fFocal;
	double					fAperture;
	double					fAspect;
};