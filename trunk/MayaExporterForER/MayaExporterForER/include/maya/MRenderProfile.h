#ifndef _MRenderProfile
#define _MRenderProfile
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
// CLASS:    MRenderProfile
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MTypes.h>
#include <maya/MString.h>

// ****************************************************************************
// CLASS DECLARATION (MRenderProfile)

//! \ingroup OpenMayaRender
//! \brief Render profile.
/*!
The MRenderProfile class describes the rendering APIs and algorithms supported
by a given rendering entity (e.g. a shading node, a renderer). A single profile
can contain multiple entries allowing, for example, a shading node to specify
that it supports both OpenGL and Direct3D rendering. The profile entries refer
to renderers rather than rendering APIs as the rendering elements may depend on
specific services, information or algorithms implemented by the renderer (e.g.
a global light table, or render state cache).
*/
class OPENMAYARENDER_EXPORT MRenderProfile
{
public:

	MRenderProfile();
	~MRenderProfile();

	//! Maya's internal renderers.
	typedef enum
	{
		kMayaSoftware,	//!< \nop
		kMayaOpenGL,	//!< \nop
		kMayaD3D	//!< \nop
	} MStandardRenderer;

	unsigned int	numberOfRenderers() const;

	void			addRenderer( MStandardRenderer renderer);

	void			addRenderer( const MString& name, float version);

	bool			hasRenderer( MStandardRenderer renderer ) const;

	bool			hasRenderer( const MString &name, float version ) const;

protected:

	void*			_ptr;

private:
// No private members

};

#endif
#endif // _MRenderProfile
