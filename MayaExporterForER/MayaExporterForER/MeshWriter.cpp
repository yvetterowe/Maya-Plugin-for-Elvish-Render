#include "MeshWriter.h"

#include <maya/MItMeshPolygon.h>

MeshWriter::MeshWriter(MDagPath dagPath, MStatus status): DagNodeWriter(dagPath,status)
{
	fMesh = new MFnMesh(dagPath,&status);
}

MeshWriter::~MeshWriter()
{
	if(fMesh!=NULL) delete fMesh;
}

MStatus MeshWriter::ExtractInfo()
{
	if (MStatus::kFailure == fMesh->getPoints(fVertexArray, MSpace::kWorld)) {
		MGlobal::displayError("MFnMesh::getPoints"); 
		return MStatus::kFailure;
	}

	if(MStatus::kFailure == fMesh->getNormals(fNormalArray,MSpace::kWorld)){
		MGlobal::displayError("MFnMesh::getNormals");
		return MStatus::kFailure;
	}

	if(MStatus::kFailure == fMesh->getTriangles(fFaceTriangleCntArray,fFaceTriangleVertexArray)){
		MGlobal::displayInfo("MFnmesh::getTriangles");
		return MStatus::kFailure;
	}
	/*MStatus status;
	MObject mObj(*fMesh);
	MItMeshPolygon itMeshPolygon(mObj,&status);

	if(MStatus::kFailure == status) {
		return status;
	}

	for(;!itMeshPolygon.isDone();itMeshPolygon.next())
	{
		int triangleCnt;
		if(MStatus::kFailure == itMeshPolygon.numTriangles(triangleCnt)){
			return MStatus::kFailure;
		}
		if(MStatus::kFailure == itMeshPolygon.getTriangles(fVertexArray,fFaceVertexIndexArray,MSpace::kWorld)){
			return MStatus::kFailure;
		}
	}*/
	
	return MStatus::kSuccess;
}

MStatus MeshWriter::WriteToFile( ostream& os )
{
	os<<"object "<<"\""<<fname<<"\" "<<"\"poly\""<<"\n";

	if(MStatus::kFailure == outputVertex(os)) {
		MGlobal::displayError("outputVertex");
		return MStatus::kFailure;
	}

	if(MStatus::kFailure == outputNormal(os)) {
		MGlobal::displayError("outputNormal");
		return MStatus::kFailure;
	}

	if(MStatus::kFailure == outputTriangleVertexIndex(os)) {
		MGlobal::displayError("outputFaceVertexIndex");
		return MStatus::kFailure;
	}
	os<<"end object"<<"\n";
	os<<"\n";
	return MStatus::kSuccess;
}

MStatus MeshWriter::outputVertex( ostream& os )
{
	int vertexCnt = fVertexArray.length();
	if(vertexCnt == 0) {
		return MStatus::kFailure;
	}

	os<<"pos_list "<<vertexCnt<<"\n";

	for(int i = 0;i<vertexCnt;++i)
	{
		os<<fVertexArray[i].x<<" "
		  <<fVertexArray[i].y<<" "
		  <<fVertexArray[i].z<<"\n";
	}

	return MStatus::kSuccess;
}

MStatus MeshWriter::outputNormal( ostream& os )
{
	int normalCnt = fNormalArray.length();
	if(normalCnt == 0) {
		return MStatus::kFailure;
	}

	os<<"nrm_list "<<normalCnt<<"\n";

	for(int i = 0;i<normalCnt;++i)
	{
		os<<fNormalArray[i].x<<" "
		  <<fNormalArray[i].y<<" "
		  <<fNormalArray[i].z<<"\n";
	}

	return MStatus::kSuccess;
}

MStatus MeshWriter::outputTriangleVertexIndex( ostream& os )
{
	int indexCnt = fFaceTriangleVertexArray.length();
	if(indexCnt == 0) {
		return MStatus::kFailure;
	}

	os<<"triangle_list "<<indexCnt<<"\n";

	for(int i = 0;i<=indexCnt-3;i+=3)
	{
		os<<fFaceTriangleVertexArray[i]<<" "
		  <<fFaceTriangleVertexArray[i+1]<<" "
		  <<fFaceTriangleVertexArray[i+2]<<"\n";
	}

	return MStatus::kSuccess;
}
