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

#include <eiAPI/ei_dbg_canvas.h>
#include <eiAPI/ei_rayhair.h>

#define IMAGE_WIDTH		640
#define IMAGE_HEIGHT	480
#define SUBDIV_DEPTH	3

eiBool ei_curve3_intersect(
	const eiScalar x, 
	const eiScalar y, 
	eiCurve3 *c, 
	const eiInt max_depth, 
	eiScalar *t, 
	eiScalar *z)
{
	eiBool	result;
	eiInt	i;

	/* transform the curve to center (x, y) to (0, 0) */
	for (i = 0; i < 4; ++i)
	{
		c->p[i].x -= x;
		c->p[i].y -= y;
	}

	result = ei_curve3_converge_exported(c, max_depth, 0.0f, 1.0f, t, z);

	/* restore the curve to the original space */
	for (i = 0; i < 4; ++i)
	{
		c->p[i].x += x;
		c->p[i].y += y;
	}

	return result;
}

class ei_raytracer_hair_unittest : public CxxTest::TestSuite
{
public:
	void setUp();
	void tearDown();

	void testOverall();

private:
};

void ei_raytracer_hair_unittest::setUp()
{
}

void ei_raytracer_hair_unittest::tearDown()
{
}

void ei_raytracer_hair_unittest::testOverall()
{
	char		cur_dir[ EI_MAX_FILE_NAME_LEN ];
	eiCurve3	curve1, curve2;

	setv(&curve1.p[0].xyz, 122.0f, 142.0f, 5.0f);
	curve1.p[0].w = 4.0f;
	setv(&curve1.p[1].xyz, 211.0f, 96.0f, 5.0f);
	curve1.p[1].w = 3.75f;
	setv(&curve1.p[2].xyz, 313.0f, 183.0f, 5.0f);
	curve1.p[2].w = 3.5f;
	setv(&curve1.p[3].xyz, 274.0f, 292.0f, 5.0f);
	curve1.p[3].w = 3.25f;
	setv(&curve2.p[0].xyz, 274.0f, 292.0f, 5.0f);
	curve2.p[0].w = 3.25f;
	setv(&curve2.p[1].xyz, 234.0f, 385.0f, 5.0f);
	curve2.p[1].w = 3.0f;
	setv(&curve2.p[2].xyz, 476.0f, 363.0f, 5.0f);
	curve2.p[2].w = 2.75f;
	setv(&curve2.p[3].xyz, 584.0f, 255.0f, 5.0f);
	curve2.p[3].w = 2.5f;
	ei_curve3_precompute_exported(&curve1);
	ei_curve3_precompute_exported(&curve2);
	TS_ASSERT(curve1.max_radius == 4.0f);
	TS_ASSERT(curve2.max_radius == 3.25f);
	
	ei_get_current_directory(cur_dir);

	ei_dbg_canvas_init();
	ei_dbg_canvas_resize(IMAGE_WIDTH, IMAGE_HEIGHT);

	for (eiInt j = 0; j < IMAGE_HEIGHT; ++j)
	{
		for (eiInt i = 0; i < IMAGE_WIDTH; ++i)
		{
			eiScalar	t, z;

			if (ei_curve3_intersect((eiScalar)i, (eiScalar)j, &curve1, SUBDIV_DEPTH, &t, &z) || 
				ei_curve3_intersect((eiScalar)i, (eiScalar)j, &curve2, SUBDIV_DEPTH, &t, &z))
			{
				eiScalar	c;
				c = (((eiInt)(t * 20.0f) % 2) == 0) ? 0.5f : 1.0f;
				ei_dbg_canvas_set_pixel((eiScalar)i, (eiScalar)j, c, c, c);
			}
			else
			{
				ei_dbg_canvas_set_pixel((eiScalar)i, (eiScalar)j, 0.0f, 0.0f, 0.0f);
			}
		}
	}

	char filename[ EI_MAX_FILE_NAME_LEN ];

	ei_append_filename(filename, cur_dir, "rtoutput_hair.bmp");

	ei_dbg_canvas_output(filename);
	ei_dbg_canvas_clear();
}
