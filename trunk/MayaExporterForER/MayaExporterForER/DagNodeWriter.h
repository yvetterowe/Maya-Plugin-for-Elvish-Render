#pragma once

#include <iostream>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MItDependencyGraph.h>


class DagNodeWriter
{
public:
	DagNodeWriter(MDagPath dagPath, MStatus status);
	virtual ~DagNodeWriter();
	virtual MStatus			ExtractInfo() = 0;
	virtual MStatus			WriteToFile(ostream& os) = 0;

protected:
	//helpers
	static  void			outputTabs (ostream & os, int tabCount);
	
	MDagPath*				fpath;
	MString					fname;
};