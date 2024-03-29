#ifndef _MThreadUtils
#define _MThreadUtils
//-
// ==========================================================================
// Copyright (C) 1995 - 2008 Autodesk, Inc., and/or its licensors.  All
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
// CLASS:    MThreadUtils
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MGlobal.h>

#if defined(_OPENMP)
#include <omp.h>
#endif

// ****************************************************************************
// CLASS DECLARATION (MThreadUtils)

//! \ingroup OpenMaya
//! \brief Helper methods for threaded plugins
/*!

   <b>Thread count controls</b>

   The user can control the number of threads currently running in
   Maya. This is useful for example to run rendering in a single
   thread when multiple frames are being rendered in parallel by
   different instances of Maya. All code should respect this thread
   count, so if a plugin is threaded, it needs to query the current
   thread count being used by Maya and set that threadcount for
   itself.

   For a plugin-local thread pool using Win32 or pthreads, use the
   getNumThreads() method to check the required number of threads and
   set the number of threads in the plugin thread pool appropriately.

   If using the Maya API MThreadPool class there is no need to do
   anything as this is an internal Maya thread pool that respects
   the user-defined thread count setting.

   If using the Intel compiler, there is no need to do anything as
   the threadcount is set globally for the Intel compiler.

   If using OpenMP with gcc on OSX and Linux, or Visual C++ on
   Windows, it is necessary to call the syncNumOpenMPThreads to
   ensure the correct threadcount is set in the plugin.

   <b>Cache line size control</b>

   The cache line size is important information when code may have
   false sharing problems. Use the query method to ensure data that
   will be modified by multiple threads is on separate cache lines.

*/

class OPENMAYA_EXPORT MThreadUtils
{
public:

	static int getNumThreads();
	static void syncNumOpenMPThreads();
	static int getCacheLineSize();

};

/*!
    Maya uses the Intel compiler for OpenMP internally. A plugin may
    be built using VC++ or gcc, both of which have OpenMP thread pools
    that are incompatible with Intel's. For these thread pools to
    respect the Maya thread count, it is important to initialize the
    number of threads explicitly for these implementations of
    OpenMP. This is done by calling this function in the plugin, which
    queries the current the threadcount from Maya and applies it to the
    plugin OpenMP implementation.

	This function does not need to be called if the plugin is compiled
	with the Intel compiler.
*/
inline void MThreadUtils::syncNumOpenMPThreads()	
{
#ifdef _OPENMP
		int numThreads = getNumThreads();
		omp_set_num_threads(numThreads);
#endif
};

#endif /* __cplusplus */
#endif /* _MThreadUtils */
