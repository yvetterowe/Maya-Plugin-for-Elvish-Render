#pragma once

#include "DagNodeWriter.h"

#include <maya/MFnLight.h>
#include <maya/MColor.h>

class LightWriter : public DagNodeWriter
{
public:
	LightWriter(MDagPath dagPath, MStatus status);
	virtual ~LightWriter();
	virtual MStatus				ExtractInfo();
	virtual MStatus				WriteToFile(ostream& os);

private:
	enum LightType {POINT_LIGHT,DIRECTIONAL_LIGHT,AREA_LIGHT,SPOT_LIGHT,TYPE_CNT};
	MFnLight*					fLight;
	LightType					fType;
	float                       fIntensity;
	MColor						fColor;
};