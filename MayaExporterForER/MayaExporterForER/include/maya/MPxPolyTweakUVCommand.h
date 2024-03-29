#ifndef _MPxPolyTweakUVCommand
#define _MPxPolyTweakUVCommand
//-
// ==========================================================================
// Copyright (C) 1995 - 2006 Autodesk, Inc., and/or its licensors.  All
// rights reserved.
//
// The coded instructions, statements, computer programs, and/or related
// material (collectively the "Data") in these files contain unpublished
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its
// licensors,  which is protected by U.S. and Canadian federal copyright law
// and by international treaties.
//
// The Data may not be disclosed or distributed to third parties or be
// copied or duplicated, in whole or in part, without the prior written
// consent of Autodesk.
//
// The copyright notices in the Software and this entire statement,
// including the above license grant, this restriction and the following
// disclaimer, must be included in all copies of the Software, in whole
// or in part, and all derivative works of the Software, unless such copies
// or derivative works are solely in the form of machine-executable object
// code generated by a source language processor.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
// AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED
// WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF
// NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE,
// OR ARISING FROM A COURSE OF DEALING, USAGE, OR TRADE PRACTICE. IN NO
// EVENT WILL AUTODESK AND/OR ITS LICENSORS BE LIABLE FOR ANY LOST
// REVENUES, DATA, OR PROFITS, OR SPECIAL, DIRECT, INDIRECT, OR
// CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK AND/OR ITS LICENSORS HAS
// BEEN ADVISED OF THE POSSIBILITY OR PROBABILITY OF SUCH DAMAGES.
// ==========================================================================
//+
//
// CLASS:    MPxPolyTweakUVCommand
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES

#include <maya/MTypes.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>

// ****************************************************************************
// DECLARATIONS

class MPxContext;
class MArgParser;
class MArgDatabase;
class MIntArray;
class MFloatArray;
class MFnMesh;
class MObject;

// ****************************************************************************
// CLASS DECLARATION (MPxPolyTweakUVCommand)

//! \ingroup OpenMaya MPx
//! \brief Base class used for moving polygon UV's.
/*!
This is the base class for UV editing commands on polygonal objects.

The purpose of this command class is to simplify the process of moving
UVs on a polygonal object. The use is only required to provide the new
positions of the UVs that have been modified.
*/
class OPENMAYA_EXPORT MPxPolyTweakUVCommand : public MPxCommand
{
public:
							MPxPolyTweakUVCommand ();
	virtual					~MPxPolyTweakUVCommand ();
	virtual MStatus			parseSyntax (MArgDatabase &argData);
	virtual MStatus			getTweakedUVs (const MObject & mesh,
										   MIntArray & uvList,
										   MFloatArray & uPos,
										   MFloatArray & vPos);
	static MSyntax			newSyntax ();

private:

	virtual MStatus			doIt(const class MArgList &);
	virtual MStatus			undoIt();
	virtual MStatus			redoIt();
	virtual bool			isUndoable() const;

	void * fData;
};

#endif /* __cplusplus */
#endif /* _MPxPolyTweakUVCommand */
