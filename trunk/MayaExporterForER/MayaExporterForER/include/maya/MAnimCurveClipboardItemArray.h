#ifndef _MAnimCurveClipboardItemArray
#define _MAnimCurveClipboardItemArray
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
// CLASS:    MAnimCurveClipboardItemArray
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MAnimCurveClipboardItem.h>
#include <maya/MStatus.h>

// ****************************************************************************
// CLASS DECLARATION (MAnimCurveClipboardItemArray)

//! \ingroup OpenMayaAnim
//! \brief  Array of MAnimCurveClipboardItem data type 
/*!
	This class implements an array of MAnimCurveClipboardItems.  Common
	convenience functions are available, and the implementation is compatible
	with the internal Maya implementation so that it can be passed efficiently
	between plugins and internal maya data structures.
*/
class OPENMAYAANIM_EXPORT MAnimCurveClipboardItemArray
{

public:
					MAnimCurveClipboardItemArray();
					MAnimCurveClipboardItemArray(
									const MAnimCurveClipboardItemArray& other );
					~MAnimCurveClipboardItemArray();
 	const MAnimCurveClipboardItem&		operator[]( unsigned int index ) const;
 	MStatus			set( const MAnimCurveClipboardItem& element,
						 unsigned int index );
 	unsigned int		length() const;
 	MStatus			remove( unsigned int index );
 	MStatus			insert( const MAnimCurveClipboardItem & element,
							unsigned int index );
 	MStatus			append( const MAnimCurveClipboardItem & element );
 	MStatus			clear();
	void			setSizeIncrement ( unsigned int newIncrement );
	unsigned int		sizeIncrement () const;
	bool			isValid( unsigned int & failedIndex ) const;

BEGIN_NO_SCRIPT_SUPPORT:

    //!	NO SCRIPT SUPPORT
	MAnimCurveClipboardItemArray(
									const MAnimCurveClipboardItem src[],
									unsigned int count );
    //!	NO SCRIPT SUPPORT
	MStatus			get( MAnimCurveClipboardItem array[] ) const;

 	//!	NO SCRIPT SUPPORT
 	MAnimCurveClipboardItem&			operator[]( unsigned int index );

END_NO_SCRIPT_SUPPORT:
	static const char*				className();

protected:
// No protected members

private:
	bool							validate( unsigned int & index,
											  unsigned int rowCount ) const;

 	MAnimCurveClipboardItemArray&	operator = (
										const MAnimCurveClipboardItemArray&);
 	void*							fArray;
};

#endif /* __cplusplus */
#endif /* _MAnimCurveClipboardItemArray */
