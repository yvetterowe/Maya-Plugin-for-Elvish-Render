#include "CamaraWriter.h"

CamaraWriter::CamaraWriter(MDagPath dagPath, MStatus status) : DagNodeWriter(dagPath,status)
{

}

CamaraWriter::~CamaraWriter()
{

}

MStatus CamaraWriter::ExtractInfo()
{
	MGlobal::displayInfo("begin to export camara info!\n");

	return MStatus::kSuccess;
}

MStatus CamaraWriter::WriteToFile( ostream& os )
{
	return MStatus::kSuccess;
}
