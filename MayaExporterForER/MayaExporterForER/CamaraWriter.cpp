#include "CamaraWriter.h"

CamaraWriter::CamaraWriter(MDagPath dagPath, MStatus status) : DagNodeWriter(dagPath,status)
{
	fCamara = new MFnCamera(dagPath,&status);
	fname = fCamara->name();
	fInstName = MString("inst" + fname);
}

CamaraWriter::~CamaraWriter()
{
	if(fCamara!=NULL) delete fCamara;
}

MStatus CamaraWriter::ExtractInfo()
{
	MGlobal::displayInfo("begin to export camara info!\n");
	
	MStatus status;
	fFocal = fCamara->focalLength(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("get focal!\n");

	fAperture = fCamara->horizontalFilmAperture(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("get aperture!\n");

	fAspect = fCamara->aspectRatio(&status);
	if(MStatus::kFailure == status){
		return MStatus::kFailure;
	}
	MGlobal::displayInfo("get aspect!\n");

	return MStatus::kSuccess;
}

MStatus CamaraWriter::WriteToFile( ostream& os )
{
	MGlobal::displayInfo("begin to write camera info to file!\n");
	os<<"camera "<<"\""<<fname.asChar()<<"\""<<"\n";
	outputOutPutConfig(os);
	outputTabs(os,1); os<<"focal "<<fFocal<<"\n";
	outputTabs(os,1); os<<"aperture "<<fAperture<<"\n";
	outputTabs(os,1); os<<"aspect "<<fAspect<<"\n";
	outputTabs(os,1); os<<"resolution 320 240"<<"\n";
	os<<"end camera"<<"\n";
	os<<"\n";

	outputInstance(os,fInstName);

	return MStatus::kSuccess;
}

void CamaraWriter::outputOutPutConfig( ostream& os )
{
	outputTabs(os,1); os<<"output "<<"\"test.bmp\" \"bmp\" \"rgb\""<<"\n";
	outputTabs(os,2); os<<"output_variable \"color\" \"vector\""<<"\n";
	outputTabs(os,1); os<<"end output"<<"\n";
}
