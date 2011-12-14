#ifndef _MClothParticle
#define _MClothParticle
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
// ****************************************************************************
//
// CLASS:    MClothParticle
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES
#include <maya/MStatus.h>



// MayaCloth Export
#ifdef _WIN32
#	ifndef MAYACLOTH_EXPORT
#		ifdef MAYACLOTH_DLL
#			define MAYACLOTH_EXPORT _declspec( dllexport )
#		else
#			define MAYACLOTH_EXPORT _declspec( dllimport )
#		endif // MAYACLOTH_DLL
#	endif // MAYACLOTH_EXPORT
#elif defined(AW_HIDE_SYMBOLS) && defined(MAYACLOTH_DLL)
#	ifndef MAYACLOTH_EXPORT
#		define MAYACLOTH_EXPORT __attribute__ ((visibility("default")))
#	endif
#else
#	ifndef MAYACLOTH_EXPORT
#		define MAYACLOTH_EXPORT
#	endif
#endif // _WIN32

// ****************************************************************************
// DECLARATIONS

class MPoint;
class MDoubleArray;
class MVector;
class MClothEdge;
class MClothTriangle;
class MClothMaterial;
class MClothPolyhedron;

// ****************************************************************************
// CLASS DECLARATION (MClothParticle)

/*!
\internal
\ingroup cloth
\brief Interface for cloth particle.

This class provides interface for cloth particle system.
Cloth solver writers should derive their particle class
from this class. Common convenience methods are available.
*/
class MAYACLOTH_EXPORT MClothParticle
{
public:
	MClothParticle();
	virtual ~MClothParticle();

	virtual MPoint getPosition( MStatus* ReturnStatus = NULL ) const = 0;
	virtual void setPosition( const MPoint &position,
							  MStatus* ReturnStatus = NULL ) = 0;

	virtual MVector getVelocity( MStatus* ReturnStatus = NULL ) const = 0;
	virtual void setVelocity( const MVector &velocity,
							  MStatus* ReturnStatus = NULL ) = 0;

	virtual MVector getAccelaration( MStatus* ReturnStatus = NULL ) const = 0;
	virtual void setAccelaration( const MVector &acc,
								  MStatus* ReturnStatus = NULL ) = 0;

	virtual MClothEdge* getEdge( unsigned int index,
								 MStatus* ReturnStatus = NULL ) const = 0;
	virtual unsigned int numEdges( MStatus* ReturnStatus = NULL ) const = 0;

	virtual bool ignoreSolid( MStatus* ReturnStatus = NULL ) const = 0;
	virtual void ignoreSolid( bool isIgnored,
							  MStatus* ReturnStatus = NULL ) = 0;

	virtual bool ignoreClothCollision( MStatus* ReturnStatus = NULL ) const = 0;
	virtual void ignoreClothCollision( bool isIgnored,
									   MStatus* ReturnStatus = NULL ) = 0;

	virtual double getMass( MStatus* ReturnStatus = NULL ) const = 0;
	virtual double getOriginalMass( MStatus* ReturnStatus = NULL ) const = 0;
	virtual void setMass( const double mass,
						  MStatus* ReturnStatus = NULL ) = 0;
	virtual void setForceMultiplier( const double forceMultiplier,
						  MStatus* ReturnStatus = NULL ) = 0;

	virtual MClothMaterial* getMaterial( MStatus* ReturnStatus = NULL ) const = 0;

	virtual unsigned getIndex() const = 0;

	virtual bool equals(MClothParticle *other) const;

	bool collideWithPolyhedron( MClothPolyhedron *poly = NULL );
	
protected:
	// No protected members

private:
	friend class CpHParticle;
	void *fData;
};

#endif /* __cplusplus */
#endif /* _MClothParticle */
