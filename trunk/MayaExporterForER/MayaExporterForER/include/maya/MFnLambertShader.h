#ifndef _MFnLambertShader
#define _MFnLambertShader
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
// CLASS:    MFnLambertShader
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MFnDependencyNode.h>

// ****************************************************************************
// DECLARATIONS

class MColor;

// ****************************************************************************
// CLASS DECLARATION (MFnLambertShader)

//! \ingroup OpenMaya MFn
//! \brief Manage Lambert shaders.
/*!
 MFnLambertShader facilitates creation and manipulation of dependency graph
 nodes representing lambertian shaders.
*/ 
class OPENMAYA_EXPORT MFnLambertShader : public MFnDependencyNode
{
	declareMFn( MFnLambertShader, MFnDependencyNode );

public:
	MObject     create( bool UIvisible = true, MStatus * ReturnStatus = NULL );
	short       refractedRayDepthLimit( MStatus * ReturnStatus = NULL ) const;
	MStatus     setRefractedRayDepthLimit( const short& new_limit );
	float       refractiveIndex( MStatus * ReturnStatus = NULL ) const;
	MStatus     setRefractiveIndex( const float& refractive_index );
	bool        rtRefractedColor( MStatus * ReturnStatus = NULL ) const;
	MStatus     setRtRefractedColor( const bool& rt_refracted_color );
	float       diffuseCoeff( MStatus * ReturnStatus = NULL ) const;
	MStatus     setDiffuseCoeff( const float& diffuse_coeff );
	MColor      color( MStatus * ReturnStatus = NULL ) const;
	MStatus     setColor( const MColor & col );
	MColor      transparency( MStatus * ReturnStatus = NULL ) const;
	MStatus     setTransparency( const MColor & transp );
	MColor      ambientColor( MStatus * ReturnStatus = NULL ) const;
	MStatus     setAmbientColor( const MColor & ambient_color );
	MColor      incandescence( MStatus * ReturnStatus = NULL ) const;
	MStatus     setIncandescence( const MColor & incand );
	float       translucenceCoeff( MStatus * ReturnStatus = NULL ) const;
	MStatus     setTranslucenceCoeff( const float& translucence_coeff );
	float       glowIntensity( MStatus * ReturnStatus = NULL ) const;
	MStatus     setGlowIntensity( const float& glow_intensity );
	bool        hideSource( MStatus * ReturnStatus = NULL ) const;
	MStatus     setHideSource( const bool& hide_source );

BEGIN_NO_SCRIPT_SUPPORT:

 	declareMFnConstConstructor( MFnLambertShader, MFnDependencyNode );

END_NO_SCRIPT_SUPPORT:

protected:
// No protected members

private:
// No private members
};

#endif /* __cplusplus */
#endif /* _MFnLambertShader */
