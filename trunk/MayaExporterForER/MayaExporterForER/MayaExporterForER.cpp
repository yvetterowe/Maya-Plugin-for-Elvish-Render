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

MStatus MayaExporterForER::writer( const MFileObject& file, const MString& optionsString, MPxFileTranslator::FileAccessMode mode )
{
#if defined (OSMac_)
	char nameBuffer[MAXPATHLEN];
	strcpy (nameBuffer, file.fullName().asChar());
	const MString fileName(nameBuffer);
#else
	const MString fileName = file.fullName();
#endif
	ofstream newFile(fileName.asChar(), ios::out);
	if (!newFile) {
		MGlobal::displayError(fileName + ": could not be opened for reading");
		return MS::kFailure;
	}
	newFile.setf(ios::unitbuf);

	//check which objects are to be exported, and invoke the corresponding
	//methods; only 'export all' and 'export selection' are allowed
	//
	if (MPxFileTranslator::kExportAccessMode == mode) {
		if (MStatus::kFailure == exportAll(newFile)) {
			return MStatus::kFailure;
		}
	} 
	else if (MPxFileTranslator::kExportActiveAccessMode == mode) {
		if (MStatus::kFailure == exportSelection(newFile)) {
			return MStatus::kFailure;
		}
	} 
	else {
		return MStatus::kFailure;
	}

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
	MStatus status;
	
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
			if (MStatus::kFailure == processDagNode(dagPath, os)) {
				return MStatus::kFailure;
			}
		}
	}
	return MStatus::kSuccess;
}

MStatus MayaExporterForER::exportSelection( ostream& os )
{
	MStatus status;
	MSelectionList selectionList;
	
	if (MStatus::kFailure == MGlobal::getActiveSelectionList(selectionList)) {
		MGlobal::displayError("MGlobal::getActiveSelectionList");
		return MStatus::kFailure;
	}

	MItSelectionList itSelectionList(selectionList, MFn::kInvalid, &status);   
	if (MStatus::kFailure == status) {
		return MStatus::kFailure;
	}

	for (itSelectionList.reset(); !itSelectionList.isDone(); itSelectionList.next()) 
	{
		MDagPath dagPath;

		if (MStatus::kFailure == itSelectionList.getDagPath(dagPath)) {
			MGlobal::displayError("MItSelectionList::getDagPath");
			return MStatus::kFailure;
		}

		if (MStatus::kFailure == processDagNode(dagPath, os)) {
			return MStatus::kFailure;
		}
	}
	
	return MStatus::kSuccess;
}

MStatus MayaExporterForER::processDagNode( const MDagPath dagPath, ostream& os )
{
	MStatus status;
	DagNodeWriter* pWriter = createDagNodeWriter(dagPath, status);
	
	if (MStatus::kFailure == status) {
		delete pWriter;
		MGlobal::displayError("new writer fail!");
		return MStatus::kFailure;
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
			return new MeshWriter(dagPath,status);
			break;
		}
	case MFn::kCamera:
		{
			return new CamaraWriter(dagPath,status);
			break;
		}
	case MFn::kLight:
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


