#include "DagNodeWriter.h"

DagNodeWriter::DagNodeWriter(MDagPath dagPath, MStatus status)
{
	fpath = new MDagPath(dagPath);
}

DagNodeWriter::~DagNodeWriter()
{
	if(fpath!=NULL) delete fpath;
}
