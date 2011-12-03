#pragma once

#include "DagNodeWriter.h"

class LightWriter : public DagNodeWriter
{
public:
	LightWriter(MDagPath dagPath, MStatus status);
	virtual ~LightWriter();
	virtual MStatus ExtractInfo();
	virtual MStatus			WriteToFile(ostream& os);
};