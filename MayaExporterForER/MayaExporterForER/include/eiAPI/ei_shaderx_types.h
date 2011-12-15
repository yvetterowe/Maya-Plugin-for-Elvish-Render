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
 
#ifndef EI_SHADERX_TYPES_H
#define EI_SHADERX_TYPES_H

/** \brief The basic types for shading language interface.
 * \file ei_shaderx_types.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiCORE/ei_util.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_matrix.h>

#ifndef PI
#define PI ((scalar)eiPI)
#endif

typedef eiUint			uint;
typedef eiScalar		scalar;
typedef eiGeoScalar		geoscalar;

class point;
class vector;
class normal;
class matrix;

class color : public eiVector {
public:
	typedef float	Type;

	inline color()
	{
		r = 0.0f;
		g = 0.0f;
		b = 0.0f;
	}
	inline color( float _r, float _g, float _b )
	{
		r = _r;
		g = _g;
		b = _b;
	}
	inline color( const color & right )
	{
		r = right.r;
		g = right.g;
		b = right.b;
	}
	inline color( const point & right );
	inline color( const vector & right );
	inline color( const normal & right );
	inline color( const float right )
	{
		r = right;
		g = right;
		b = right;
	}
	inline color & operator = ( const color & right )
	{
		r = right.r;
		g = right.g;
		b = right.b;
		return *this;
	}
	inline color & operator = ( const point & right );
	inline color & operator = ( const vector & right );
	inline color & operator = ( const normal & right );
	inline color & operator = ( const float right )
	{
		r = right;
		g = right;
		b = right;
		return *this;
	}
	inline color operator + ( const color & right ) const
	{
		return color( r + right.r,
					   g + right.g,
					   b + right.b );
	}
	inline void operator += ( const color & right )
	{
		r += right.r;
		g += right.g;
		b += right.b;
	}
	inline color operator - ( const color & right ) const
	{
		return color( r - right.r,
					   g - right.g,
					   b - right.b );
	}
	inline void operator -= ( const color & right )
	{
		r -= right.r;
		g -= right.g;
		b -= right.b;
	}
	inline color operator * ( const color & right ) const
	{
		return color( r * right.r,
					   g * right.g,
					   b * right.b );
	}
	inline void operator *= ( const color & right )
	{
		r *= right.r;
		g *= right.g;
		b *= right.b;
	}
	inline color operator / ( const color & right ) const
	{
		return color( r / right.r,
					   g / right.g,
					   b / right.b );
	}
	inline void operator /= ( const color & right )
	{
		r /= right.r;
		g /= right.g;
		b /= right.b;
	}
	inline color operator + ( const float right ) const
	{
		return color( r + right,
					   g + right,
					   b + right );
	}
	inline void operator += ( const float right )
	{
		r += right;
		g += right;
		b += right;
	}
	inline color operator - ( const float right ) const
	{
		return color( r - right,
					   g - right,
					   b - right );
	}
	inline void operator -= ( const float right )
	{
		r -= right;
		g -= right;
		b -= right;
	}
	inline color operator * ( const float right ) const
	{
		return color( r * right,
					   g * right,
					   b * right );
	}
	inline void operator *= ( const float right )
	{
		r *= right;
		g *= right;
		b *= right;
	}
	inline color operator / ( const float right ) const
	{
		return color( r / right,
					   g / right,
					   b / right );
	}
	inline void operator /= ( const float right )
	{
		r /= right;
		g /= right;
		b /= right;
	}
	inline bool operator == ( const color & right ) const
	{
		return ( r == right.r && g == right.g && b == right.b );
	}
	inline bool operator != ( const color & right ) const
	{
		return !( *this == right );
	}
};

inline color operator - ( const color & a )
{
	return color( -a.r, -a.g, -a.b );
}
inline color operator + ( const float left, const color & right )
{
	return color( left + right.r,
				   left + right.g,
				   left + right.b );
}
inline color operator - ( const float left, const color & right )
{
	return color( left - right.r,
				   left - right.g,
				   left - right.b );
}
inline color operator * ( const float left, const color & right )
{
	return color( left * right.r,
				   left * right.g,
				   left * right.b );
}
inline color operator / ( const float left, const color & right )
{
	return color( left / right.r,
				   left / right.g,
				   left / right.b );
}

class point : public eiVector {
public:
	typedef float	Type;

	inline point()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	inline point( float _x, float _y, float _z )
	{
		x = _x;
		y = _y;
		z = _z;
	}
	inline point( const point & right )
	{
		x = right.x;
		y = right.y;
		z = right.z;
	}
	inline point( const color & right )
	{
		x = right.r;
		y = right.g;
		z = right.b;
	}
	inline point( const vector & right );
	inline point( const normal & right );
	inline point( const float right )
	{
		x = right;
		y = right;
		z = right;
	}
	inline point & operator = ( const point & right )
	{
		x = right.x;
		y = right.y;
		z = right.z;
		return *this;
	}
	inline point & operator = ( const vector & right );
	inline point & operator = ( const normal & right );
	inline point & operator = ( const float right )
	{
		x = right;
		y = right;
		z = right;
		return *this;
	}
	inline point & operator = ( const color & right )
	{
		x = right.r;
		y = right.g;
		z = right.b;
		return *this;
	}
	inline point operator + ( const point & right ) const
	{
		return point( x + right.x,
					   y + right.y,
					   z + right.z );
	}
	inline void operator += ( const point & right )
	{
		x += right.x;
		y += right.y;
		z += right.z;
	}
	inline point operator - ( const point & right ) const
	{
		return point( x - right.x,
					   y - right.y,
					   z - right.z );
	}
	inline void operator -= ( const point & right )
	{
		x -= right.x;
		y -= right.y;
		z -= right.z;
	}
	inline point operator * ( const point & right ) const
	{
		return point( x * right.x,
					   y * right.y,
					   z * right.z );
	}
	inline void operator *= ( const point & right )
	{
		x *= right.x;
		y *= right.y;
		z *= right.z;
	}
	inline point operator / ( const point & right ) const
	{
		return point( x / right.x,
					   y / right.y,
					   z / right.z );
	}
	inline void operator /= ( const point & right )
	{
		x /= right.x;
		y /= right.y;
		z /= right.z;
	}
	inline point operator + ( const float right ) const
	{
		return point( x + right,
					   y + right,
					   z + right );
	}
	inline void operator += ( const float right )
	{
		x += right;
		y += right;
		z += right;
	}
	inline point operator - ( const float right ) const
	{
		return point( x - right,
					   y - right,
					   z - right );
	}
	inline void operator -= ( const float right )
	{
		x -= right;
		y -= right;
		z -= right;
	}
	inline point operator * ( const float right ) const
	{
		return point( x * right,
					   y * right,
					   z * right );
	}
	inline void operator *= ( const float right )
	{
		x *= right;
		y *= right;
		z *= right;
	}
	inline point operator / ( const float right ) const
	{
		return point( x / right,
					   y / right,
					   z / right );
	}
	inline void operator /= ( const float right )
	{
		x /= right;
		y /= right;
		z /= right;
	}
	// cross product
	inline point operator ^ ( const point & right ) const
	{
		return point( y * right.z - z * right.y,
					   z * right.x - x * right.z,
					   x * right.y - y * right.x );
	}
	// dot product
	inline float operator % ( const point & right ) const
	{
		return ( x * right.x + y * right.y + z * right.z );
	}
	inline point operator * ( const matrix & right ) const;
	inline bool operator == ( const point & right ) const
	{
		return ( x == right.x && y == right.y && z == right.z );
	}
	inline bool operator != ( const point & right ) const
	{
		return !( *this == right );
	}
};

inline point operator - ( const point & a )
{
	return point( -a.x, -a.y, -a.z );
}
inline point operator + ( const float left, const point & right )
{
	return point( left + right.x,
				   left + right.y,
				   left + right.z );
}
inline point operator - ( const float left, const point & right )
{
	return point( left - right.x,
				   left - right.y,
				   left - right.z );
}
inline point operator * ( const float left, const point & right )
{
	return point( left * right.x,
				   left * right.y,
				   left * right.z );
}
inline point operator / ( const float left, const point & right )
{
	return point( left / right.x,
				   left / right.y,
				   left / right.z );
}

class vector : public eiVector {
public:
	typedef float	Type;

	inline vector()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	inline vector( float _x, float _y, float _z )
	{
		x = _x;
		y = _y;
		z = _z;
	}
	inline vector( const vector & right )
	{
		x = right.x;
		y = right.y;
		z = right.z;
	}
	inline vector( const color & right )
	{
		x = right.r;
		y = right.g;
		z = right.b;
	}
	inline vector( const point & right );
	inline vector( const normal & right );
	inline vector( const float right )
	{
		x = right;
		y = right;
		z = right;
	}
	inline vector & operator = ( const vector & right )
	{
		x = right.x;
		y = right.y;
		z = right.z;
		return *this;
	}
	inline vector & vector::operator = ( const point & right );
	inline vector & vector::operator = ( const normal & right );
	inline vector & operator = ( const float right )
	{
		x = right;
		y = right;
		z = right;
		return *this;
	}
	inline vector & operator = ( const color & right )
	{
		x = right.r;
		y = right.g;
		z = right.b;
		return *this;
	}
	inline vector operator + ( const vector & right ) const
	{
		return vector( x + right.x,
					   y + right.y,
					   z + right.z );
	}
	inline void operator += ( const vector & right )
	{
		x += right.x;
		y += right.y;
		z += right.z;
	}
	inline vector operator - ( const vector & right ) const
	{
		return vector( x - right.x,
					   y - right.y,
					   z - right.z );
	}
	inline void operator -= ( const vector & right )
	{
		x -= right.x;
		y -= right.y;
		z -= right.z;
	}
	inline vector operator * ( const vector & right ) const
	{
		return vector( x * right.x,
					   y * right.y,
					   z * right.z );
	}
	inline void operator *= ( const vector & right )
	{
		x *= right.x;
		y *= right.y;
		z *= right.z;
	}
	inline vector operator / ( const vector & right ) const
	{
		return vector( x / right.x,
					   y / right.y,
					   z / right.z );
	}
	inline void operator /= ( const vector & right )
	{
		x /= right.x;
		y /= right.y;
		z /= right.z;
	}
	inline vector operator + ( const float right ) const
	{
		return vector( x + right,
					   y + right,
					   z + right );
	}
	inline void operator += ( const float right )
	{
		x += right;
		y += right;
		z += right;
	}
	inline vector operator - ( const float right ) const
	{
		return vector( x - right,
					   y - right,
					   z - right );
	}
	inline void operator -= ( const float right )
	{
		x -= right;
		y -= right;
		z -= right;
	}
	inline vector operator * ( const float right ) const
	{
		return vector( x * right,
					   y * right,
					   z * right );
	}
	inline void operator *= ( const float right )
	{
		x *= right;
		y *= right;
		z *= right;
	}
	inline vector operator / ( const float right ) const
	{
		return vector( x / right,
					   y / right,
					   z / right );
	}
	inline void operator /= ( const float right )
	{
		x /= right;
		y /= right;
		z /= right;
	}
	// cross product
	inline vector operator ^ ( const vector & right ) const
	{
		return vector( y * right.z - z * right.y,
					   z * right.x - x * right.z,
					   x * right.y - y * right.x );
	}
	// dot product
	inline float operator % ( const vector & right ) const
	{
		return ( x * right.x + y * right.y + z * right.z );
	}
	inline vector operator * ( const matrix & right ) const;
	inline bool operator == ( const vector & right ) const
	{
		return ( x == right.x && y == right.y && z == right.z );
	}
	inline bool operator != ( const vector & right ) const
	{
		return !( *this == right );
	}
};

inline vector operator - ( const vector & a )
{
	return vector( -a.x, -a.y, -a.z );
}
inline vector operator + ( const float left, const vector & right )
{
	return vector( left + right.x,
				   left + right.y,
				   left + right.z );
}
inline vector operator - ( const float left, const vector & right )
{
	return vector( left - right.x,
				   left - right.y,
				   left - right.z );
}
inline vector operator * ( const float left, const vector & right )
{
	return vector( left * right.x,
				   left * right.y,
				   left * right.z );
}
inline vector operator / ( const float left, const vector & right )
{
	return vector( left / right.x,
				   left / right.y,
				   left / right.z );
}

class normal : public eiVector {
public:
	typedef float	Type;

	inline normal()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	inline normal( float _x, float _y, float _z )
	{
		x = _x;
		y = _y;
		z = _z;
	}
	inline normal( const normal & right )
	{
		x = right.x;
		y = right.y;
		z = right.z;
	}
	inline normal( const color & right )
	{
		x = right.r;
		y = right.g;
		z = right.b;
	}
	inline normal( const point & right );
	inline normal( const vector & right );
	inline normal( const float right )
	{
		x = right;
		y = right;
		z = right;
	}
	inline normal & operator = ( const normal & right )
	{
		x = right.x;
		y = right.y;
		z = right.z;
		return *this;
	}
	inline normal & operator = ( const point & right );
	inline normal & operator = ( const vector & right );
	inline normal & operator = ( const float right )
	{
		x = right;
		y = right;
		z = right;
		return *this;
	}
	inline normal & operator = ( const color & right )
	{
		x = right.r;
		y = right.g;
		z = right.b;
		return *this;
	}
	inline normal operator + ( const normal & right ) const
	{
		return normal( x + right.x,
					   y + right.y,
					   z + right.z );
	}
	inline void operator += ( const normal & right )
	{
		x += right.x;
		y += right.y;
		z += right.z;
	}
	inline normal operator - ( const normal & right ) const
	{
		return normal( x - right.x,
					   y - right.y,
					   z - right.z );
	}
	inline void operator -= ( const normal & right )
	{
		x -= right.x;
		y -= right.y;
		z -= right.z;
	}
	inline normal operator * ( const normal & right ) const
	{
		return normal( x * right.x,
					   y * right.y,
					   z * right.z );
	}
	inline void operator *= ( const normal & right )
	{
		x *= right.x;
		y *= right.y;
		z *= right.z;
	}
	inline normal operator / ( const normal & right ) const
	{
		return normal( x / right.x,
					   y / right.y,
					   z / right.z );
	}
	inline void operator /= ( const normal & right )
	{
		x /= right.x;
		y /= right.y;
		z /= right.z;
	}
	inline normal operator + ( const float right ) const
	{
		return normal( x + right,
					   y + right,
					   z + right );
	}
	inline void operator += ( const float right )
	{
		x += right;
		y += right;
		z += right;
	}
	inline normal operator - ( const float right ) const
	{
		return normal( x - right,
					   y - right,
					   z - right );
	}
	inline void operator -= ( const float right )
	{
		x -= right;
		y -= right;
		z -= right;
	}
	inline normal operator * ( const float right ) const
	{
		return normal( x * right,
					   y * right,
					   z * right );
	}
	inline void operator *= ( const float right )
	{
		x *= right;
		y *= right;
		z *= right;
	}
	inline normal operator / ( const float right ) const
	{
		return normal( x / right,
					   y / right,
					   z / right );
	}
	inline void operator /= ( const float right )
	{
		x /= right;
		y /= right;
		z /= right;
	}
	// cross product
	inline normal operator ^ ( const normal & right ) const
	{
		return normal( y * right.z - z * right.y,
					   z * right.x - x * right.z,
					   x * right.y - y * right.x );
	}
	// dot product
	inline float operator % ( const normal & right ) const
	{
		return ( x * right.x + y * right.y + z * right.z );
	}
	inline normal operator * ( const matrix & right ) const;
	inline bool operator == ( const normal & right ) const
	{
		return ( x == right.x && y == right.y && z == right.z );
	}
	inline bool operator != ( const normal & right ) const
	{
		return !( *this == right );
	}
};

inline normal operator - ( const normal & a )
{
	return normal( -a.x, -a.y, -a.z );
}
inline normal operator + ( const float left, const normal & right )
{
	return normal( left + right.x,
				   left + right.y,
				   left + right.z );
}
inline normal operator - ( const float left, const normal & right )
{
	return normal( left - right.x,
				   left - right.y,
				   left - right.z );
}
inline normal operator * ( const float left, const normal & right )
{
	return normal( left * right.x,
				   left * right.y,
				   left * right.z );
}
inline normal operator / ( const float left, const normal & right )
{
	return normal( left / right.x,
				   left / right.y,
				   left / right.z );
}

inline color::color( const point & right )
{
	r = right.x;
	g = right.y;
	b = right.z;
}

inline color::color( const vector & right )
{
	r = right.x;
	g = right.y;
	b = right.z;
}

inline color::color( const normal & right )
{
	r = right.x;
	g = right.y;
	b = right.z;
}

inline color & color::operator = ( const point & right )
{
	r = right.x;
	g = right.y;
	b = right.z;
	return *this;
}

inline color & color::operator = ( const vector & right )
{
	r = right.x;
	g = right.y;
	b = right.z;
	return *this;
}

inline color & color::operator = ( const normal & right )
{
	r = right.x;
	g = right.y;
	b = right.z;
	return *this;
}

inline point::point( const vector & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
}
inline point::point( const normal & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
}
inline vector::vector( const point & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
}
inline vector::vector( const normal & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
}
inline normal::normal( const point & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
}
inline normal::normal( const vector & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
}

inline point & point::operator = ( const vector & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
	return *this;
}
inline point & point::operator = ( const normal & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
	return *this;
}
inline vector & vector::operator = ( const point & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
	return *this;
}
inline vector & vector::operator = ( const normal & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
	return *this;
}
inline normal & normal::operator = ( const point & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
	return *this;
}
inline normal & normal::operator = ( const vector & right )
{
	x = right.x;
	y = right.y;
	z = right.z;
	return *this;
}

class matrix : public eiMatrix {
public:
	typedef float	Type;

	inline matrix()
	{
		m[0][0] = 1.0f;
		m[0][1] = 0.0f;
		m[0][2] = 0.0f;
		m[0][3] = 0.0f;
		m[1][0] = 0.0f;
		m[1][1] = 1.0f;
		m[1][2] = 0.0f;
		m[1][3] = 0.0f;
		m[2][0] = 0.0f;
		m[2][1] = 0.0f;
		m[2][2] = 1.0f;
		m[2][3] = 0.0f;
		m[3][0] = 0.0f;
		m[3][1] = 0.0f;
		m[3][2] = 0.0f;
		m[3][3] = 1.0f;
	}
	inline matrix( const float m00, const float m01, const float m02, const float m03,
					const float m10, const float m11, const float m12, const float m13,
					const float m20, const float m21, const float m22, const float m23,
					const float m30, const float m31, const float m32, const float m33 )
	{
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;
		m[0][3] = m03;
		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;
		m[1][3] = m13;
		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
		m[2][3] = m23;
		m[3][0] = m30;
		m[3][1] = m31;
		m[3][2] = m32;
		m[3][3] = m33;
	}
	inline matrix( const matrix & right )
	{
		m[0][0] = right.m[0][0];
		m[0][1] = right.m[0][1];
		m[0][2] = right.m[0][2];
		m[0][3] = right.m[0][3];
		m[1][0] = right.m[1][0];
		m[1][1] = right.m[1][1];
		m[1][2] = right.m[1][2];
		m[1][3] = right.m[1][3];
		m[2][0] = right.m[2][0];
		m[2][1] = right.m[2][1];
		m[2][2] = right.m[2][2];
		m[2][3] = right.m[2][3];
		m[3][0] = right.m[3][0];
		m[3][1] = right.m[3][1];
		m[3][2] = right.m[3][2];
		m[3][3] = right.m[3][3];
	}
	inline matrix( const float right )
	{
		m[0][0] = right;
		m[0][1] = 0.0f;
		m[0][2] = 0.0f;
		m[0][3] = 0.0f;
		m[1][0] = 0.0f;
		m[1][1] = right;
		m[1][2] = 0.0f;
		m[1][3] = 0.0f;
		m[2][0] = 0.0f;
		m[2][1] = 0.0f;
		m[2][2] = right;
		m[2][3] = 0.0f;
		m[3][0] = 0.0f;
		m[3][1] = 0.0f;
		m[3][2] = 0.0f;
		m[3][3] = right;
	}
	inline matrix & operator = ( const matrix & right )
	{
		m[0][0] = right.m[0][0];
		m[0][1] = right.m[0][1];
		m[0][2] = right.m[0][2];
		m[0][3] = right.m[0][3];
		m[1][0] = right.m[1][0];
		m[1][1] = right.m[1][1];
		m[1][2] = right.m[1][2];
		m[1][3] = right.m[1][3];
		m[2][0] = right.m[2][0];
		m[2][1] = right.m[2][1];
		m[2][2] = right.m[2][2];
		m[2][3] = right.m[2][3];
		m[3][0] = right.m[3][0];
		m[3][1] = right.m[3][1];
		m[3][2] = right.m[3][2];
		m[3][3] = right.m[3][3];
		return *this;
	}
	inline matrix & operator = ( const float right )
	{
		m[0][0] = right;
		m[0][1] = 0.0f;
		m[0][2] = 0.0f;
		m[0][3] = 0.0f;
		m[1][0] = 0.0f;
		m[1][1] = right;
		m[1][2] = 0.0f;
		m[1][3] = 0.0f;
		m[2][0] = 0.0f;
		m[2][1] = 0.0f;
		m[2][2] = right;
		m[2][3] = 0.0f;
		m[3][0] = 0.0f;
		m[3][1] = 0.0f;
		m[3][2] = 0.0f;
		m[3][3] = right;
		return *this;
	}
	inline matrix operator + ( const matrix & right ) const
	{
		return matrix(  m[0][0] + right.m[0][0],
						m[0][1] + right.m[0][1],
						m[0][2] + right.m[0][2],
						m[0][3] + right.m[0][3],
						m[1][0] + right.m[1][0],
						m[1][1] + right.m[1][1],
						m[1][2] + right.m[1][2],
						m[1][3] + right.m[1][3],
						m[2][0] + right.m[2][0],
						m[2][1] + right.m[2][1],
						m[2][2] + right.m[2][2],
						m[2][3] + right.m[2][3],
						m[3][0] + right.m[3][0],
						m[3][1] + right.m[3][1],
						m[3][2] + right.m[3][2],
						m[3][3] + right.m[3][3] );
	}
	inline void operator += ( const matrix & right )
	{
		m[0][0] += right.m[0][0];
		m[0][1] += right.m[0][1];
		m[0][2] += right.m[0][2];
		m[0][3] += right.m[0][3];
		m[1][0] += right.m[1][0];
		m[1][1] += right.m[1][1];
		m[1][2] += right.m[1][2];
		m[1][3] += right.m[1][3];
		m[2][0] += right.m[2][0];
		m[2][1] += right.m[2][1];
		m[2][2] += right.m[2][2];
		m[2][3] += right.m[2][3];
		m[3][0] += right.m[3][0];
		m[3][1] += right.m[3][1];
		m[3][2] += right.m[3][2];
		m[3][3] += right.m[3][3];
	}
	inline matrix operator - ( const matrix & right ) const
	{
		return matrix(  m[0][0] - right.m[0][0],
						m[0][1] - right.m[0][1],
						m[0][2] - right.m[0][2],
						m[0][3] - right.m[0][3],
						m[1][0] - right.m[1][0],
						m[1][1] - right.m[1][1],
						m[1][2] - right.m[1][2],
						m[1][3] - right.m[1][3],
						m[2][0] - right.m[2][0],
						m[2][1] - right.m[2][1],
						m[2][2] - right.m[2][2],
						m[2][3] - right.m[2][3],
						m[3][0] - right.m[3][0],
						m[3][1] - right.m[3][1],
						m[3][2] - right.m[3][2],
						m[3][3] - right.m[3][3] );
	}
	inline void operator -= ( const matrix & right )
	{
		m[0][0] -= right.m[0][0];
		m[0][1] -= right.m[0][1];
		m[0][2] -= right.m[0][2];
		m[0][3] -= right.m[0][3];
		m[1][0] -= right.m[1][0];
		m[1][1] -= right.m[1][1];
		m[1][2] -= right.m[1][2];
		m[1][3] -= right.m[1][3];
		m[2][0] -= right.m[2][0];
		m[2][1] -= right.m[2][1];
		m[2][2] -= right.m[2][2];
		m[2][3] -= right.m[2][3];
		m[3][0] -= right.m[3][0];
		m[3][1] -= right.m[3][1];
		m[3][2] -= right.m[3][2];
		m[3][3] -= right.m[3][3];
	}
	inline matrix operator * ( const matrix & right ) const
	{
		return matrix(  m[0][0] * right.m[0][0] + m[0][1] * right.m[1][0] + m[0][2] * right.m[2][0] + m[0][3] * right.m[3][0],
						m[0][0] * right.m[0][1] + m[0][1] * right.m[1][1] + m[0][2] * right.m[2][1] + m[0][3] * right.m[3][1],
						m[0][0] * right.m[0][2] + m[0][1] * right.m[1][2] + m[0][2] * right.m[2][2] + m[0][3] * right.m[3][2],
						m[0][0] * right.m[0][3] + m[0][1] * right.m[1][3] + m[0][2] * right.m[2][3] + m[0][3] * right.m[3][3],
						
						m[1][0] * right.m[0][0] + m[1][1] * right.m[1][0] + m[1][2] * right.m[2][0] + m[1][3] * right.m[3][0],
						m[1][0] * right.m[0][1] + m[1][1] * right.m[1][1] + m[1][2] * right.m[2][1] + m[1][3] * right.m[3][1],
						m[1][0] * right.m[0][2] + m[1][1] * right.m[1][2] + m[1][2] * right.m[2][2] + m[1][3] * right.m[3][2],
						m[1][0] * right.m[0][3] + m[1][1] * right.m[1][3] + m[1][2] * right.m[2][3] + m[1][3] * right.m[3][3],

						m[2][0] * right.m[0][0] + m[2][1] * right.m[1][0] + m[2][2] * right.m[2][0] + m[2][3] * right.m[3][0],
						m[2][0] * right.m[0][1] + m[2][1] * right.m[1][1] + m[2][2] * right.m[2][1] + m[2][3] * right.m[3][1],
						m[2][0] * right.m[0][2] + m[2][1] * right.m[1][2] + m[2][2] * right.m[2][2] + m[2][3] * right.m[3][2],
						m[2][0] * right.m[0][3] + m[2][1] * right.m[1][3] + m[2][2] * right.m[2][3] + m[2][3] * right.m[3][3],

						m[3][0] * right.m[0][0] + m[3][1] * right.m[1][0] + m[3][2] * right.m[2][0] + m[3][3] * right.m[3][0],
						m[3][0] * right.m[0][1] + m[3][1] * right.m[1][1] + m[3][2] * right.m[2][1] + m[3][3] * right.m[3][1],
						m[3][0] * right.m[0][2] + m[3][1] * right.m[1][2] + m[3][2] * right.m[2][2] + m[3][3] * right.m[3][2],
						m[3][0] * right.m[0][3] + m[3][1] * right.m[1][3] + m[3][2] * right.m[2][3] + m[3][3] * right.m[3][3] );
	}
	inline void operator *= ( const matrix & right )
	{
		matrix	mx(		m[0][0] * right.m[0][0] + m[0][1] * right.m[1][0] + m[0][2] * right.m[2][0] + m[0][3] * right.m[3][0],
						m[0][0] * right.m[0][1] + m[0][1] * right.m[1][1] + m[0][2] * right.m[2][1] + m[0][3] * right.m[3][1],
						m[0][0] * right.m[0][2] + m[0][1] * right.m[1][2] + m[0][2] * right.m[2][2] + m[0][3] * right.m[3][2],
						m[0][0] * right.m[0][3] + m[0][1] * right.m[1][3] + m[0][2] * right.m[2][3] + m[0][3] * right.m[3][3],
						
						m[1][0] * right.m[0][0] + m[1][1] * right.m[1][0] + m[1][2] * right.m[2][0] + m[1][3] * right.m[3][0],
						m[1][0] * right.m[0][1] + m[1][1] * right.m[1][1] + m[1][2] * right.m[2][1] + m[1][3] * right.m[3][1],
						m[1][0] * right.m[0][2] + m[1][1] * right.m[1][2] + m[1][2] * right.m[2][2] + m[1][3] * right.m[3][2],
						m[1][0] * right.m[0][3] + m[1][1] * right.m[1][3] + m[1][2] * right.m[2][3] + m[1][3] * right.m[3][3],

						m[2][0] * right.m[0][0] + m[2][1] * right.m[1][0] + m[2][2] * right.m[2][0] + m[2][3] * right.m[3][0],
						m[2][0] * right.m[0][1] + m[2][1] * right.m[1][1] + m[2][2] * right.m[2][1] + m[2][3] * right.m[3][1],
						m[2][0] * right.m[0][2] + m[2][1] * right.m[1][2] + m[2][2] * right.m[2][2] + m[2][3] * right.m[3][2],
						m[2][0] * right.m[0][3] + m[2][1] * right.m[1][3] + m[2][2] * right.m[2][3] + m[2][3] * right.m[3][3],

						m[3][0] * right.m[0][0] + m[3][1] * right.m[1][0] + m[3][2] * right.m[2][0] + m[3][3] * right.m[3][0],
						m[3][0] * right.m[0][1] + m[3][1] * right.m[1][1] + m[3][2] * right.m[2][1] + m[3][3] * right.m[3][1],
						m[3][0] * right.m[0][2] + m[3][1] * right.m[1][2] + m[3][2] * right.m[2][2] + m[3][3] * right.m[3][2],
						m[3][0] * right.m[0][3] + m[3][1] * right.m[1][3] + m[3][2] * right.m[2][3] + m[3][3] * right.m[3][3] );

		*this = mx;
	}
	inline matrix operator * ( const float right ) const
	{
		return matrix( m[0][0] * right,
					   m[0][1] * right,
					   m[0][2] * right,
					   m[0][3] * right,
					   m[1][0] * right,
					   m[1][1] * right,
					   m[1][2] * right,
					   m[1][3] * right,
					   m[2][0] * right,
					   m[2][1] * right,
					   m[2][2] * right,
					   m[2][3] * right,
					   m[3][0] * right,
					   m[3][1] * right,
					   m[3][2] * right,
					   m[3][3] * right );
	}
	inline void operator *= ( const float right )
	{
		m[0][0] *= right;
		m[0][1] *= right;
		m[0][2] *= right;
		m[0][3] *= right;
		m[1][0] *= right;
		m[1][1] *= right;
		m[1][2] *= right;
		m[1][3] *= right;
		m[2][0] *= right;
		m[2][1] *= right;
		m[2][2] *= right;
		m[2][3] *= right;
		m[3][0] *= right;
		m[3][1] *= right;
		m[3][2] *= right;
		m[3][3] *= right;
	}
	inline matrix operator / ( const float right ) const
	{
		return matrix( m[0][0] / right,
					   m[0][1] / right,
					   m[0][2] / right,
					   m[0][3] / right,
					   m[1][0] / right,
					   m[1][1] / right,
					   m[1][2] / right,
					   m[1][3] / right,
					   m[2][0] / right,
					   m[2][1] / right,
					   m[2][2] / right,
					   m[2][3] / right,
					   m[3][0] / right,
					   m[3][1] / right,
					   m[3][2] / right,
					   m[3][3] / right );
	}
	inline void operator /= ( const float right )
	{
		m[0][0] /= right;
		m[0][1] /= right;
		m[0][2] /= right;
		m[0][3] /= right;
		m[1][0] /= right;
		m[1][1] /= right;
		m[1][2] /= right;
		m[1][3] /= right;
		m[2][0] /= right;
		m[2][1] /= right;
		m[2][2] /= right;
		m[2][3] /= right;
		m[3][0] /= right;
		m[3][1] /= right;
		m[3][2] /= right;
		m[3][3] /= right;
	}
	inline bool operator == ( const matrix & right ) const
	{
		return ( m[0][0] == right.m[0][0] && 
				 m[0][1] == right.m[0][1] && 
				 m[0][2] == right.m[0][2] && 
				 m[0][3] == right.m[0][3] && 
				 m[1][0] == right.m[1][0] && 
				 m[1][1] == right.m[1][1] && 
				 m[1][2] == right.m[1][2] && 
				 m[1][3] == right.m[1][3] && 
				 m[2][0] == right.m[2][0] && 
				 m[2][1] == right.m[2][1] && 
				 m[2][2] == right.m[2][2] && 
				 m[2][3] == right.m[2][3] && 
				 m[3][0] == right.m[3][0] && 
				 m[3][1] == right.m[3][1] && 
				 m[3][2] == right.m[3][2] && 
				 m[3][3] == right.m[3][3] );
	}
	inline bool operator != ( const matrix & right ) const
	{
		return !( *this == right );
	}
};

inline matrix operator - ( const matrix & a )
{
	return matrix(  -a.m[0][0], -a.m[0][1], -a.m[0][2], -a.m[0][3], 
					-a.m[1][0], -a.m[1][1], -a.m[1][2], -a.m[1][3], 
					-a.m[2][0], -a.m[2][1], -a.m[2][2], -a.m[2][3], 
					-a.m[3][0], -a.m[3][1], -a.m[3][2], -a.m[3][3] );
}

inline matrix operator * ( const float left, const matrix & right )
{
	return matrix( left * right.m[0][0],
				   left * right.m[0][1],
				   left * right.m[0][2],
				   left * right.m[0][3],
				   left * right.m[1][0],
				   left * right.m[1][1],
				   left * right.m[1][2],
				   left * right.m[1][3],
				   left * right.m[2][0],
				   left * right.m[2][1],
				   left * right.m[2][2],
				   left * right.m[2][3],
				   left * right.m[3][0],
				   left * right.m[3][1],
				   left * right.m[3][2],
				   left * right.m[3][3] );
}

inline float determinant( const matrix & a )
{
	return	a.m[0][3] * a.m[1][2] * a.m[2][1] * a.m[3][0] - a.m[0][2] * a.m[1][3] * a.m[2][1] * a.m[3][0] - a.m[0][3] * a.m[1][1] * a.m[2][2] * a.m[3][0] + a.m[0][1] * a.m[1][3] * a.m[2][2] * a.m[3][0] +
			a.m[0][2] * a.m[1][1] * a.m[2][3] * a.m[3][0] - a.m[0][1] * a.m[1][2] * a.m[2][3] * a.m[3][0] - a.m[0][3] * a.m[1][2] * a.m[2][0] * a.m[3][1] + a.m[0][2] * a.m[1][3] * a.m[2][0] * a.m[3][1] +
			a.m[0][3] * a.m[1][0] * a.m[2][2] * a.m[3][1] - a.m[0][0] * a.m[1][3] * a.m[2][2] * a.m[3][1] - a.m[0][2] * a.m[1][0] * a.m[2][3] * a.m[3][1] + a.m[0][0] * a.m[1][2] * a.m[2][3] * a.m[3][1] +
			a.m[0][3] * a.m[1][1] * a.m[2][0] * a.m[3][2] - a.m[0][1] * a.m[1][3] * a.m[2][0] * a.m[3][2] - a.m[0][3] * a.m[1][0] * a.m[2][1] * a.m[3][2] + a.m[0][0] * a.m[1][3] * a.m[2][1] * a.m[3][2] +
			a.m[0][1] * a.m[1][0] * a.m[2][3] * a.m[3][2] - a.m[0][0] * a.m[1][1] * a.m[2][3] * a.m[3][2] - a.m[0][2] * a.m[1][1] * a.m[2][0] * a.m[3][3] + a.m[0][1] * a.m[1][2] * a.m[2][0] * a.m[3][3] +
			a.m[0][2] * a.m[1][0] * a.m[2][1] * a.m[3][3] - a.m[0][0] * a.m[1][2] * a.m[2][1] * a.m[3][3] - a.m[0][1] * a.m[1][0] * a.m[2][2] * a.m[3][3] + a.m[0][0] * a.m[1][1] * a.m[2][2] * a.m[3][3];
}

inline matrix inverse( const matrix & a )
{
	matrix	mx;
	inv(&mx, &a);
	return mx;
}

inline matrix transpose( const matrix & a )
{
	return matrix(	a.m[0][0], a.m[1][0], a.m[2][0], a.m[3][0],
					a.m[0][1], a.m[1][1], a.m[2][1], a.m[3][1],
					a.m[0][2], a.m[1][2], a.m[2][2], a.m[3][2],
					a.m[0][3], a.m[1][3], a.m[2][3], a.m[3][3] );
}

inline matrix translate( float tx, float ty, float tz )
{
	return matrix(	1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					tx, ty, tz, 1.0f );
}

inline matrix scale( float sx, float sy, float sz )
{
	return matrix(	sx, 0.0f, 0.0f, 0.0f,
					0.0f, sy, 0.0f, 0.0f,
					0.0f, 0.0f, sz, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f );
}

inline matrix rotate_x( float angle )
{
	float	sine = sin( angle );
	float	cosine = cos( angle );

	return matrix(	1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, cosine, sine, 0.0f,
					0.0f, -sine, cosine, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f );
}

inline matrix rotate_y( float angle )
{
	float	sine = sin( angle );
	float	cosine = cos( angle );

	return matrix(	cosine, 0.0f, -sine, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					sine, 0.0f, cosine, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f );
}

inline matrix rotate_z( float angle )
{
	float	sine = sin( angle );
	float	cosine = cos( angle );

	return matrix(	cosine, sine, 0.0f, 0.0f,
					-sine, cosine, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f );
}

inline matrix rotate( float angle, const vector & axis )
{
	float	sn	= sin( angle );
	float	cs	= cos( angle );
	float	one_minus_cs = 1.0f - cs;

	return matrix(	axis.x * axis.x + (1.0f - axis.x * axis.x) * cs,
					axis.x * axis.y * one_minus_cs + axis.z * sn,
					axis.x * axis.z * one_minus_cs - axis.y * sn,
					0.0f,
					axis.x * axis.y * one_minus_cs - axis.z * sn,
					axis.y * axis.y + (1.0f - axis.y * axis.y) * cs,
					axis.y * axis.z * one_minus_cs + axis.x * sn,
					0.0f,
					axis.x * axis.z * one_minus_cs + axis.y * sn,
					axis.y * axis.z * one_minus_cs - axis.x * sn,
					axis.z * axis.z + (1.0f - axis.z * axis.z) * cs,
					0.0f,
					0.0f, 0.0f, 0.0f, 1.0f );
}

inline point point::operator * ( const matrix & right ) const
{
	return point(	x * right.m[0][0] + y * right.m[1][0] + z * right.m[2][0] + right.m[3][0],
					x * right.m[0][1] + y * right.m[1][1] + z * right.m[2][1] + right.m[3][1],
					x * right.m[0][2] + y * right.m[1][2] + z * right.m[2][2] + right.m[3][2] );
}

inline vector vector::operator * ( const matrix & right ) const
{
	return vector(	x * right.m[0][0] + y * right.m[1][0] + z * right.m[2][0],
					x * right.m[0][1] + y * right.m[1][1] + z * right.m[2][1],
					x * right.m[0][2] + y * right.m[1][2] + z * right.m[2][2] );
}

inline normal normal::operator * ( const matrix & right ) const
{
	matrix	invm( inverse( right ) );

	return normal(	x * invm.m[0][0] + y * invm.m[0][1] + z * invm.m[0][2],
					x * invm.m[1][0] + y * invm.m[1][1] + z * invm.m[1][2],
					x * invm.m[2][0] + y * invm.m[2][1] + z * invm.m[2][2] );
}

#endif
