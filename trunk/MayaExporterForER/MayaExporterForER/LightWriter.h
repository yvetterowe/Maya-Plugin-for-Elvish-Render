#pragma once

#include "DagNodeWriter.h"

#include <maya/MFnLight.h>
#include <maya/MColor.h>
#include <maya/MFloatVector.h>

class LightWriter : public DagNodeWriter
{
public:
	LightWriter(MDagPath dagPath, MStatus status);
	virtual ~LightWriter();
	virtual MStatus				ExtractInfo();
	virtual MStatus				WriteToFile(ostream& os);

protected:
	float                       fIntensity;
	MColor						fColor;
	MString						fShaderName;
};