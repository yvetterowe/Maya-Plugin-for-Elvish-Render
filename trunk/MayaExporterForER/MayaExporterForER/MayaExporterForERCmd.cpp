//
// Copyright (C) 
// 
// File: MayaExporterForERCmd.cpp
//
// MEL Command: MayaExporterForER
//
// Author: Maya Plug-in Wizard 2.0
//

// Includes everything needed to register a simple MEL command with Maya.
// 
#include <maya/MSimple.h>
#include <maya/MSimple.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MIOStream.h>
#include <maya/MFileObject.h>
#include <maya/MPxFileTranslator.h>
#include <stdio.h>

#include "MayaExporterForER.h"


// Use helper macro to register a command with Maya.  It creates and
// registers a command that does not support undo or redo.  The 
// created class derives off of MPxCommand.
//
//DeclareSimpleCommand( MayaExporterForER, "", "2012");

class ExportMayaScene : public MPxCommand
{
public:
	MStatus doIt( const MArgList& args );
	static void* creator();
};


MStatus ExportMayaScene::doIt( const MArgList& args )
//
//	Description:
//		implements the MEL MayaExporterForER command.
//
//	Arguments:
//		args - the argument list that was passes to the command from MEL
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - command failed (returning this value will cause the 
//                     MEL script that is being run to terminate unless the
//                     error is caught using a "catch" statement.
//
{
	MStatus stat = MS::kSuccess;

	// Since this class is derived off of MPxCommand, you can use the 
	// inherited methods to return values and set error messages
	//

	MayaExporterForER* exporter = new MayaExporterForER;
	MString filename("fout.txt");
	MFileObject file;
	file.setRawName(filename);

	exporter->writer(file,"none",MPxFileTranslator::kExportAccessMode);

	setResult( "MayaExporterForER command executed!\n" );

	delete exporter;

	return stat;
}

void* ExportMayaScene::creator()
{
	return new ExportMayaScene;
}

MStatus initializePlugin( MObject obj ) {
	MFnPlugin plugin( obj, "Autodesk", "1.0", "Any" );
	plugin.registerCommand( "ExportMayaScene", ExportMayaScene::creator );
	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj ) {
	MFnPlugin plugin( obj );
	plugin.deregisterCommand( "ExportMayaScene" );
	return MS::kSuccess;
}
