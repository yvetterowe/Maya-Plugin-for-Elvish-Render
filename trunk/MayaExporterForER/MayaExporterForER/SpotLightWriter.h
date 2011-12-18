#pragma once

#include "LightWriter.h"

#include <maya/MFnSpotLight.h>
class SpotLightWriter : public LightWriter
{
public:
	SpotLightWriter(MDagPath dagPath, MStatus status);
	virtual ~SpotLightWriter();
	virtual MStatus				ExtractInfo();
	//virtual MStatus				WriteToFile(ostream& os);
	virtual MStatus             render();

private:
	MFnSpotLight*				fSpotLight;
	MFloatVector				fDirection;
	float						fSpread;
	float						fDeltaAngle;
};