#pragma once

#include "DagNodeWriter.h"

class CamaraWriter : public DagNodeWriter
{
public:
	CamaraWriter(MDagPath dagPath, MStatus status);
	virtual ~CamaraWriter();
	virtual MStatus			ExtractGeometry();
	virtual MStatus			WriteToFile(ostream& os);
};