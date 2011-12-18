#pragma once

#include "LightWriter.h"

#include <maya/MFnPointLight.h>
class PointLightWriter : public LightWriter
{
public:
	PointLightWriter(MDagPath dagPath, MStatus status);
	virtual ~PointLightWriter();
	virtual MStatus				ExtractInfo();
	//virtual MStatus				WriteToFile(ostream& os);
	virtual MStatus             render();                                           

private:
	MFnPointLight*				fPointLight;

};