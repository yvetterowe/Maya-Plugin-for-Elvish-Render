#ifndef _MClothConstraintCmd
#define _MClothConstraintCmd
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
// CLASS:    MClothConstraintCmd
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES
#include <maya/MStatus.h>
#include <maya/MClothConstraint.h>
#include <maya/MClothConstraintBridge.h>



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
class MClothParticle;

// ****************************************************************************
// CLASS DECLARATION (MClothConstraintCmd)

/*!
\internal
\ingroup cloth
\brief Interface for cloth solver constraint command.

Interface that must be implemented by solver writers to override
custom behavior for various constraints. MayaCloth plugin implements
MClothConstraintCmd in plugin (PinConstraint, TransformConstraint).

Solver should provide MClothConstraintBridge object to override custom
behavior during constraint evaluation. MClothConstraintBridge object
should get set in addCommand method of MClothSystem implementation.
*/
class MAYACLOTH_EXPORT MClothConstraintCmd : public MClothConstraint
{
public:
	MClothConstraintCmd( MClothParticle* particle );
	MClothConstraintCmd( MClothParticle* particle,
						 double shear,
						 double damp );
	virtual ~MClothConstraintCmd();

	virtual MPoint	desired_position( double frame ) = 0;

	void	setSoft(bool soft);
	bool	isSoft() const;

	void	setStrength( double shear, double damp );
	double	getShear() const;
	double	getDamp() const;

	bool	userDefined();

public:
	// This must be provided by MClothConstraintBridge.
	MStatus	execute();

	void setCustomConstraintBehavior(MClothConstraintBridge* impl);
	MClothConstraintBridge* getCustomConstraintBehavior() const;

protected:
	MClothConstraintBridge* fCustomConstraint;
	double		fShear, fDamp;
	bool		fSoft;

private:
	// No private members
};

#endif /* __cplusplus */
#endif /* _MClothConstraintCmd */
