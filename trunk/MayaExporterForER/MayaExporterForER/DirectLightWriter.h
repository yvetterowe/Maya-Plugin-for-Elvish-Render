#pragma once

#include "LightWriter.h"

#include <maya/MFnDirectionalLight.h>

class DirectLightWriter : public LightWriter
{
public:
	DirectLightWriter(MDagPath dagPath, MStatus status);
	virtual ~DirectLightWriter();
	virtual MStatus				ExtractInfo();
	virtual MStatus				WriteToFile(ostream& os);

private:
	MFnDirectionalLight*		fDirectLight;
	MFloatVector				fDirection;
	float						fSpread;
};