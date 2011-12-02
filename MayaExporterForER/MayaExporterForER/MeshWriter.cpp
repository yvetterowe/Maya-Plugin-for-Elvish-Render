#include "MeshWriter.h"

MeshWriter::MeshWriter(MDagPath dagPath, MStatus status): DagNodeWriter(dagPath,status)
{

}

MeshWriter::~MeshWriter()
{
	
}

MStatus MeshWriter::ExtractGeometry()
{
	return MStatus::kSuccess;
}

MStatus MeshWriter::WriteToFile( ostream& os )
{
	return MStatus::kSuccess;
}
