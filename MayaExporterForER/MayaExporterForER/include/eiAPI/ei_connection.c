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

#include <eiAPI/ei_connection.h>
#include <eiCORE/ei_assert.h>

#define NUM_SEVERITY_LEVELS		5

static void ei_default_connection_print(
	eiConnection *connection, 
	const eiInt severity, 
	const char *message)
{
	static const char	*severity_strings[ NUM_SEVERITY_LEVELS ] = {"FATAL",
																	"ERROR",
																	"WARNING",
																	"INFO",
																	"DEBUG"};

	if (severity < 1 || severity > NUM_SEVERITY_LEVELS)
	{
		return;
	}

	printf("%s> %s", severity_strings[severity - 1], message);
}

static eiBool ei_default_connection_progress(
	eiConnection *connection, 
	const eiScalar percent)
{
	printf("PROG> %f %%\n", percent);

	return eiTRUE;
}

static void ei_default_connection_clear_tile(
	eiConnection *connection, 
	const eiInt left, 
	const eiInt right, 
	const eiInt top, 
	const eiInt bottom, 
	const eiHostID host)
{
}

static void ei_default_connection_update_tile(
	eiConnection *connection, 
	eiFrameBufferCache *colorFrameBuffer, 
	eiFrameBufferCache *opacityFrameBuffer, 
	ei_array *frameBuffers, 
	const eiInt left, 
	const eiInt right, 
	const eiInt top, 
	const eiInt bottom)
{
}

static void ei_default_connection_draw_pixel(
	eiConnection *connection, 
	const eiInt x, 
	const eiInt y, 
	const eiVector *color)
{
}

static void ei_default_connection_update_sub_window(
	eiConnection *connection, 
	const eiInt left, 
	const eiInt right, 
	const eiInt top, 
	const eiInt bottom)
{
}

static eiConnection g_DefaultConnection = {
	ei_default_connection_print, 
	ei_default_connection_progress, 
	ei_default_connection_clear_tile, 
	ei_default_connection_update_tile, 
	ei_default_connection_draw_pixel, 
	ei_default_connection_update_sub_window
};

eiConnection *ei_get_default_connection()
{
	return &g_DefaultConnection;
}
