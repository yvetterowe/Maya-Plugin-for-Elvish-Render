#include "PointLightWriter.h"

#include "stringprintf.h"

PointLightWriter::PointLightWriter( MDagPath dagPath, MStatus status ) :LightWriter(dagPath,status)
{
	fPointLight = new MFnPointLight(dagPath);
	fname = fPointLight->name();
	fInstName = MString("inst"+fname);
	fShaderName = MString(fname+"_shader");
}

PointLightWriter::~PointLightWriter()
{
	if(fPointLight!=NULL){
		delete fPointLight;
	}
}

MStatus PointLightWriter::ExtractInfo()
{
	MGlobal::displayInfo("begin to extract info of pointlight!\n");

	MStatus status;
	fIntensity = fPointLight->intensity(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}

	fColor = fPointLight->color(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}

	return MStatus::kSuccess;
}


MStatus PointLightWriter::WriteToFile( ostream& os )
{
	MGlobal::displayInfo("begin to write pointlight info to file!\n");

	//light_shader
	os<<"shader "<<"\""<<fShaderName.asChar()<<"\"\n";
	outputTabs(os,1); os<<"param_string \"desc\" \"pointlight\"\n";
	outputTabs(os,1); os<<StringPrintf("param_scalar \"intensity\" %.6lf\n",fIntensity);
	outputTabs(os,1); os<<StringPrintf("param_vector \"lightcolor\" %.6lf %.6lf %.6lf\n",fColor.r,fColor.g,fColor.b);
	os<<"end shader\n";
	os<<"\n";

	//light
	os<<"light "<<"\""<<fname.asChar()<<"\"\n";
	outputTabs(os,1); os<<"add_light "<<"\""<<fShaderName.asChar()<<"\"\n";
	outputTabs(os,1); os<<"origin 0.0 0.0 0.0\n";
	os<<"end light\n";
	os<<"\n";

	outputInstance(os,fInstName);
	return MStatus::kSuccess;
}