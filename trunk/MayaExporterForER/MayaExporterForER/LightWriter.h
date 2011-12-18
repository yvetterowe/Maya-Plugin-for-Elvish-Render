#pragma once

#include "DagNodeWriter.h"

#include <maya/MFnLight.h>
#include <maya/MColor.h>
#include <maya/MFloatVector.h>

#include<eiAPI\ei.h>

class LightWriter : public DagNodeWriter
{
public:
	LightWriter(MDagPath dagPath, MStatus status);
	virtual ~LightWriter();
	virtual MStatus				ExtractInfo();
	//virtual MStatus				WriteToFile(ostream& os);

	virtual MStatus             render();

protected:
	//emitter config
	void						render_emitter();
	
	//light common attributes
	float                       fIntensity;
	MColor						fColor;
	MString						fShaderName;
};