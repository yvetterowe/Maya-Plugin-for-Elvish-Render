#ifndef _MFnDistanceManip
#define _MFnDistanceManip
//
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
// CLASS:    MFnDistanceManip
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MFnBase.h>
#include <maya/MFnManip3D.h>
#include <maya/MObject.h>

// ****************************************************************************
// CLASS DECLARATION (MFnDistanceManip)

//! \ingroup OpenMayaUI MFn
//! \brief DistanceManip function set
/*!
The DistanceManip allows the user to manipulate a point that is
constrained to move along a line. This manipulator generates a single
floating point value. Scaling factors can be used to determine how
int the manipulator appears when it is drawn.
*/
class OPENMAYAUI_EXPORT MFnDistanceManip : public MFnManip3D
{
	declareDagMFn(MFnDistanceManip, MFnManip3D);

public:
	MObject		create(MStatus *ReturnStatus = NULL);
	MObject		create(const MString &manipName,
					   const MString &distanceName,
					   MStatus *ReturnStatus = NULL);
	MStatus		connectToDistancePlug(MPlug &distancePlug);
	MStatus		setStartPoint(const MPoint &point);
	MStatus		setDirection(const MVector &vector);
	MStatus		setDrawStart(bool state);
	MStatus		setDrawLine(bool state);
	MStatus		setScalingFactor(double scalingFactor);
	bool		isDrawStartOn(MStatus *ReturnStatus = NULL) const;
	bool		isDrawLineOn(MStatus *ReturnStatus = NULL) const;
	double		scalingFactor(MStatus *ReturnStatus = NULL) const;
	unsigned int	distanceIndex(MStatus *ReturnStatus = NULL) const;
	unsigned int	directionIndex(MStatus *ReturnStatus = NULL) const;
	unsigned int	startPointIndex(MStatus *ReturnStatus = NULL) const;
	unsigned int	currentPointIndex(MStatus *ReturnStatus = NULL) const;

BEGIN_NO_SCRIPT_SUPPORT:

 	declareDagMFnConstConstructor( MFnDistanceManip, MFnManip3D );

END_NO_SCRIPT_SUPPORT:

protected:
// No protected members

private:
// No private members

};

#endif /* __cplusplus */
#endif /* _MFnDistanceManip */
