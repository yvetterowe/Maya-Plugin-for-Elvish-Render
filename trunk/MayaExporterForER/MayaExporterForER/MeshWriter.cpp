#include "MeshWriter.h"

#include <maya/MItMeshPolygon.h>

MeshWriter::MeshWriter(MDagPath dagPath, MStatus status): DagNodeWriter(dagPath,status)
{
	fMesh = new MFnMesh(dagPath,&status);
	fname = fMesh->name();
	fInstName = MString("inst"+fname);
}

MeshWriter::~MeshWriter()
{
	if(fMesh!=NULL) delete fMesh;
}

MStatus MeshWriter::ExtractInfo()
{
	MGlobal::displayInfo("begin to extract info of mesh!\n");
	
	if (MStatus::kFailure == fMesh->getPoints(fVertexArray, MSpace::kWorld)) {
		MGlobal::displayError("MFnMesh::getPoints"); 
		return MStatus::kFailure;
	}

	if(MStatus::kFailure == fMesh->/*getNormals(fNormalArray,MSpace::kWorld)*/getVertexNormals(false,fNormalArray,MSpace::kWorld)){
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
	MGlobal::displayInfo("begin to write mesh info to file!\n");
	
	os<<"object "<<"\""<<fname.asChar()<<"\" "<<"\"poly\""<<"\n";

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

	outputInstance(os,fInstName);
	return MStatus::kSuccess;
}

MStatus MeshWriter::outputVertex( ostream& os )
{
	MGlobal::displayInfo("begin to output vertex!\n");
	int vertexCnt = fVertexArray.length();
	if(vertexCnt == 0) {
		return MStatus::kFailure;
	}

	outputTabs(os,1);
	os<<"pos_list "<<vertexCnt<<"\n";

	for(int i = 0;i<vertexCnt;++i)
	{
		outputTabs(os,1);
		os<<fVertexArray[i].x<<" "
		  <<fVertexArray[i].y<<" "
		  <<fVertexArray[i].z<<"\n";
	}

	return MStatus::kSuccess;
}

MStatus MeshWriter::outputNormal( ostream& os )
{
	MGlobal::displayInfo("begin to output normal!\n");
	
	int normalCnt = fNormalArray.length();
	if(normalCnt == 0) {
		return MStatus::kFailure;
	}

	outputTabs(os,1);
	os<<"nrm_list "<<normalCnt<<"\n";

	for(int i = 0;i<normalCnt;++i)
	{
		outputTabs(os,1);
		os<<fNormalArray[i].x<<" "
		  <<fNormalArray[i].y<<" "
		  <<fNormalArray[i].z<<"\n";
	}

	return MStatus::kSuccess;
}

MStatus MeshWriter::outputTriangleVertexIndex( ostream& os )
{
	MGlobal::displayInfo("begin to output triangleindex!\n");
	
	int indexCnt = fFaceTriangleVertexArray.length();
	if(indexCnt == 0) {
		return MStatus::kFailure;
	}

	outputTabs(os,1);
	os<<"triangle_list "<<indexCnt<<"\n";

	for(int i = 0;i<=indexCnt-3;i+=3)
	{
		outputTabs(os,1);
		os<<fFaceTriangleVertexArray[i]<<" "
		  <<fFaceTriangleVertexArray[i+1]<<" "
		  <<fFaceTriangleVertexArray[i+2]<<"\n";
	}

	return MStatus::kSuccess;
}
