#include "DirectLightWriter.h"

#include "stringprintf.h"

DirectLightWriter::DirectLightWriter( MDagPath dagPath, MStatus status ) :LightWriter(dagPath,status)
{
	fDirectLight = new MFnDirectionalLight(dagPath);
	fname = fDirectLight->name();
	fInstName = MString("inst"+fname);
	fShaderName = MString(fname+"_shader");
}

DirectLightWriter::~DirectLightWriter()
{
	if(fDirectLight!=NULL){
		delete fDirectLight;
	}
}

MStatus DirectLightWriter::ExtractInfo()
{
	MGlobal::displayInfo("begin to extract info of directlight!\n");

	MStatus status;
	fIntensity = fDirectLight->intensity(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}

	fColor = fDirectLight->color(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}

	fDirection = fDirectLight->lightDirection(status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}

	fSpread = 1000.0;

	return MStatus::kSuccess;
}


MStatus DirectLightWriter::WriteToFile( ostream& os )
{
	MGlobal::displayInfo("begin to write directlight info to file!\n");

	//light_shader
	os<<"shader "<<"\""<<fShaderName.asChar()<<"\"\n";
	outputTabs(os,1); os<<"param_string \"desc\" \"directlight\"\n";
	outputTabs(os,1); os<<StringPrintf("param_scalar \"intensity\" %.6lf\n",fIntensity);
	outputTabs(os,1); os<<StringPrintf("param_vector \"lightcolor\" %.6lf %.6lf %.6lf\n",fColor.r,fColor.g,fColor.b);	
	outputTabs(os,1); os<<StringPrintf("param_vector \"direction\" %.6lf %.6lf %.6lf\n",fDirection.x,fDirection.y,fDirection.z);
	outputTabs(os,1); os<<StringPrintf("param_scalar \"spread\" %.6lf\n",fSpread);
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

MStatus DirectLightWriter::render()
{
	MGlobal::displayInfo("render directlight !\n");

	//light_shader
	ei_shader(fShaderName.asChar());
	    ei_shader_param_string("desc","directlight");
		ei_shader_param_scalar("intensity",fIntensity);
		ei_shader_param_vector("lightcolor",fColor.r,fColor.g,fColor.b);
		ei_shader_param_vector("direction",fDirection.x,fDirection.y,fDirection.z);
		ei_shader_param_scalar("spread",fSpread);
	ei_end_shader();


	//light
	ei_light(fname.asChar());
	    ei_add_light(fShaderName.asChar());
	    ei_origin(fTranslation.x,fTranslation.y,fTranslation.z);
	ei_end_light();

	render_instance(fInstName);
	return MStatus::kSuccess;
}