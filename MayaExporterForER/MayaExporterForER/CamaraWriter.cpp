#include "CamaraWriter.h"
#include "stringprintf.h"


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
	MGlobal::displayInfo("begin to export camera info!\n");

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

/*MStatus CamaraWriter::WriteToFile( ostream& os )
{
	MGlobal::displayInfo("begin to write camera info to file!\n");

	os<<"camera "<<"\""<<fname.asChar()<<"\""<<"\n";
	outputOutPutConfig(os);
	//outputTabs(os,1);os<<"add_imager \"gamma_correction_shader\"\n";
	outputTabs(os,1); os<<StringPrintf("focal %.6lf\n",fFocal/10.0);
	outputTabs(os,1); os<<StringPrintf("aperture %.6lf\n",fAperture*2.54);
	outputTabs(os,1); os<<StringPrintf("aspect %.6lf\n",fAspect);
	outputTabs(os,1); os<<"resolution 640 480"<<"\n";
	os<<"end camera"<<"\n";
	os<<"\n";

	outputInstance(os,fInstName);

	return MStatus::kSuccess;
}*/

MStatus CamaraWriter::render()
{
	MGlobal::displayInfo("render camera !\n");

	ei_camera(fname.asChar());

	//outputOutPutConfig(os);
	render_configure();
	ei_add_imager("gamma_correction_shader");

	ei_focal(fFocal/10.0);
	ei_aperture(fAperture*2.54);
	ei_aspect(fAspect);
	//ei_resolution(640,480);
	ei_resolution(fWidth,fHeight);

	ei_end_camera();

	render_instance(fInstName);

	return MStatus::kSuccess;

}

/*void CamaraWriter::outputOutPutConfig( ostream& os )
{
	outputTabs(os,1); os<<"output "<<"\"test.bmp\" \"bmp\" \"rgb\""<<"\n";
	outputTabs(os,2); os<<"output_variable \"color\" \"vector\""<<"\n";
	outputTabs(os,1); os<<"end output"<<"\n";
}*/

void CamaraWriter::render_configure()
{
	char	cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char	output_filename[ EI_MAX_FILE_NAME_LEN ];
	ei_get_current_directory(cur_dir);


	ei_append_filename(output_filename,cur_dir, "frame01.bmp");
	ei_output("test.bmp","bmp", EI_IMG_DATA_RGB);
	//ei_output("C:\\Users\\lenovo\\code\\cg\\ElvishRenderer\\build\\r990\\x86\\test.bmp","bmp",EI_IMG_DATA_RGB);
	ei_output_variable("color", EI_DATA_TYPE_VECTOR);
	ei_end_output();
}

void CamaraWriter::setResolution( int w,int h )
{
	fWidth = w;
	fHeight = h;
}
