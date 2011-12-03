#include "LightWriter.h"

LightWriter::LightWriter(MDagPath dagPath, MStatus status) : DagNodeWriter(dagPath,status)
{
	fLight = new MFnLight(dagPath,&status);
	fname = fLight->name();
	fInstName = MString("inst"+fname);
}

LightWriter::~LightWriter()
{
	if(fLight!=NULL) delete fLight;
}

MStatus LightWriter::ExtractInfo()
{
	MGlobal::displayInfo("begin to extract info of light!\n");

	MStatus status;
	fIntensity = fLight->intensity(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}

	fColor = fLight->color(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}

	return MStatus::kSuccess;
}

MStatus LightWriter::WriteToFile( ostream& os )
{
	MGlobal::displayInfo("begin to write light info to file!\n");

	//light_shader
	os<<"shader \"point_light_shader\"\n";
	outputTabs(os,1); os<<"param_string \"desc\" \"pointlight\"\n";
	outputTabs(os,1); os<<"param_scalar \"intensity\" "<<fIntensity<<"\n";
	outputTabs(os,1); os<<"param_vector \"lightcolor\" "<<fColor.r<<" "
														<<fColor.g<<" "
														<<fColor.b<<"\n";
	os<<"end shader\n";
	os<<"\n";

	//light
	os<<"light "<<"\""<<fname.asChar()<<"\"\n";
	outputTabs(os,1); os<<"add_light \"point_light_shader\"\n";
	outputTabs(os,1); os<<"origin 0.0 0.0 0.0\n";
	os<<"end light\n";
	os<<"\n";

	outputInstance(os,fInstName);
	return MStatus::kSuccess;
}
