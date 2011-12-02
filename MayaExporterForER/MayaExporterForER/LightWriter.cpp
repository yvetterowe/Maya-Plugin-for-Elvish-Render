#include "LightWriter.h"

LightWriter::LightWriter(MDagPath dagPath, MStatus status) : DagNodeWriter(dagPath,status)
{

}

LightWriter::~LightWriter()
{

}

MStatus LightWriter::ExtractGeometry()
{
	return MStatus::kSuccess;
}

MStatus LightWriter::WriteToFile( ostream& os )
{
	return MStatus::kSuccess;
}
