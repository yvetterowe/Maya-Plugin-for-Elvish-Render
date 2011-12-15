/*
 * Copyright 2010 elvish render Team
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef EI_NOISE_H
#define EI_NOISE_H

/** \brief Noise functions for the shading language interface.
 * \file ei_noise.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_noise_table.h>

#define FADE(t) ( t * t * t * ( t * ( t * 6 - 15 ) + 10 ) )
#define FASTFLOOR(x) ( ((x)>0) ? ((int)x) : ((int)x-1 ) )
#define LERP(t, a, b) ((a) + (t)*((b)-(a)))

template < typename T >
inline T grad( int hash, T x )
{
    int h		= hash & 15;
    T	grad	= (T) 1.0 + (h & 7);
    if (h&8) grad = -grad;
    return ( grad * x );
}

template < typename T >
inline T grad( int hash, T x, T y )
{
    int		h = hash & 7;
    const T	u = h<4 ? x : y;
    const T	v = h<4 ? y : x;
    return ((h&1)? -u : u) + ((h&2)? (T) -2.0*v : (T) 2.0*v);
}

template < typename T >
inline T grad( int hash, T x, T y, T z )
{
    int		h = hash & 15;
    const T	u = h<8 ? x : y;
    const T	v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1)? -u : u) + ((h&2)? -v : v);
}

template < typename T >
inline T grad( int hash, T x, T y, T z, T t )
{
    int		h = hash & 31;
    const T	u = h<24 ? x : y;
    const T	v = h<16 ? y : z;
    const T	w = h<8 ? z : t;
    return ((h&1)? -u : u) + ((h&2)? -v : v) + ((h&4)? -w : w);
}

template < typename T >
inline T noise( T x, const unsigned char *perm )
{
    int ix0, ix1;
    T	fx0, fx1;
    T	s, n0, n1;

    ix0 = FASTFLOOR( x );
    fx0 = x - ix0;
    fx1 = fx0 - 1.0f;
    ix1 = ( ix0+1 ) & 0xff;
    ix0 = ix0 & 0xff;

    s	= FADE( fx0 );

    n0	= grad( perm[ ix0 ], fx0 );
    n1	= grad( perm[ ix1 ], fx1 );
    return 0.5f * (1.0f + 0.188f * ( LERP( s, n0, n1 ) ));
}

template < typename T >
inline T pnoise( T x, int px, const unsigned char *perm )
{
    int ix0, ix1;
    T	fx0, fx1;
    T	s, n0, n1;

	px	=	MAX(px,1);

    ix0 = FASTFLOOR( x );
    fx0 = x - ix0;
    fx1 = fx0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px) & 0xff;
    ix0 = ( ix0 % px ) & 0xff;

    s = FADE( fx0 );

    n0 = grad( perm[ ix0 ], fx0 );
    n1 = grad( perm[ ix1 ], fx1 );
    return 0.5f * (1.0f + 0.188f * ( LERP( s, n0, n1 ) ));
}

template < typename T >
inline T noise( T x, T y, const unsigned char *perm )
{
    int ix0, iy0, ix1, iy1;
    T	fx0, fy0, fx1, fy1;
    T	s, t, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x );
    iy0 = FASTFLOOR( y );
    fx0 = x - ix0;
    fy0 = y - iy0;
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    ix1 = (ix0 + 1) & 0xff;
    iy1 = (iy0 + 1) & 0xff;
    ix0 = ix0 & 0xff;
    iy0 = iy0 & 0xff;
    
    t = FADE( fy0 );
    s = FADE( fx0 );

    nx0 = grad(perm[ix0 + perm[iy0]], fx0, fy0);
    nx1 = grad(perm[ix0 + perm[iy1]], fx0, fy1);
    n0 = LERP( t, nx0, nx1 );

    nx0 = grad(perm[ix1 + perm[iy0]], fx1, fy0);
    nx1 = grad(perm[ix1 + perm[iy1]], fx1, fy1);
    n1 = LERP(t, nx0, nx1);

    return 0.5f * (1.0f + 0.507f * ( LERP( s, n0, n1 ) ) );
}

template < typename T >
inline T pnoise( T x, T y, int px, int py, const unsigned char *perm )
{
    int ix0, iy0, ix1, iy1;
    T	fx0, fy0, fx1, fy1;
    T	s, t, nx0, nx1, n0, n1;

	px	=	MAX(px,1);
	py	=	MAX(py,1);

    ix0 = FASTFLOOR( x );
    iy0 = FASTFLOOR( y );
    fx0 = x - ix0;
    fy0 = y - iy0;
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px) & 0xff;
    iy1 = (( iy0 + 1 ) % py) & 0xff;
    ix0 = ( ix0 % px ) & 0xff;
    iy0 = ( iy0 % py ) & 0xff;
    
    t = FADE( fy0 );
    s = FADE( fx0 );

    nx0 = grad(perm[ix0 + perm[iy0]], fx0, fy0);
    nx1 = grad(perm[ix0 + perm[iy1]], fx0, fy1);
    n0 = LERP( t, nx0, nx1 );

    nx0 = grad(perm[ix1 + perm[iy0]], fx1, fy0);
    nx1 = grad(perm[ix1 + perm[iy1]], fx1, fy1);
    n1 = LERP(t, nx0, nx1);

    return 0.5f * (1.0f + 0.507f * ( LERP( s, n0, n1 ) ) );
}

template < typename T >
inline T noise( T x, T y, T z, const unsigned char *perm )
{
    int ix0, iy0, ix1, iy1, iz0, iz1;
    T	fx0, fy0, fz0, fx1, fy1, fz1;
    T	s, t, r;
    T	nxy0, nxy1, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x );
    iy0 = FASTFLOOR( y );
    iz0 = FASTFLOOR( z );
    fx0 = x - ix0;
    fy0 = y - iy0;
    fz0 = z - iz0;
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    fz1 = fz0 - 1.0f;
    ix1 = ( ix0 + 1 ) & 0xff;
    iy1 = ( iy0 + 1 ) & 0xff;
    iz1 = ( iz0 + 1 ) & 0xff;
    ix0 = ix0 & 0xff;
    iy0 = iy0 & 0xff;
    iz0 = iz0 & 0xff;
    
    r = FADE( fz0 );
    t = FADE( fy0 );
    s = FADE( fx0 );

    nxy0 = grad(perm[ix0 + perm[iy0 + perm[iz0]]], fx0, fy0, fz0);
    nxy1 = grad(perm[ix0 + perm[iy0 + perm[iz1]]], fx0, fy0, fz1);
    nx0 = LERP( r, nxy0, nxy1 );

    nxy0 = grad(perm[ix0 + perm[iy1 + perm[iz0]]], fx0, fy1, fz0);
    nxy1 = grad(perm[ix0 + perm[iy1 + perm[iz1]]], fx0, fy1, fz1);
    nx1 = LERP( r, nxy0, nxy1 );

    n0 = LERP( t, nx0, nx1 );

    nxy0 = grad(perm[ix1 + perm[iy0 + perm[iz0]]], fx1, fy0, fz0);
    nxy1 = grad(perm[ix1 + perm[iy0 + perm[iz1]]], fx1, fy0, fz1);
    nx0 = LERP( r, nxy0, nxy1 );

    nxy0 = grad(perm[ix1 + perm[iy1 + perm[iz0]]], fx1, fy1, fz0);
    nxy1 = grad(perm[ix1 + perm[iy1 + perm[iz1]]], fx1, fy1, fz1);
    nx1 = LERP( r, nxy0, nxy1 );

    n1 = LERP( t, nx0, nx1 );
    
    return 0.5f * (1.0f + 0.936f * ( LERP( s, n0, n1 ) ) );
}

template < typename T >
inline T pnoise( T x, T y, T z, int px, int py, int pz, const unsigned char *perm )
{
    int ix0, iy0, ix1, iy1, iz0, iz1;
    T fx0, fy0, fz0, fx1, fy1, fz1;
    T s, t, r;
    T nxy0, nxy1, nx0, nx1, n0, n1;

	px	=	MAX(px,1);
	py	=	MAX(py,1);
	pz	=	MAX(pz,1);

    ix0 = FASTFLOOR( x );
    iy0 = FASTFLOOR( y );
    iz0 = FASTFLOOR( z );
    fx0 = x - ix0;
    fy0 = y - iy0;
    fz0 = z - iz0;
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    fz1 = fz0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px ) & 0xff;
    iy1 = (( iy0 + 1 ) % py ) & 0xff;
    iz1 = (( iz0 + 1 ) % pz ) & 0xff;
    ix0 = ( ix0 % px ) & 0xff;
    iy0 = ( iy0 % py ) & 0xff;
    iz0 = ( iz0 % pz ) & 0xff;
    
    r = FADE( fz0 );
    t = FADE( fy0 );
    s = FADE( fx0 );

    nxy0 = grad(perm[ix0 + perm[iy0 + perm[iz0]]], fx0, fy0, fz0);
    nxy1 = grad(perm[ix0 + perm[iy0 + perm[iz1]]], fx0, fy0, fz1);
    nx0 = LERP( r, nxy0, nxy1 );

    nxy0 = grad(perm[ix0 + perm[iy1 + perm[iz0]]], fx0, fy1, fz0);
    nxy1 = grad(perm[ix0 + perm[iy1 + perm[iz1]]], fx0, fy1, fz1);
    nx1 = LERP( r, nxy0, nxy1 );

    n0 = LERP( t, nx0, nx1 );

    nxy0 = grad(perm[ix1 + perm[iy0 + perm[iz0]]], fx1, fy0, fz0);
    nxy1 = grad(perm[ix1 + perm[iy0 + perm[iz1]]], fx1, fy0, fz1);
    nx0 = LERP( r, nxy0, nxy1 );

    nxy0 = grad(perm[ix1 + perm[iy1 + perm[iz0]]], fx1, fy1, fz0);
    nxy1 = grad(perm[ix1 + perm[iy1 + perm[iz1]]], fx1, fy1, fz1);
    nx1 = LERP( r, nxy0, nxy1 );

    n1 = LERP( t, nx0, nx1 );
    
    return 0.5f * (1.0f + 0.936f * ( LERP( s, n0, n1 ) ) );
}

template < typename T >
inline T noise( T x, T y, T z, T w, const unsigned char *perm )
{
    int ix0, iy0, iz0, iw0, ix1, iy1, iz1, iw1;
    T fx0, fy0, fz0, fw0, fx1, fy1, fz1, fw1;
    T s, t, r, q;
    T nxyz0, nxyz1, nxy0, nxy1, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x );
    iy0 = FASTFLOOR( y );
    iz0 = FASTFLOOR( z );
    iw0 = FASTFLOOR( w );
    fx0 = x - ix0;
    fy0 = y - iy0;
    fz0 = z - iz0;
    fw0 = w - iw0;
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    fz1 = fz0 - 1.0f;
    fw1 = fw0 - 1.0f;
    ix1 = ( ix0 + 1 ) & 0xff;
    iy1 = ( iy0 + 1 ) & 0xff;
    iz1 = ( iz0 + 1 ) & 0xff;
    iw1 = ( iw0 + 1 ) & 0xff;
    ix0 = ix0 & 0xff;
    iy0 = iy0 & 0xff;
    iz0 = iz0 & 0xff;
    iw0 = iw0 & 0xff;

    q = FADE( fw0 );
    r = FADE( fz0 );
    t = FADE( fy0 );
    s = FADE( fx0 );

    nxyz0 = grad(perm[ix0 + perm[iy0 + perm[iz0 + perm[iw0]]]], fx0, fy0, fz0, fw0);
    nxyz1 = grad(perm[ix0 + perm[iy0 + perm[iz0 + perm[iw1]]]], fx0, fy0, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad(perm[ix0 + perm[iy0 + perm[iz1 + perm[iw0]]]], fx0, fy0, fz1, fw0);
    nxyz1 = grad(perm[ix0 + perm[iy0 + perm[iz1 + perm[iw1]]]], fx0, fy0, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );
        
    nx0 = LERP ( r, nxy0, nxy1 );

    nxyz0 = grad(perm[ix0 + perm[iy1 + perm[iz0 + perm[iw0]]]], fx0, fy1, fz0, fw0);
    nxyz1 = grad(perm[ix0 + perm[iy1 + perm[iz0 + perm[iw1]]]], fx0, fy1, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad(perm[ix0 + perm[iy1 + perm[iz1 + perm[iw0]]]], fx0, fy1, fz1, fw0);
    nxyz1 = grad(perm[ix0 + perm[iy1 + perm[iz1 + perm[iw1]]]], fx0, fy1, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx1 = LERP ( r, nxy0, nxy1 );

    n0 = LERP( t, nx0, nx1 );

    nxyz0 = grad(perm[ix1 + perm[iy0 + perm[iz0 + perm[iw0]]]], fx1, fy0, fz0, fw0);
    nxyz1 = grad(perm[ix1 + perm[iy0 + perm[iz0 + perm[iw1]]]], fx1, fy0, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad(perm[ix1 + perm[iy0 + perm[iz1 + perm[iw0]]]], fx1, fy0, fz1, fw0);
    nxyz1 = grad(perm[ix1 + perm[iy0 + perm[iz1 + perm[iw1]]]], fx1, fy0, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx0 = LERP ( r, nxy0, nxy1 );

    nxyz0 = grad(perm[ix1 + perm[iy1 + perm[iz0 + perm[iw0]]]], fx1, fy1, fz0, fw0);
    nxyz1 = grad(perm[ix1 + perm[iy1 + perm[iz0 + perm[iw1]]]], fx1, fy1, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad(perm[ix1 + perm[iy1 + perm[iz1 + perm[iw0]]]], fx1, fy1, fz1, fw0);
    nxyz1 = grad(perm[ix1 + perm[iy1 + perm[iz1 + perm[iw1]]]], fx1, fy1, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx1 = LERP ( r, nxy0, nxy1 );

    n1 = LERP( t, nx0, nx1 );

    return 0.5f * (1.0f + 0.87f * ( LERP( s, n0, n1 ) ) );
}

template < typename T >
inline T pnoise( T x, T y, T z, float w, int px, int py, int pz, int pw, const unsigned char *perm )
{
    int ix0, iy0, iz0, iw0, ix1, iy1, iz1, iw1;
    T fx0, fy0, fz0, fw0, fx1, fy1, fz1, fw1;
    T s, t, r, q;
    T nxyz0, nxyz1, nxy0, nxy1, nx0, nx1, n0, n1;

	px	=	MAX(px,1);
	py	=	MAX(py,1);
	pz	=	MAX(pz,1);
	pw	=	MAX(pw,1);

    ix0 = FASTFLOOR( x );
    iy0 = FASTFLOOR( y );
    iz0 = FASTFLOOR( z );
    iw0 = FASTFLOOR( w );
    fx0 = x - ix0;
    fy0 = y - iy0;
    fz0 = z - iz0;
    fw0 = w - iw0;
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    fz1 = fz0 - 1.0f;
    fw1 = fw0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px ) & 0xff;
    iy1 = (( iy0 + 1 ) % py ) & 0xff;
    iz1 = (( iz0 + 1 ) % pz ) & 0xff;
    iw1 = (( iw0 + 1 ) % pw ) & 0xff;
    ix0 = ( ix0 % px ) & 0xff;
    iy0 = ( iy0 % py ) & 0xff;
    iz0 = ( iz0 % pz ) & 0xff;
    iw0 = ( iw0 % pw ) & 0xff;

    q = FADE( fw0 );
    r = FADE( fz0 );
    t = FADE( fy0 );
    s = FADE( fx0 );

    nxyz0 = grad(perm[ix0 + perm[iy0 + perm[iz0 + perm[iw0]]]], fx0, fy0, fz0, fw0);
    nxyz1 = grad(perm[ix0 + perm[iy0 + perm[iz0 + perm[iw1]]]], fx0, fy0, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad(perm[ix0 + perm[iy0 + perm[iz1 + perm[iw0]]]], fx0, fy0, fz1, fw0);
    nxyz1 = grad(perm[ix0 + perm[iy0 + perm[iz1 + perm[iw1]]]], fx0, fy0, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );
        
    nx0 = LERP ( r, nxy0, nxy1 );

    nxyz0 = grad(perm[ix0 + perm[iy1 + perm[iz0 + perm[iw0]]]], fx0, fy1, fz0, fw0);
    nxyz1 = grad(perm[ix0 + perm[iy1 + perm[iz0 + perm[iw1]]]], fx0, fy1, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad(perm[ix0 + perm[iy1 + perm[iz1 + perm[iw0]]]], fx0, fy1, fz1, fw0);
    nxyz1 = grad(perm[ix0 + perm[iy1 + perm[iz1 + perm[iw1]]]], fx0, fy1, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx1 = LERP ( r, nxy0, nxy1 );

    n0 = LERP( t, nx0, nx1 );

    nxyz0 = grad(perm[ix1 + perm[iy0 + perm[iz0 + perm[iw0]]]], fx1, fy0, fz0, fw0);
    nxyz1 = grad(perm[ix1 + perm[iy0 + perm[iz0 + perm[iw1]]]], fx1, fy0, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad(perm[ix1 + perm[iy0 + perm[iz1 + perm[iw0]]]], fx1, fy0, fz1, fw0);
    nxyz1 = grad(perm[ix1 + perm[iy0 + perm[iz1 + perm[iw1]]]], fx1, fy0, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx0 = LERP ( r, nxy0, nxy1 );

    nxyz0 = grad(perm[ix1 + perm[iy1 + perm[iz0 + perm[iw0]]]], fx1, fy1, fz0, fw0);
    nxyz1 = grad(perm[ix1 + perm[iy1 + perm[iz0 + perm[iw1]]]], fx1, fy1, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad(perm[ix1 + perm[iy1 + perm[iz1 + perm[iw0]]]], fx1, fy1, fz1, fw0);
    nxyz1 = grad(perm[ix1 + perm[iy1 + perm[iz1 + perm[iw1]]]], fx1, fy1, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx1 = LERP ( r, nxy0, nxy1 );

    n1 = LERP( t, nx0, nx1 );

    return 0.5f * (1.0f + 0.87f * ( LERP( s, n0, n1 ) ) );
}

inline float noise( float arg )
{
	return noise<float>( arg, permX );
}

inline float noise( float uarg, float varg )
{
	return noise<float>( uarg, varg, permX );
}

inline float noise( const point & vec )
{
	return noise<float>( vec.x, vec.y, vec.z, permX );
}

inline float noise( const point & argu, float argv )
{
	return noise<float>( argu.x, argu.y, argu.z, argv, permX );
}

inline point noise_point( float arg )
{
	return point(	noise<float>( arg, permX ),
					noise<float>( arg, permY ),
					noise<float>( arg, permZ ) );
}

inline point noise_point( float uarg, float varg )
{
	return point(	noise<float>( uarg, varg, permX ),
					noise<float>( uarg, varg, permY ),
					noise<float>( uarg, varg, permZ ) );
}

inline point noise_point( const point & vec )
{
	return point(	noise<float>( vec.x, vec.y, vec.z, permX ),
					noise<float>( vec.x, vec.y, vec.z, permY ),
					noise<float>( vec.x, vec.y, vec.z, permZ ) );
}

inline point noise_point( const point & argu, float argv )
{
	return point(	noise<float>( argu.x, argu.y, argu.z, argv, permX ),
					noise<float>( argu.x, argu.y, argu.z, argv, permY ),
					noise<float>( argu.x, argu.y, argu.z, argv, permZ ) );
}

inline vector noise_vector( float arg )
{
	return vector(	noise<float>( arg, permX ),
					noise<float>( arg, permY ),
					noise<float>( arg, permZ ) );
}

inline vector noise_vector( float uarg, float varg )
{
	return vector(	noise<float>( uarg, varg, permX ),
					noise<float>( uarg, varg, permY ),
					noise<float>( uarg, varg, permZ ) );
}

inline vector noise_vector( const point & vec )
{
	return vector(	noise<float>( vec.x, vec.y, vec.z, permX ),
					noise<float>( vec.x, vec.y, vec.z, permY ),
					noise<float>( vec.x, vec.y, vec.z, permZ ) );
}

inline vector noise_vector( const point & argu, float argv )
{
	return vector(	noise<float>( argu.x, argu.y, argu.z, argv, permX ),
					noise<float>( argu.x, argu.y, argu.z, argv, permY ),
					noise<float>( argu.x, argu.y, argu.z, argv, permZ ) );
}

inline color noise_color( float arg )
{
	return color(	noise<float>( arg, permX ),
					noise<float>( arg, permY ),
					noise<float>( arg, permZ ) );
}

inline color noise_color( float uarg, float varg )
{
	return color(	noise<float>( uarg, varg, permX ),
					noise<float>( uarg, varg, permY ),
					noise<float>( uarg, varg, permZ ) );
}

inline color noise_color( const point & vec )
{
	return color(	noise<float>( vec.x, vec.y, vec.z, permX ),
					noise<float>( vec.x, vec.y, vec.z, permY ),
					noise<float>( vec.x, vec.y, vec.z, permZ ) );
}

inline color noise_color( const point & argu, float argv )
{
	return color(	noise<float>( argu.x, argu.y, argu.z, argv, permX ),
					noise<float>( argu.x, argu.y, argu.z, argv, permY ),
					noise<float>( argu.x, argu.y, argu.z, argv, permZ ) );
}

inline float pnoise( float u, float uperiod )
{
	return pnoise<float>( u, (int) uperiod, permX );
}

inline float pnoise( float u, float v, float uperiod, float vperiod )
{
	return pnoise<float>( u, v, (int) uperiod, (int) vperiod, permX );
}

inline float pnoise( const point & arg, const point & periods )
{
	return pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permX );
}

inline float pnoise( const point & arg, float w, const point & periods, float wperiod )
{
	return pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permX );
}

inline point pnoise_point( float u, float uperiod )
{
	return point(	pnoise<float>( u, (int) uperiod, permX ),
					pnoise<float>( u, (int) uperiod, permY ),
					pnoise<float>( u, (int) uperiod, permZ ) );
}

inline point pnoise_point( float u, float v, float uperiod, float vperiod )
{
	return point(	pnoise<float>( u, v, (int) uperiod, (int) vperiod, permX ),
					pnoise<float>( u, v, (int) uperiod, (int) vperiod, permY ),
					pnoise<float>( u, v, (int) uperiod, (int) vperiod, permZ ) );
}

inline point pnoise_point( const point & arg, const point & periods )
{
	return point(	pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permX ),
					pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permY ),
					pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permZ ) );
}

inline point pnoise_point( const point & arg, float w, const point & periods, float wperiod )
{
	return point(	pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permX ),
					pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permY ),
					pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permZ ) );
}

inline vector pnoise_vector( float u, float uperiod )
{
	return vector(	pnoise<float>( u, (int) uperiod, permX ),
					pnoise<float>( u, (int) uperiod, permY ),
					pnoise<float>( u, (int) uperiod, permZ ) );
}

inline vector pnoise_vector( float u, float v, float uperiod, float vperiod )
{
	return vector(	pnoise<float>( u, v, (int) uperiod, (int) vperiod, permX ),
					pnoise<float>( u, v, (int) uperiod, (int) vperiod, permY ),
					pnoise<float>( u, v, (int) uperiod, (int) vperiod, permZ ) );
}

inline vector pnoise_vector( const point & arg, const point & periods )
{
	return vector(	pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permX ),
					pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permY ),
					pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permZ ) );
}

inline vector pnoise_vector( const point & arg, float w, const point & periods, float wperiod )
{
	return vector(	pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permX ),
					pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permY ),
					pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permZ ) );
}

inline color pnoise_color( float u, float uperiod )
{
	return color(	pnoise<float>( u, (int) uperiod, permX ),
					pnoise<float>( u, (int) uperiod, permY ),
					pnoise<float>( u, (int) uperiod, permZ ) );
}

inline color pnoise_color( float u, float v, float uperiod, float vperiod )
{
	return color(	pnoise<float>( u, v, (int) uperiod, (int) vperiod, permX ),
					pnoise<float>( u, v, (int) uperiod, (int) vperiod, permY ),
					pnoise<float>( u, v, (int) uperiod, (int) vperiod, permZ ) );
}

inline color pnoise_color( const point & arg, const point & periods )
{
	return color(	pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permX ),
					pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permY ),
					pnoise<float>( arg.x, arg.y, arg.z, (int) periods.x, (int) periods.y, (int) periods.z, permZ ) );
}

inline color pnoise_color( const point & arg, float w, const point & periods, float wperiod )
{
	return color(	pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permX ),
					pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permY ),
					pnoise<float>( arg.x, arg.y, arg.z, w, (int) periods.x, (int) periods.y, (int) periods.z, (int) wperiod, permZ ) );
}

#define	permute(__val)																		\
							if (__val < 0)	i = permN[ (i + ((uint) (__val - 1))) & 4095 ];	\
							else			i = permN[ (i + ((uint) __val)) & 4095 ];

inline float cellnoise( float arg )
{
	uint	i = 0;

	permute( arg );

	return randN[i];
}

inline float cellnoise( float u, float v )
{
	uint	i = 0;

	permute( u );
	permute( v );

	return randN[i];
}

inline float cellnoise( const point & arg )
{
	uint	i = 0;

	permute( arg.x );
	permute( arg.y );
	permute( arg.z );

	return randN[i];
}

inline float cellnoise( const point & arg, float w )
{
	uint	i = 0;

	permute( arg.x );
	permute( arg.y );
	permute( arg.z );
	permute( w );

	return randN[i];
}

inline point cellnoise_point( float arg )
{
	uint	i = 0;

	permute( arg );

	return point(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline point cellnoise_point( float u, float v )
{
	uint	i = 0;

	permute( u );
	permute( v );

	return point(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline point cellnoise_point( const point & arg )
{
	uint	i = 0;

	permute( arg.x );
	permute( arg.y );
	permute( arg.z );

	return point(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline point cellnoise_point( const point & arg, float w )
{
	uint	i = 0;

	permute( arg.x );
	permute( arg.y );
	permute( arg.y );
	permute( w );

	return point(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline vector cellnoise_vector( float arg )
{
	uint	i = 0;

	permute( arg );

	return vector(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline vector cellnoise_vector( float u, float v )
{
	uint	i = 0;

	permute( u );
	permute( v );

	return vector(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline vector cellnoise_vector( const point & arg )
{
	uint	i = 0;

	permute( arg.x );
	permute( arg.y );
	permute( arg.z );

	return vector(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline vector cellnoise_vector( const point & arg, float w )
{
	uint	i = 0;

	permute( arg.x );
	permute( arg.y );
	permute( arg.y );
	permute( w );

	return vector(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline color cellnoise_color( float arg )
{
	uint	i = 0;

	permute( arg );

	return color(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline color cellnoise_color( float u, float v )
{
	uint	i = 0;

	permute( u );
	permute( v );

	return color(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline color cellnoise_color( const point & arg )
{
	uint	i = 0;

	permute( arg.x );
	permute( arg.y );
	permute( arg.z );

	return color(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

inline color cellnoise_color( const point & arg, float w )
{
	uint	i = 0;

	permute( arg.x );
	permute( arg.y );
	permute( arg.y );
	permute( w );

	return color(	randN[i],
					randN[permN[i]],
					randN[permN[permN[i]]] );
}

#endif
