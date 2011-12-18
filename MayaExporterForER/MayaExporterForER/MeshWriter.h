#pragma once

#include "DagNodeWriter.h"

#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>

#include <eiAPI\ei.h>

class MeshWriter : public DagNodeWriter
{
public:
	MeshWriter(MDagPath dagPath, MStatus status);
	virtual ~MeshWriter();

	virtual MStatus			ExtractInfo();
	virtual MStatus			WriteToFile(ostream& os);

	virtual void			outputInstance(ostream&os,MString instName);
	virtual MStatus         render();
	virtual void            render_instance(MString instName);

private:
	
	//scene info
	MFnMesh*				fMesh;
	MDagPath                fPath;
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

	//render
	MStatus                 render_shader();
	MStatus                 render_vertex();
	MStatus                 render_normal();
	MStatus                 render_triangleVertexIndex();
	MStatus					render_photon();
};