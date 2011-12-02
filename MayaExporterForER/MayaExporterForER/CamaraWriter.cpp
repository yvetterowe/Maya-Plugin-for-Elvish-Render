#include "CamaraWriter.h"

CamaraWriter::CamaraWriter(MDagPath dagPath, MStatus status) : DagNodeWriter(dagPath,status)
{

}

CamaraWriter::~CamaraWriter()
{

}

MStatus CamaraWriter::ExtractGeometry()
{
	return MStatus::kSuccess;
}

MStatus CamaraWriter::WriteToFile( ostream& os )
{
	return MStatus::kSuccess;
}
