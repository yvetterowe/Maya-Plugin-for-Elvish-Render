#include "LightWriter.h"
#include "stringprintf.h"

LightWriter::LightWriter(MDagPath dagPath,MStatus status) : DagNodeWriter(dagPath,status)
{
}

LightWriter::~LightWriter()
{
}

MStatus LightWriter::ExtractInfo()
{
	return MStatus::kSuccess;
}

MStatus LightWriter::WriteToFile( ostream& os )
{
	return MStatus::kSuccess;
}
