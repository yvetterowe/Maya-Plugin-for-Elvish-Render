#pragma once

#include "DagNodeWriter.h"

class MeshWriter : public DagNodeWriter
{
public:
	MeshWriter(MDagPath dagPath, MStatus status);
	virtual ~MeshWriter();
	virtual MStatus			ExtractGeometry();
	virtual MStatus			WriteToFile(ostream& os);
};