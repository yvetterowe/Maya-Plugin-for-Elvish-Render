#include "DagNodeWriter.h"

DagNodeWriter::DagNodeWriter(MDagPath dagPath, MStatus status)
{
	fpath = new MDagPath(dagPath);
}

DagNodeWriter::~DagNodeWriter()
{
	if(fpath!=NULL) delete fpath;
}

void DagNodeWriter::outputTabs( ostream& os, int tabCount )
{
	for (int i = 0; i < tabCount; i++) {
		os << "\t";
	}
}

void DagNodeWriter::outputInstance( ostream& os, MString instName )
{
	os<<"instance "<<"\""<<instName.asChar()<<"\"\n";
	outputTabs(os,1);
	os<<"element "<<"\""<<fname.asChar()<<"\"\n";
	os<<"end instance"<<"\n";
	os<<"\n";
}

void DagNodeWriter::outputOptions( ostream&os )
{
	os<<"options \"opt\""<<"\n";
	outputTabs(os,1); os<<"samples 0 2"<<"\n";
	outputTabs(os,1); os<<"contrast 0.05 0.05 0.05 0.05"<<"\n";
	outputTabs(os,1); os<<"filter \"gaussian\" 3.0"<<"\n";
	os<<"end options"<<"\n";
}

MString DagNodeWriter::GetInstName()
{
	return fInstName;
}
