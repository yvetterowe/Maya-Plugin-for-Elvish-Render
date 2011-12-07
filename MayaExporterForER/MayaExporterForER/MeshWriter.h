#pragma once

#include "DagNodeWriter.h"

#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>

class MeshWriter : public DagNodeWriter
{
public:
	MeshWriter(MDagPath dagPath, MStatus status);
	virtual ~MeshWriter();

	virtual MStatus			ExtractInfo();
	virtual MStatus			WriteToFile(ostream& os);
	virtual void			outputInstance(ostream&os,MString instName);

private:
	MFnMesh*				fMesh;
	MPointArray				fVertexArray;
	MFloatVectorArray		fNormalArray;
	MIntArray				fFaceTriangleCntArray;
	MIntArray				fFaceTriangleVertexArray;
	MObjectArray            fShaderArray;
	MIntArray				fShaderFaceArray;
	MString					fMaterialName;

	//helper methods
	MStatus					outputVertex(ostream& os);
	MStatus					outputNormal(ostream& os);
	MStatus					outputTriangleVertexIndex(ostream& os);
	MStatus					outputShader(ostream& os);
};