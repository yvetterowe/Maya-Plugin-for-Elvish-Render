#ifndef _MRenderTarget
#define _MRenderTarget
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
// CLASS:    MRenderTarget
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MTypes.h>

class MHardwareRenderer;
class THviewportRenderer;
class MRenderingInfo;
class MImage;

// ****************************************************************************
// CLASS DECLARATION (MRenderTarget)

//! \ingroup OpenMayaRender
//! \brief Information to perform rendering into a hardware render target.
/*!
	MRenderTarget is a class contains information about a given
	hardware render target.
*/

class OPENMAYARENDER_EXPORT MRenderTarget
{
public:
	//! Width of render target in pixels
	unsigned int	width() const;

	//! Height of render target in pixels
	unsigned int	height() const;

	void			makeTargetCurrent() const;
	MStatus			writeColorBuffer( const MImage &image, signed short x = 0, signed short y = 0,
										bool writeDepth = false) const;

protected:
	// No protected data

private:
	MRenderTarget();
	~MRenderTarget();
	void			set( void * );
	void			setWidth( unsigned int w ) { fWidth = w; }
	void			setHeight( unsigned int h ) { fHeight = h; }

	friend class MHardwareRenderer;
	friend class MRenderingInfo;

	// Render target dimensions
	unsigned int	fWidth;
	unsigned int	fHeight;

	void			*fRenderTarget;
};

#endif // _MRenderTarget
#endif
