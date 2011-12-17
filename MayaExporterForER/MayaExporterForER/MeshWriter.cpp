#include "MeshWriter.h"
#include "stringprintf.h"

#include <maya/MItMeshPolygon.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnPhongEShader.h>

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
	
	if (MStatus::kFailure == fMesh->getPoints(fVertexArray, MSpace::kObject)) {
		MGlobal::displayError("MFnMesh::getPoints"); 
		return MStatus::kFailure;
	}

	if(MStatus::kFailure == fMesh->getNormals(fNormalArray,MSpace::kWorld)/*getVertexNormals(false,fNormalArray,MSpace::kObject)*/){
		MGlobal::displayError("MFnMesh::getNormals");
		return MStatus::kFailure;
	}

	if(MStatus::kFailure == fMesh->getTriangles(fFaceTriangleCntArray,fFaceTriangleVertexArray)){
		MGlobal::displayError("MFnmesh::getTriangles");
		return MStatus::kFailure;
	}

	if(MStatus::kFailure == fMesh->getConnectedShaders(0,fShaderArray,fShaderFaceArray)){
		MGlobal::displayError("MFnmesh::getConnectedShaders");
	}

	return MStatus::kSuccess;
}

MStatus MeshWriter::WriteToFile( ostream& os )
{
	MGlobal::displayInfo("begin to write mesh info to file!\n");
	
	outputShader(os);

	os<<"object "<<"\""<<fname.asChar()<<"\" "<<"\"poly\""<<"\n";

	if(MStatus::kFailure == outputVertex(os)) {
		MGlobal::displayError("outputVertex");
		return MStatus::kFailure;
	}

	/*if(MStatus::kFailure == outputNormal(os)) {
		MGlobal::displayError("outputNormal");
		return MStatus::kFailure;
	}*/

	if(MStatus::kFailure == outputTriangleVertexIndex(os)) {
		MGlobal::displayError("outputFaceVertexIndex");
		return MStatus::kFailure;
	}
	os<<"end object"<<"\n";
	os<<"\n";

	outputInstance(os,fInstName);
	return MStatus::kSuccess;
}

MStatus MeshWriter::render()
{
	MGlobal::displayInfo("render mesh!\n");
	
	render_shader();

	ei_object(fname.asChar(),"poly");
	MGlobal::displayInfo("set poly name succeed!\n");

	if(MStatus::kFailure == render_vertex()) {
		MGlobal::displayError("renderVertex");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("set poly vertex succeed!\n");

	//normal???

	if(MStatus::kFailure == render_triangleVertexIndex()) {
		MGlobal::displayError("renderFaceVertexIndex");
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("set poly triverindex succeed!\n");
	ei_end_object();

	render_instance(fInstName);
	MGlobal::displayInfo("set poly iinstance succeed!\n");
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
		os<<StringPrintf("%.6lf %.6lf %.6lf\n",fVertexArray[i].x,fVertexArray[i].y,fVertexArray[i].z);	
	}

	return MStatus::kSuccess;
}

MStatus MeshWriter::render_vertex()
{
	MGlobal::displayInfo("begin to render vertex\n");
	
	int vertexCnt = fVertexArray.length();
	if(vertexCnt == 0) {
		return MStatus::kFailure;
	}
	
	//ei_pos_list(1024);
	ei_pos_list(ei_tab(EI_DATA_TYPE_VECTOR, 1024));

	for(int i = 0;i<vertexCnt;++i)
	{
		ei_tab_add_vector(fVertexArray[i].x,fVertexArray[i].y,fVertexArray[i].z);
	}
	ei_end_tab();

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
		os<<StringPrintf("%.6lf %.6lf %.6lf\n",fNormalArray[i].x,fNormalArray[i].y,fNormalArray[i].z);	
	}

	return MStatus::kSuccess;
}

MStatus MeshWriter::render_normal()
{
	MGlobal::displayInfo("begin to render normal!\n");
	
	int normalCnt = fNormalArray.length();
	if(normalCnt == 0) {
		return MStatus::kFailure;
	}

	//os<<"nrm_list "<<normalCnt<<"\n";
	
	for(int i = 0;i<normalCnt;++i)
	{
		//outputTabs(os,1);
		//os<<StringPrintf("%.6lf %.6lf %.6lf\n",fNormalArray[i].x,fNormalArray[i].y,fNormalArray[i].z);	
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

MStatus MeshWriter::render_triangleVertexIndex()
{
	MGlobal::displayInfo("begin to render triangleindex!\n");
	
	int indexCnt = fFaceTriangleVertexArray.length();
	if(indexCnt == 0) {
		return MStatus::kFailure;
	}

	//ei_triangle_list(indexCnt);
	ei_triangle_list(ei_tab(EI_DATA_TYPE_VECTOR, 1024));

	for(int i = 0;i<=indexCnt-3;i+=3)
	{
		ei_tab_add_index(fFaceTriangleVertexArray[i]);
		ei_tab_add_index(fFaceTriangleVertexArray[i+1]);
		ei_tab_add_index(fFaceTriangleVertexArray[i+2]);

	}
	ei_end_tab();

	return MStatus::kSuccess;
}

MStatus MeshWriter::outputShader( ostream& os )
{
	MGlobal::displayInfo("begin to output shaders!\n");
	if(fShaderArray.length() == 0){
		return MStatus::kFailure;
	}

	for(int i = 0;i<fShaderArray.length();++i)
	{
		MPlugArray connections;
		MFnDependencyNode shaderGroup(fShaderArray[i]);
		MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
		shaderPlug.connectedTo(connections,true,false);

		for(int j = 0;j<connections.length();++j)
		{
			if(connections[j].node().hasFn(MFn::kLambert)){
				MFnLambertShader lambertShader(connections[j].node());
				os<<"shader "<<"\""<<lambertShader.name().asChar()<<"\"\n";
				outputTabs(os,1);os<<"param_string \"desc\" \"plastic\"\n";
				outputTabs(os,1);os<<StringPrintf("param_vector \"Cs\" %.6lf %.6lf %.6lf\n",
												   lambertShader.color().r
												  ,lambertShader.color().g
												  ,lambertShader.color().b);
				outputTabs(os,1);os<<StringPrintf("param_vector \"Kd\" %.6lf %.6lf %.6lf\n",
												   lambertShader.diffuseCoeff()
												  ,lambertShader.diffuseCoeff()
					                              ,lambertShader.diffuseCoeff());
				outputTabs(os,1);os<<StringPrintf("param_vector \"Ks\" 0.0\n");
				os<<"end shader\n";
				os<<"\n";

				fMaterialName = MString("mtl"+lambertShader.name());
				os<<"material "<<"\""<<fMaterialName.asChar()<<"\"\n";
				outputTabs(os,1);os<<"add_surface "<<"\""<<lambertShader.name().asChar()<<"\"\n";
				os<<"end material\n";

				os<<"\n";
			}
		}
	}

	return MStatus::kSuccess;
}

MStatus MeshWriter::render_shader()
{
	MGlobal::displayInfo("begin to render shaders!\n");
	if(fShaderArray.length() == 0){
		return MStatus::kFailure;
	}

	//shadow shader
	ei_shader("opaque_shadow");
		ei_shader_param_string("desc","opaque");
	ei_end_shader();

	for(int i = 0;i<fShaderArray.length();++i)
	{
		MPlugArray connections;
		MFnDependencyNode shaderGroup(fShaderArray[i]);
		MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
		shaderPlug.connectedTo(connections,true,false);

		for(int j = 0;j<connections.length();++j)
		{
			//lambert
			if(connections[j].node().hasFn(MFn::kLambert)){
				MGlobal::displayInfo("find lambert shader!\n");
				MFnLambertShader lambertShader(connections[j].node());

				ei_shader(lambertShader.name().asChar()); 
				ei_shader_param_string("desc","plastic"); 
				ei_shader_param_vector("Cs",lambertShader.color().r
					,lambertShader.color().g
					,lambertShader.color().b);
				ei_shader_param_vector("Kd",  lambertShader.diffuseCoeff()
					,lambertShader.diffuseCoeff()
					,lambertShader.diffuseCoeff());
				ei_shader_param_scalar("Ks",0.0);
				ei_end_shader();

				fMaterialName = MString("mtl"+lambertShader.name());

				ei_material(fMaterialName.asChar());
					ei_add_surface(lambertShader.name().asChar());
					ei_add_shadow("opaque_shadow");
				ei_end_material();

			}

			//phong
			if(connections[j].node().hasFn(MFn::kPhong)){
				MGlobal::displayInfo("find phong shader!\n");
				MFnPhongShader phongShader(connections[j].node());
				ei_shader(phongShader.name().asChar()); 
				ei_shader_param_string("desc","plastic"); 
				ei_shader_param_vector("Cs",phongShader.color().r
					,phongShader.color().g
					,phongShader.color().b);
				ei_shader_param_vector("Kd",phongShader.diffuseCoeff()
					,phongShader.diffuseCoeff()
					,phongShader.diffuseCoeff());
				ei_shader_param_scalar("Ks",phongShader.reflectivity());
				ei_shader_param_scalar("roughness",1.0/phongShader.cosPower());
				ei_shader_param_vector("specularcolor",phongShader.specularColor().r,
					phongShader.specularColor().g,
					phongShader.specularColor().b);
				ei_end_shader();

				fMaterialName = MString("mtl"+phongShader.name());

				ei_material(fMaterialName.asChar());
					ei_add_surface(phongShader.name().asChar());
					ei_add_shadow("opaque_shadow");
				ei_end_material();
			}
		}
	}
	MGlobal::displayInfo("set material shader succeed\n");
	return MStatus::kSuccess;
}

void MeshWriter::outputInstance( ostream&os,MString instName )
{
	os<<"instance "<<"\""<<instName.asChar()<<"\"\n";
	outputTabs(os,1); os<<"element "<<"\""<<fname.asChar()<<"\"\n";
	outputTabs(os,1); os<<"add_material "<<"\""<<fMaterialName.asChar()<<"\"\n";
	outputTabs(os,1); outputTransform(os);
	os<<"end instance"<<"\n";
	os<<"\n";
}

void MeshWriter::render_instance(MString instName)
{
	ei_instance(instName.asChar());
	    ei_element(fname.asChar());
		ei_add_material(fMaterialName.asChar());
		render_transform();
	ei_end_instance();

}
