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

#include <cxxtest/TestSuite.h>

#include <eiAPI/ei_poly_object.h>

class ei_approx_stitch_unittest : public CxxTest::TestSuite
{
public:
	void setUp();
	void tearDown();

	void testStitch();
	void testConnect();
};

void ei_approx_stitch_unittest::setUp()
{
}

void ei_approx_stitch_unittest::tearDown()
{
}

#define ADD_EDGE_VERTEX(x, y) \
	setv(&vtx.pos, (x), (y), 0.0f);\
	movv(&vtx.m_pos, &vtx.pos);\
	ei_array_push_back(&vertices, &vtx);

#define TEST_FACE(i, i1, i2, i3) \
	tri = (eiRayTriangle *)ei_array_get(&triangles, (i));\
	TS_ASSERT(tri->v1 == (i1));\
	TS_ASSERT(tri->v2 == (i2));\
	TS_ASSERT(tri->v3 == (i3));

void ei_approx_stitch_unittest::testStitch()
{
	eiPolyTessel	poly;
	const eiInt		e1_size = 9;
	const eiInt		e2_size = 9;
	eiIndex			e1[e1_size];
	eiIndex			e2[e2_size];
	ei_array		vertices;
	ei_array		triangles;
	eiRayVertex		vtx;
	eiRayTriangle	*tri;
	eiInt			i;

	memset(&poly, 0, sizeof(eiPolyTessel));

	ei_array_init(&vertices, sizeof(eiRayVertex));
	ei_array_init(&triangles, sizeof(eiRayTriangle));

	ADD_EDGE_VERTEX(-112.456f, -29.848f);
	ADD_EDGE_VERTEX(-81.573f, -34.668f);
	ADD_EDGE_VERTEX(-52.671f, -33.415f);
	ADD_EDGE_VERTEX(-24.773f, -28.504f);
	ADD_EDGE_VERTEX(3.897f, -15.112f);
	ADD_EDGE_VERTEX(27.758f, -39.045f);
	ADD_EDGE_VERTEX(58.589f, -29.93f);
	ADD_EDGE_VERTEX(94.72f, -31.185f);
	ADD_EDGE_VERTEX(112.957f, -25.386f);

	ADD_EDGE_VERTEX(-97.04f, -3.021f);
	ADD_EDGE_VERTEX(-78.14f, -7.41f);
	ADD_EDGE_VERTEX(-49.389f, -6.76f);
	ADD_EDGE_VERTEX(-24.235f, 2.822f);
	ADD_EDGE_VERTEX(4.96f, 2.055f);
	ADD_EDGE_VERTEX(31.757f, -7.623f);
	ADD_EDGE_VERTEX(63.1f, -7.084f);
	ADD_EDGE_VERTEX(86.947f, 5.229f);
	ADD_EDGE_VERTEX(114.812f, 2.008f);

	for (i = 0; i < e1_size; ++i)
	{
		e1[i] = i;
	}

	for (i = 0; i < e2_size; ++i)
	{
		e2[i] = e1_size + i;
	}

	stitch_two_edges_exported(
		&vertices, 
		&triangles, 
		0, 
		e1, e1_size, 
		e2, e2_size, 
		NULL, 
		&poly);

	TEST_FACE(0, 0, 1, 9);
	TEST_FACE(1, 9, 1, 10);
	TEST_FACE(2, 1, 2, 10);
	TEST_FACE(3, 10, 2, 11);
	TEST_FACE(4, 2, 3, 11);
	TEST_FACE(5, 11, 3, 12);
	TEST_FACE(6, 3, 4, 12);
	TEST_FACE(7, 12, 4, 13);
	TEST_FACE(8, 13, 4, 14);
	TEST_FACE(9, 4, 5, 14);
	TEST_FACE(10, 5, 6, 14);
	TEST_FACE(11, 14, 6, 15);
	TEST_FACE(12, 6, 7, 15);
	TEST_FACE(13, 15, 7, 16);
	TEST_FACE(14, 16, 7, 17);
	TEST_FACE(15, 7, 8, 17);

	ei_array_clear(&triangles);
	ei_array_clear(&vertices);
}

void ei_approx_stitch_unittest::testConnect()
{
	eiPolyTessel	poly;
	const eiInt		e1_size = 9;
	const eiInt		e2_size = 8;
	eiIndex			e1[e1_size];
	eiIndex			e2[e2_size];
	ei_array		vertices;
	ei_array		triangles;
	eiRayVertex		vtx;
	eiRayTriangle	*tri;
	eiInt			i;

	memset(&poly, 0, sizeof(eiPolyTessel));

	ei_array_init(&vertices, sizeof(eiRayVertex));
	ei_array_init(&triangles, sizeof(eiRayTriangle));

	ADD_EDGE_VERTEX(-112.456f, -29.848f);
	ADD_EDGE_VERTEX(-81.573f, -34.668f);
	ADD_EDGE_VERTEX(-52.671f, -33.415f);
	ADD_EDGE_VERTEX(-24.773f, -28.504f);
	ADD_EDGE_VERTEX(3.897f, -15.112f);
	ADD_EDGE_VERTEX(27.758f, -39.045f);
	ADD_EDGE_VERTEX(58.589f, -29.93f);
	ADD_EDGE_VERTEX(94.72f, -31.185f);
	ADD_EDGE_VERTEX(112.957f, -25.386f);

	ADD_EDGE_VERTEX(-97.04f, -3.021f);
	ADD_EDGE_VERTEX(-78.14f, -7.41f);
	ADD_EDGE_VERTEX(-49.389f, -6.76f);
	ADD_EDGE_VERTEX(-24.235f, 2.822f);
	ADD_EDGE_VERTEX(4.96f, 2.055f);
	ADD_EDGE_VERTEX(31.757f, -7.623f);
	ADD_EDGE_VERTEX(63.1f, -7.084f);
	ADD_EDGE_VERTEX(86.947f, 5.229f);

	for (i = 0; i < e1_size; ++i)
	{
		e1[i] = i;
	}

	for (i = 0; i < e2_size; ++i)
	{
		e2[i] = e1_size + i;
	}

	connect_two_edges_exported(
		&triangles, 
		0, 
		e1, e1_size, 
		e2, e2_size, 
		NULL, 
		&poly);

	TEST_FACE(0, 0, 1, 9);
	TEST_FACE(1, 9, 1, 10);
	TEST_FACE(2, 1, 2, 10);
	TEST_FACE(3, 10, 2, 11);
	TEST_FACE(4, 2, 3, 11);
	TEST_FACE(5, 11, 3, 12);
	TEST_FACE(6, 3, 4, 12);
	TEST_FACE(7, 12, 4, 13);
	TEST_FACE(8, 4, 5, 13);
	TEST_FACE(9, 13, 5, 14);
	TEST_FACE(10, 5, 6, 14);
	TEST_FACE(11, 14, 6, 15);
	TEST_FACE(12, 6, 7, 15);
	TEST_FACE(13, 15, 7, 16);
	TEST_FACE(14, 7, 8, 16);

	ei_array_clear(&triangles);
	ei_array_clear(&vertices);
}
