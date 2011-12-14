#include "MayaExporterForER.h"

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MIOStream.h>
#include <maya/MFStream.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnAreaLight.h>
#include <maya/MFnDirectionalLight.h>
#include <maya/MFnSpotLight.h>

#include <sstream>
#include "DagNodeWriter.h"
#include "MeshWriter.h"
#include "CamaraWriter.h"
#include "LightWriter.h"

MayaExporterForER::MayaExporterForER()
{

}

MayaExporterForER::~MayaExporterForER()
{

}

MStatus MayaExporterForER::writer( const MFileObject& file, 
								   const MString& optionsString, 
								   MPxFileTranslator::FileAccessMode mode )
{
#if defined (OSMac_)
	char nameBuffer[MAXPATHLEN];
	strcpy (nameBuffer, file.fullName().asChar());
	const MString fileName(nameBuffer);
#else
	const MString fileName = file.fullName();
#endif
	std::ostringstream out;
	//out<<std::fixed();
	ofstream newFile(fileName.asChar(), ios::out);
	if (!newFile) {
		MGlobal::displayError(fileName + ": could not be opened for reading");
		return MS::kFailure;
	}
	out.setf(ios::unitbuf);
	out.precision(6);
	out.setf(ios::fixed, ios::floatfield);

	if (MPxFileTranslator::kExportAccessMode == mode) {
		if (MStatus::kFailure == exportAll(newFile)) {
			return MStatus::kFailure;
		}
	}  
	else {
		return MStatus::kFailure;
	}

	newFile << out.str();
	newFile.flush();
	newFile.close();

	MGlobal::displayInfo("Export to " + fileName + " successful!");
	return MS::kSuccess;
}

bool MayaExporterForER::haveWriteMethod() const
{
	return true;
}

bool MayaExporterForER::haveReadMethod() const
{
	return false;
}

bool MayaExporterForER::canBeOpened() const
{
	return true;
}

bool MayaExporterForER::isVisible( MFnDagNode& fnDag, MStatus& status )
{
	if(fnDag.isIntermediateObject())
		return false;

	MPlug visPlug = fnDag.findPlug("visibility", &status);
	
	if (MStatus::kFailure == status) {
		MGlobal::displayError("MPlug::findPlug");
		return false;
	} 
	else {
		bool visible;
		status = visPlug.getValue(visible);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MPlug::getValue");
		}
		return visible;
	}
}

MStatus MayaExporterForER::exportAll( ostream& os )
{
	MGlobal::displayInfo("begin to export all!");
	
	MStatus status;

	outputOptions(os);
	
	MItDag itDag(MItDag::kDepthFirst, MFn::kInvalid, &status);

	if (MStatus::kFailure == status) {
		MGlobal::displayError("MItDag::MItDag");
		return MStatus::kFailure;
	}

	for(;!itDag.isDone();itDag.next()) 
	{
		MDagPath dagPath;

		if (MStatus::kFailure == itDag.getPath(dagPath)) {
			MGlobal::displayError("MDagPath::getPath");
			return MStatus::kFailure;
		}

		MFnDagNode dagNode(dagPath);

		if(isVisible(dagNode, status) && MStatus::kSuccess == status) {
			processDagNode(dagPath,os);
		}		
	}

	//for default perspective camera
	MItDag itCamera(MItDag::kDepthFirst, MFn::kCamera, &status);

	if (MStatus::kFailure == status) {
		MGlobal::displayError("MItDag::MItDag");
		return MStatus::kFailure;
	}

	for(;!itCamera.isDone();itCamera.next()) 
	{
		MFnCamera cam(itCamera.item());
		if(cam.name()== "perspShape"){
			MGlobal::displayInfo("find persp camera\n");
			MDagPath dagCamPath;

			if (MStatus::kFailure == itCamera.getPath(dagCamPath)) {
				MGlobal::displayError("MDagPath::getPath");
				return MStatus::kFailure;
			}

			processDagNode(dagCamPath,os);		

		}
	}
	outputRenderConfig(os);

	return MStatus::kSuccess;
}

MStatus MayaExporterForER::processDagNode( const MDagPath dagPath, ostream& os )
{
	MGlobal::displayInfo("begin to process node!\n");
	
	MStatus status;
	DagNodeWriter* pWriter = createDagNodeWriter(dagPath, status);
	if(NULL == pWriter){
		MGlobal::displayInfo("pWriter null!");
		return MStatus::kFailure;
	}
	
	if (MStatus::kFailure == status) {
		delete pWriter;
		MGlobal::displayError("new writer fail!");
		return MStatus::kFailure;
	}

	instanceContainer.append(pWriter->GetInstName());
	
	if(dagPath.apiType() == MFn::kCamera){
		camaraInstance = pWriter->GetInstName();
	}

	if (MStatus::kFailure == pWriter->ExtractInfo()) {
		MGlobal::displayError("extractInfo fail!");
		delete pWriter;
		return MStatus::kFailure;
	}
	if (MStatus::kFailure == pWriter->WriteToFile(os)) {
		MGlobal::displayError("write to file fail!");
		delete pWriter;
		return MStatus::kFailure;
	}

	delete pWriter;
	return MStatus::kSuccess;
}

DagNodeWriter* MayaExporterForER::createDagNodeWriter( const MDagPath dagPath, MStatus& status )
{
	switch(dagPath.apiType())
	{
	case MFn::kMesh:
		{
			MGlobal::displayInfo("new a mesh writer!\n");
			return new MeshWriter(dagPath,status);
			break;
		}
	case MFn::kCamera:
		{
			return new CamaraWriter(dagPath,status);
			break;
		}
	case MFn::kPointLight:
		{
			return new LightWriter(dagPath,status);
			break;
		}
	case MFn::kAreaLight:
		{
			return new LightWriter(dagPath,status);
			break;
		}
	case MFn::kDirectionalLight:
		{
			return new LightWriter(dagPath,status);
			break;
		}
	case MFn::kSpotLight:
		{
			return new LightWriter(dagPath,status);
			break;
		}
	default: 
		{
			return NULL;
			break;
		}	
	}
}

MString MayaExporterForER::defaultExtension() const
{
	return MString("ess");
}

void MayaExporterForER::outputRenderConfig( ostream& os )
{
	os<<"instgroup \"world\""<<"\n";
	for(int i = 0;i<instanceContainer.length();++i)
	{
		os<<"\t"<<"add_instance "<<"\""<<instanceContainer[i].asChar()<<"\"\n";
	}
	os<<"end instgroup"<<"\n";
	os<<"\n";

	os<<"render "<<"\"world\" "<<"\""<<camaraInstance.asChar()<<"\" "<<"\"opt\"\n";
}

void MayaExporterForER::outputOptions( ostream& os )
{
	os<<"options \"opt\""<<"\n";
	os<<"\t"<<"contrast "<<opContrast.r<<" "
						 <<opContrast.g<<" "
						 <<opContrast.b<<" "
						 <<opContrast.a<<"\n";
	os<<"\t"<<"samples "<<opSample.sMin<<" "
						<<opSample.sMax<<"\n";
	if(opFilter.fTypeId>0){
		os<<"\t"<<"filter " <<"\""<<opFilter.fName.asChar()<<"\""<<" "
			<<opFilter.fSize<<"\n";
	}
	os<<"\t"<<"trace_depth "<<opTraceDepth.reflect<<" "
							<<opTraceDepth.refrect<<" "
							<<opTraceDepth.sum<<"\n";
	os<<"\t"<<"globillum "<<opGlobalIllumi<<"\n";
	os<<"\t"<<"finalgather "<<opFinalGather<<"\n";
	os<<"end options"<<"\n";
	os<<"\n";
}

void MayaExporterForER::setContrast( double r,double g,double b,double a )
{
	opContrast.r = r;
	opContrast.g = g;
	opContrast.b = b;
	opContrast.a = a;
}

void MayaExporterForER::setSample( int sMin,int sMax )
{
	opSample.sMin = sMin;
	opSample.sMax = sMax;
}

void MayaExporterForER::setFilter( MString t,int s )
{
	opFilter.fName = t;
	opFilter.fSize = s;

	if(t == "none"){
		opFilter.fTypeId = 0;
	}
	else if(t == "box"){
		opFilter.fTypeId = 1;
	}
	else if(t == "triangle"){
		opFilter.fTypeId = 2;
	}
	else if(t == "catmullrom"){
		opFilter.fTypeId = 3;
	}
	else if(t == "gaussian"){
		opFilter.fTypeId = 4;
	}
	else if(t == "sinc"){
		opFilter.fTypeId = 5;
	}
	else{
		opFilter.fTypeId = -1;
	}
}

void MayaExporterForER::setTraceDepth( int tRl,int tRR,int tSum )
{
	opTraceDepth.reflect = tRl;
	opTraceDepth.refrect = tRR;
	opTraceDepth.sum = tSum;
}

void MayaExporterForER::setGlobalIllumi( int g )
{
	opGlobalIllumi = g;
}

void MayaExporterForER::setFinalGather( int f )
{
	opFinalGather = f;
}

void MayaExporterForER::parseArglist( const MArgList& args )
{
	for(int i = 0;i<args.length();++i)
	{
		if(args.asString(i) == "-contrast"){
			setContrast(args.asDouble(i+1),args.asDouble(i+2),args.asDouble(i+3),args.asDouble(i+4));
		}
		else if(args.asString(i) == "-sample"){
			setSample(args.asInt(i+1),args.asInt(i+2));
		}
		else if(args.asString(i) == "-filter"){
			setFilter(args.asString(i+1),args.asInt(i+2));
		}
		else if(args.asString(i) == "-traceDepth"){
			setTraceDepth(args.asInt(i+1),args.asInt(i+2),args.asInt(i+3));
			MGlobal::displayInfo(args.asString(i));
			MGlobal::displayInfo(args.asString(i+1));
			MGlobal::displayInfo(args.asString(i+2));
			MGlobal::displayInfo(args.asString(i+3));
		}
		else if(args.asString(i) == "-globalIllumi"){
			setGlobalIllumi(args.asInt(i+1));
		}
		else if(args.asString(i) == "-finalGather"){
			setFinalGather(args.asInt(i+1));
		}
		else if(args.asString(i) == "-resolution"){
		}
		else if(args.asString(i) == "-gamma"){

		}
		else{
			MGlobal::displayInfo("this is not a flag!\n");
		}
	}
}
