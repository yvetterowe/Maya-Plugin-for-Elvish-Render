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

#ifndef EI_MATRIX_H
#define EI_MATRIX_H

#include <eiCORE/ei_util.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_vector4.h>
#include <eiCORE/ei_bound.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiMatrix {
	union {
		struct {
			eiScalar	m1, m2, m3, m4;
			eiScalar	m5, m6, m7, m8;
			eiScalar	m9, m10, m11, m12;
			eiScalar	m13, m14, m15, m16;
		};
		eiScalar		comp[16];
		eiScalar		m[4][4];
	};
} eiMatrix;

eiFORCEINLINE void ei_byteswap_matrix(eiMatrix *mat)
{
	ei_byteswap_scalar(&mat->m1);
	ei_byteswap_scalar(&mat->m2);
	ei_byteswap_scalar(&mat->m3);
	ei_byteswap_scalar(&mat->m4);
	ei_byteswap_scalar(&mat->m5);
	ei_byteswap_scalar(&mat->m6);
	ei_byteswap_scalar(&mat->m7);
	ei_byteswap_scalar(&mat->m8);
	ei_byteswap_scalar(&mat->m9);
	ei_byteswap_scalar(&mat->m10);
	ei_byteswap_scalar(&mat->m11);
	ei_byteswap_scalar(&mat->m12);
	ei_byteswap_scalar(&mat->m13);
	ei_byteswap_scalar(&mat->m14);
	ei_byteswap_scalar(&mat->m15);
	ei_byteswap_scalar(&mat->m16);
}

eiFORCEINLINE void initm(eiMatrix *r, const eiScalar x)
{
	r->m1 = x; r->m2 = 0.0f; r->m3 = 0.0f; r->m4 = 0.0f;
	r->m5 = 0.0f; r->m6 = x; r->m7 = 0.0f; r->m8 = 0.0f;
	r->m9 = 0.0f; r->m10 = 0.0f; r->m11 = x; r->m12 = 0.0f;
	r->m13 = 0.0f; r->m14 = 0.0f; r->m15 = 0.0f; r->m16 = x;
}

eiFORCEINLINE void setm(eiMatrix *r, 
						const eiScalar _m1, const eiScalar _m2, const eiScalar _m3, const eiScalar _m4, 
						const eiScalar _m5, const eiScalar _m6, const eiScalar _m7, const eiScalar _m8, 
						const eiScalar _m9, const eiScalar _m10, const eiScalar _m11, const eiScalar _m12, 
						const eiScalar _m13, const eiScalar _m14, const eiScalar _m15, const eiScalar _m16)
{
	r->m1 = _m1; r->m2 = _m2; r->m3 = _m3; r->m4 = _m4;
	r->m5 = _m5; r->m6 = _m6; r->m7 = _m7; r->m8 = _m8;
	r->m9 = _m9; r->m10 = _m10; r->m11 = _m11; r->m12 = _m12;
	r->m13 = _m13; r->m14 = _m14; r->m15 = _m15; r->m16 = _m16;
}

eiFORCEINLINE void movm(eiMatrix *a, const eiMatrix *b)
{
	memcpy(a, b, sizeof(eiMatrix));
}

eiFORCEINLINE eiInt cmpm(const eiMatrix *a, const eiMatrix *b)
{
	return memcmp(a, b, sizeof(eiMatrix));
}

static const eiMatrix g_IdentityMatrix = { 
	1.0f, 0.0f, 0.0f, 0.0f, 
	0.0f, 1.0f, 0.0f, 0.0f, 
	0.0f, 0.0f, 1.0f, 0.0f, 
	0.0f, 0.0f, 0.0f, 1.0f
};

static const eiMatrix g_ZeroMatrix = {
	0.0f, 0.0f, 0.0f, 0.0f, 
	0.0f, 0.0f, 0.0f, 0.0f, 
	0.0f, 0.0f, 0.0f, 0.0f, 
	0.0f, 0.0f, 0.0f, 0.0f, 
};

eiFORCEINLINE void mulmf(eiMatrix *a, const eiScalar s)
{
	a->m1 *= s;
	a->m2 *= s;
	a->m3 *= s;
	a->m4 *= s;
	a->m5 *= s;
	a->m6 *= s;
	a->m7 *= s;
	a->m8 *= s;
	a->m9 *= s;
	a->m10 *= s;
	a->m11 *= s;
	a->m12 *= s;
	a->m13 *= s;
	a->m14 *= s;
	a->m15 *= s;
	a->m16 *= s;
}

eiFORCEINLINE eiScalar det(const eiMatrix *a)
{
	return	a->m[0][3] * a->m[1][2] * a->m[2][1] * a->m[3][0] - a->m[0][2] * a->m[1][3] * a->m[2][1] * a->m[3][0] - a->m[0][3] * a->m[1][1] * a->m[2][2] * a->m[3][0] + a->m[0][1] * a->m[1][3] * a->m[2][2] * a->m[3][0] + 
			a->m[0][2] * a->m[1][1] * a->m[2][3] * a->m[3][0] - a->m[0][1] * a->m[1][2] * a->m[2][3] * a->m[3][0] - a->m[0][3] * a->m[1][2] * a->m[2][0] * a->m[3][1] + a->m[0][2] * a->m[1][3] * a->m[2][0] * a->m[3][1] + 
			a->m[0][3] * a->m[1][0] * a->m[2][2] * a->m[3][1] - a->m[0][0] * a->m[1][3] * a->m[2][2] * a->m[3][1] - a->m[0][2] * a->m[1][0] * a->m[2][3] * a->m[3][1] + a->m[0][0] * a->m[1][2] * a->m[2][3] * a->m[3][1] + 
			a->m[0][3] * a->m[1][1] * a->m[2][0] * a->m[3][2] - a->m[0][1] * a->m[1][3] * a->m[2][0] * a->m[3][2] - a->m[0][3] * a->m[1][0] * a->m[2][1] * a->m[3][2] + a->m[0][0] * a->m[1][3] * a->m[2][1] * a->m[3][2] + 
			a->m[0][1] * a->m[1][0] * a->m[2][3] * a->m[3][2] - a->m[0][0] * a->m[1][1] * a->m[2][3] * a->m[3][2] - a->m[0][2] * a->m[1][1] * a->m[2][0] * a->m[3][3] + a->m[0][1] * a->m[1][2] * a->m[2][0] * a->m[3][3] + 
			a->m[0][2] * a->m[1][0] * a->m[2][1] * a->m[3][3] - a->m[0][0] * a->m[1][2] * a->m[2][1] * a->m[3][3] - a->m[0][1] * a->m[1][0] * a->m[2][2] * a->m[3][3] + a->m[0][0] * a->m[1][1] * a->m[2][2] * a->m[3][3];
}

/****************************************************************
*
* input:
*  mat - pointer to array of 16 floats (source matrix)
* output:
*  dst - pointer to array of 16 floats (invert matrix)
* from:
*  ftp://download.intel.com/design/PentiumIII/sml/24504301.pdf
*
****************************************************************/

eiFORCEINLINE void Invert2(eiScalar *dst, const eiScalar *mat)
{
	eiScalar tmp[12];	/* temp array for pairs */
	eiScalar src[16];	/* array of transpose source matrix */
	eiScalar det;		/* determinant */

#define TRANSPOSE_ROW(i)	\
	src[i]		= mat[i*4];\
	src[i + 4]	= mat[i*4 + 1];\
	src[i + 8]	= mat[i*4 + 2];\
	src[i + 12]	= mat[i*4 + 3];

	/* transpose matrix */
	TRANSPOSE_ROW(0);
	TRANSPOSE_ROW(1);
	TRANSPOSE_ROW(2);
	TRANSPOSE_ROW(3);

	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0]	= src[10]	* src[15];
	tmp[1]	= src[11]	* src[14];
	tmp[2]	= src[9]	* src[15];
	tmp[3]	= src[11]	* src[13];
	tmp[4]	= src[9]	* src[14];
	tmp[5]	= src[10]	* src[13];
	tmp[6]	= src[8]	* src[15];
	tmp[7]	= src[11]	* src[12];
	tmp[8]	= src[8]	* src[14];
	tmp[9]	= src[10]	* src[12];
	tmp[10]	= src[8]	* src[13];
	tmp[11]	= src[9]	* src[12];

	/* calculate first 8 elements (cofactors) */
	dst[0]	 = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0]	-= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1]	 = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1]	-= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2]	 = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2]	-= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3]	 = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3]	-= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4]	 = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4]	-= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5]	 = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5]	-= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6]	 = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6]	-= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7]	 = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7]	-= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];

	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0]	= src[2]*src[7];
	tmp[1]	= src[3]*src[6];
	tmp[2]	= src[1]*src[7];
	tmp[3]	= src[3]*src[5];
	tmp[4]	= src[1]*src[6];
	tmp[5]	= src[2]*src[5];
	tmp[6]	= src[0]*src[7];
	tmp[7]	= src[3]*src[4];
	tmp[8]	= src[0]*src[6];
	tmp[9]	= src[2]*src[4];
	tmp[10]	= src[0]*src[5];
	tmp[11]	= src[1]*src[4];

	/* calculate second 8 elements (cofactors) */
	dst[8]	 = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8]	-= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9]	 = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9]	-= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10]	 = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]	-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11]	 = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]	-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12]	 = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]	-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13]	 = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]	-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14]	 = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]	-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15]	 = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]	-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];

	/* calculate determinant */
	det = src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

	if (almost_zero(det, eiSCALAR_EPS))
	{
		dst[0] = 1.0f; dst[1] = 0.0f; dst[2] = 0.0f; dst[3] = 0.0f;
		dst[4] = 0.0f; dst[5] = 1.0f; dst[6] = 0.0f; dst[7] = 0.0f;
		dst[8] = 0.0f; dst[9] = 0.0f; dst[10] = 1.0f; dst[11] = 0.0f;
		dst[12] = 0.0f; dst[13] = 0.0f; dst[14] = 0.0f; dst[15] = 1.0f;
		return;
	}

	/* calculate matrix inverse */
	det = 1.0f / det;
	dst[0] *= det;
	dst[1] *= det;
	dst[2] *= det;
	dst[3] *= det;
	dst[4] *= det;
	dst[5] *= det;
	dst[6] *= det;
	dst[7] *= det;
	dst[8] *= det;
	dst[9] *= det;
	dst[10] *= det;
	dst[11] *= det;
	dst[12] *= det;
	dst[13] *= det;
	dst[14] *= det;
	dst[15] *= det;
}

eiFORCEINLINE void inv(eiMatrix *r, const eiMatrix *a)
{
	Invert2(r->comp, a->comp);
}

eiFORCEINLINE void invi(eiMatrix *a)
{
	eiMatrix new_a;
	inv(&new_a, a);
	movm(a, &new_a);
}

eiFORCEINLINE void transpose(eiMatrix *r, const eiMatrix *a)
{
	setm(r, 
		a->m[0][0], a->m[1][0], a->m[2][0], a->m[3][0],
		a->m[0][1], a->m[1][1], a->m[2][1], a->m[3][1],
		a->m[0][2], a->m[1][2], a->m[2][2], a->m[3][2],
		a->m[0][3], a->m[1][3], a->m[2][3], a->m[3][3]);
}

eiFORCEINLINE void transposei(eiMatrix *a)
{
	eiMatrix new_a;
	transpose(&new_a, a);
	movm(a, &new_a);
}

eiFORCEINLINE void inv_transpose(eiMatrix *r, const eiMatrix *a)
{
	eiMatrix inv_a;
	inv(&inv_a, a);
	transpose(r, &inv_a);
}

eiFORCEINLINE void inv_transposei(eiMatrix *a)
{
	eiMatrix new_a;
	inv_transpose(&new_a, a);
	movm(a, &new_a);
}

eiFORCEINLINE void mulmm(eiMatrix *rm, const eiMatrix *mx1, const eiMatrix *mx2)
{
	rm->m1 = mx1->m1 * mx2->m1 + mx1->m2 * mx2->m5 + mx1->m3 * mx2->m9 + mx1->m4 * mx2->m13;
    rm->m2 = mx1->m1 * mx2->m2 + mx1->m2 * mx2->m6 + mx1->m3 * mx2->m10 + mx1->m4 * mx2->m14;
    rm->m3 = mx1->m1 * mx2->m3 + mx1->m2 * mx2->m7 + mx1->m3 * mx2->m11 + mx1->m4 * mx2->m15;
    rm->m4 = mx1->m1 * mx2->m4 + mx1->m2 * mx2->m8 + mx1->m3 * mx2->m12 + mx1->m4 * mx2->m16;

    rm->m5 = mx1->m5 * mx2->m1 + mx1->m6 * mx2->m5 + mx1->m7 * mx2->m9 + mx1->m8 * mx2->m13;
    rm->m6 = mx1->m5 * mx2->m2 + mx1->m6 * mx2->m6 + mx1->m7 * mx2->m10 + mx1->m8 * mx2->m14;
    rm->m7 = mx1->m5 * mx2->m3 + mx1->m6 * mx2->m7 + mx1->m7 * mx2->m11 + mx1->m8 * mx2->m15;
    rm->m8 = mx1->m5 * mx2->m4 + mx1->m6 * mx2->m8 + mx1->m7 * mx2->m12 + mx1->m8 * mx2->m16;

    rm->m9 = mx1->m9 * mx2->m1 + mx1->m10 * mx2->m5 + mx1->m11 * mx2->m9 + mx1->m12 * mx2->m13;
    rm->m10 = mx1->m9 * mx2->m2 + mx1->m10 * mx2->m6 + mx1->m11 * mx2->m10 + mx1->m12 * mx2->m14;
    rm->m11 = mx1->m9 * mx2->m3 + mx1->m10 * mx2->m7 + mx1->m11 * mx2->m11 + mx1->m12 * mx2->m15;
    rm->m12 = mx1->m9 * mx2->m4 + mx1->m10 * mx2->m8 + mx1->m11 * mx2->m12 + mx1->m12 * mx2->m16;

    rm->m13 = mx1->m13 * mx2->m1 + mx1->m14 * mx2->m5 + mx1->m15 * mx2->m9 + mx1->m16 * mx2->m13;
    rm->m14 = mx1->m13 * mx2->m2 + mx1->m14 * mx2->m6 + mx1->m15 * mx2->m10 + mx1->m16 * mx2->m14;
    rm->m15 = mx1->m13 * mx2->m3 + mx1->m14 * mx2->m7 + mx1->m15 * mx2->m11 + mx1->m16 * mx2->m15;
    rm->m16 = mx1->m13 * mx2->m4 + mx1->m14 * mx2->m8 + mx1->m15 * mx2->m12 + mx1->m16 * mx2->m16;
}

eiFORCEINLINE void mulmmi(eiMatrix *r, const eiMatrix *a)
{
	eiMatrix c;
	mulmm(&c, r, a);
	movm(r, &c);
}

eiFORCEINLINE void mulvm(eiVector *rv, const eiVector *vec, const eiMatrix *mx)
{
	rv->x = vec->x * mx->m1 + vec->y * mx->m5 + vec->z * mx->m9 + mx->m13;
	rv->y = vec->x * mx->m2 + vec->y * mx->m6 + vec->z * mx->m10 + mx->m14;
	rv->z = vec->x * mx->m3 + vec->y * mx->m7 + vec->z * mx->m11 + mx->m15;
}

eiFORCEINLINE void mulvmi(eiVector *rv, const eiMatrix *mx)
{
	eiVector c;
	mulvm(&c, rv, mx);
	movv(rv, &c);
}

eiFORCEINLINE void mulvm4(eiVector4 *rv, const eiVector4 *vec, const eiMatrix *mx)
{
	rv->x = vec->x * mx->m1 + vec->y * mx->m5 + vec->z * mx->m9  + vec->w * mx->m13;
    rv->y = vec->x * mx->m2 + vec->y * mx->m6 + vec->z * mx->m10 + vec->w * mx->m14;
    rv->z = vec->x * mx->m3 + vec->y * mx->m7 + vec->z * mx->m11 + vec->w * mx->m15;
    rv->w = vec->x * mx->m4 + vec->y * mx->m8 + vec->z * mx->m12 + vec->w * mx->m16;
}

eiFORCEINLINE void mulvm4i(eiVector4 *rv, const eiMatrix *mx)
{
	eiVector4 c;
	mulvm4(&c, rv, mx);
	movv4(rv, &c);
}

eiFORCEINLINE void point_transform(eiVector *rv, const eiVector *vec, const eiMatrix *mx)
{
	setv(rv, 
		vec->x * mx->m1 + vec->y * mx->m5 + vec->z * mx->m9 + mx->m13, 
		vec->x * mx->m2 + vec->y * mx->m6 + vec->z * mx->m10 + mx->m14, 
		vec->x * mx->m3 + vec->y * mx->m7 + vec->z * mx->m11 + mx->m15);
}

eiFORCEINLINE void point_transformi(eiVector *rv, const eiMatrix *mx)
{
	eiVector vec;
	movv(&vec, rv);
	point_transform(rv, &vec, mx);
}

eiFORCEINLINE void vector_transform(eiVector *rv, const eiVector *vec, const eiMatrix *mx)
{
	setv(rv, 
		vec->x * mx->m1 + vec->y * mx->m5 + vec->z * mx->m9, 
		vec->x * mx->m2 + vec->y * mx->m6 + vec->z * mx->m10, 
		vec->x * mx->m3 + vec->y * mx->m7 + vec->z * mx->m11);
}

eiFORCEINLINE void vector_transformi(eiVector *rv, const eiMatrix *mx)
{
	eiVector vec;
	movv(&vec, rv);
	vector_transform(rv, &vec, mx);
}

eiFORCEINLINE void transpose_vector_transform(eiVector *rv, const eiVector *vec, const eiMatrix *mx)
{
	setv(rv, 
		vec->x * mx->m1 + vec->y * mx->m2 + vec->z * mx->m3, 
		vec->x * mx->m5 + vec->y * mx->m6 + vec->z * mx->m7, 
		vec->x * mx->m9 + vec->y * mx->m10 + vec->z * mx->m11);
}

eiFORCEINLINE void transpose_vector_transformi(eiVector *rv, const eiMatrix *mx)
{
	eiVector vec;
	movv(&vec, rv);
	transpose_vector_transform(rv, &vec, mx);
}

eiFORCEINLINE void normal_transform(eiVector *rv, const eiVector *vec, const eiMatrix *mx)
{
	eiMatrix invm;
	inv(&invm, mx);
	setv(rv, 
		vec->x * invm.m[0][0] + vec->y * invm.m[0][1] + vec->z * invm.m[0][2], 
		vec->x * invm.m[1][0] + vec->y * invm.m[1][1] + vec->z * invm.m[1][2], 
		vec->x * invm.m[2][0] + vec->y * invm.m[2][1] + vec->z * invm.m[2][2]);
}

eiFORCEINLINE void normal_transformi(eiVector *rv, const eiMatrix *mx)
{
	eiVector vec;
	movv(&vec, rv);
	normal_transform(rv, &vec, mx);
}

eiFORCEINLINE void transform_box(eiBound *t_box, const eiBound *box, const eiMatrix *transform)
{
	eiVector pos, vx;

	pos.x = box->xmin;
	pos.y = box->ymin;
	pos.z = box->zmin;
	mulvm(&vx, &pos, transform);
	setbv(t_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymin;
	pos.z = box->zmax;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymax;
	pos.z = box->zmin;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymax;
	pos.z = box->zmax;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymin;
	pos.z = box->zmin;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymin;
	pos.z = box->zmax;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymax;
	pos.z = box->zmin;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymax;
	pos.z = box->zmax;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);
}

eiFORCEINLINE void transform_motion_box(
	eiBound *t_box, 
	const eiBound *box, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform)
{
	eiVector pos, vx;

	pos.x = box->xmin;
	pos.y = box->ymin;
	pos.z = box->zmin;
	mulvm(&vx, &pos, transform);
	setbv(t_box, &vx);
	mulvm(&vx, &pos, motion_transform);
	addbv(t_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymin;
	pos.z = box->zmax;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);
	mulvm(&vx, &pos, motion_transform);
	addbv(t_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymax;
	pos.z = box->zmin;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);
	mulvm(&vx, &pos, motion_transform);
	addbv(t_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymax;
	pos.z = box->zmax;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);
	mulvm(&vx, &pos, motion_transform);
	addbv(t_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymin;
	pos.z = box->zmin;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);
	mulvm(&vx, &pos, motion_transform);
	addbv(t_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymin;
	pos.z = box->zmax;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);
	mulvm(&vx, &pos, motion_transform);
	addbv(t_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymax;
	pos.z = box->zmin;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);
	mulvm(&vx, &pos, motion_transform);
	addbv(t_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymax;
	pos.z = box->zmax;
	mulvm(&vx, &pos, transform);
	addbv(t_box, &vx);
	mulvm(&vx, &pos, motion_transform);
	addbv(t_box, &vx);
}

#ifdef __cplusplus
}
#endif

#endif