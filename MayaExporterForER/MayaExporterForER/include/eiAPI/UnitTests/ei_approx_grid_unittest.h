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

class ei_approx_grid_unittest : public CxxTest::TestSuite
{
public:
	void setUp();
	void tearDown();

	void testGrid();
};

void ei_approx_grid_unittest::setUp()
{
}

void ei_approx_grid_unittest::tearDown()
{
}

#define TEST_FACE(i, i1, i2, i3) \
	tri = (eiRayTriangle *)ei_array_get(&triangles, (i));\
	TS_ASSERT(tri->v1 == (i1));\
	TS_ASSERT(tri->v2 == (i2));\
	TS_ASSERT(tri->v3 == (i3));

void ei_approx_grid_unittest::testGrid()
{
	eiPolyTessel	poly;
	ei_array		vertices;
	ei_array		triangles;
	eiVector		pos1, pos2, pos3;
	eiRayTriangle	*tri;

	memset(&poly, 0, sizeof(eiPolyTessel));

	ei_array_init(&vertices, sizeof(eiRayVertex));
	ei_array_init(&triangles, sizeof(eiRayTriangle));

	setv(&pos1, 0.0f, 1.0f, 0.0f);
	setv(&pos2, -0.577f, 0.0f, 0.0f);
	setv(&pos3, 0.577f, 0.0f, 0.0f);

	build_triangle_grid_exported(
		&vertices, 
		NULL, 
		NULL, 
		NULL, 
		&triangles, 
		0, 
		0, 0, 0, 
		&pos1, &pos2, &pos3, 
		&pos1, &pos2, &pos3, 
		5, 
		0, 
		NULL, 
		&poly);

	TEST_FACE(0, 0, 1, 2);
	TEST_FACE(1, 1, 3, 4);
	TEST_FACE(2, 1, 4, 2);
	TEST_FACE(3, 2, 4, 5);
	TEST_FACE(4, 3, 6, 7);
	TEST_FACE(5, 3, 7, 4);
	TEST_FACE(6, 4, 7, 8);
	TEST_FACE(7, 4, 8, 5);
	TEST_FACE(8, 5, 8, 9);
	TEST_FACE(9, 6, 10, 11);
	TEST_FACE(10, 6, 11, 7);
	TEST_FACE(11, 7, 11, 12);
	TEST_FACE(12, 7, 12, 8);
	TEST_FACE(13, 8, 12, 13);
	TEST_FACE(14, 8, 13, 9);
	TEST_FACE(15, 9, 13, 14);
	TEST_FACE(16, 10, 15, 16);
	TEST_FACE(17, 10, 16, 11);
	TEST_FACE(18, 11, 16, 17);
	TEST_FACE(19, 11, 17, 12);
	TEST_FACE(20, 12, 17, 18);
	TEST_FACE(21, 12, 18, 13);
	TEST_FACE(22, 13, 18, 19);
	TEST_FACE(23, 13, 19, 14);
	TEST_FACE(24, 14, 19, 20);

	ei_array_clear(&triangles);
	ei_array_clear(&vertices);
}
