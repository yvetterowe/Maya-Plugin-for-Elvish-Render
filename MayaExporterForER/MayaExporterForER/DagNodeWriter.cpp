#include "DagNodeWriter.h"

DagNodeWriter::DagNodeWriter(MDagPath dagPath, MStatus status)
{
	fpath = new MDagPath(dagPath);
}

DagNodeWriter::~DagNodeWriter()
{
	if(fpath!=NULL) delete fpath;
}

void DagNodeWriter::outputTabs( ostream & os, int tabCount )
{
	for (int i = 0; i < tabCount; i++) {
		os << "\t";
	}
}
