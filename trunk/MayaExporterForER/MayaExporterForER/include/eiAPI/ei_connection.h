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
 
#ifndef EI_CONNECTION_H
#define EI_CONNECTION_H

/** \brief Connections for interactions between client 
 * applications and the renderer.
 * \file ei_connection.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiCORE/ei_array.h>
#include <eiCORE/ei_vector.h>
#include <eiAPI/ei_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The interface to be implemented by users to interact between client 
 * applications and the renderer. the renderer makes sure that the functions in 
 * this class will always be called in one single thread in sequential 
 * manner, so clients only need to make sure that corruptions of 
 * multi-threading don't happen in their own implementations, not in 
 * the renderer.
 */
typedef struct eiConnection		eiConnection;

/** \brief This will be called if any error detected. 
 * this makes sure that we can always call error handler 
 * without NULL checking. this is also used to print a 
 * string on the display or log it. */
typedef void (*ei_connection_print)(
	eiConnection *connection, 
	const eiInt severity, 
	const char *message);
/** \brief This function updates the progress being displayed 
 * and check the abort status. it returns a boolean 
 * indicates whether to abort rendering, if false, the 
 * rendering will stop at once.
 * @param percent How many rendering jobs are completed 
 * in percentage. */
typedef eiBool (*ei_connection_progress)(
	eiConnection *connection, 
	const eiScalar percent);
/** \brief Clear an image tile on the client application. 
 * this will be called when starting to render a tile. */
typedef void (*ei_connection_clear_tile)(
	eiConnection *connection, 
	const eiInt left, 
	const eiInt right, 
	const eiInt top, 
	const eiInt bottom, 
	const eiHostID host);
/** \brief Update an image tile on the client application. 
 * this will be called when a tile is finished. */
typedef void (*ei_connection_update_tile)(
	eiConnection *connection, 
	eiFrameBufferCache *colorFrameBuffer, 
	eiFrameBufferCache *opacityFrameBuffer, 
	ei_array *frameBuffers, 
	const eiInt left, 
	const eiInt right, 
	const eiInt top, 
	const eiInt bottom);
/** \brief Set a pixel by location with given color in 
 * client application.
 * @param x The x location of the pixel to draw.
 * @param y The y location of the pixel to draw.
 * @param color The color of the pixel to draw. */
typedef void (*ei_connection_draw_pixel)(
	eiConnection *connection, 
	const eiInt x, 
	const eiInt y, 
	const eiVector *color);
/** \brief Update a sub-window of client application. */
typedef void (*ei_connection_update_sub_window)(
	eiConnection *connection, 
	const eiInt left, 
	const eiInt right, 
	const eiInt top, 
	const eiInt bottom);

struct eiConnection {
	ei_connection_print				print;
	ei_connection_progress			progress;
	ei_connection_clear_tile		clear_tile;
	ei_connection_update_tile		update_tile;
	ei_connection_draw_pixel		draw_pixel;
	ei_connection_update_sub_window	update_sub_window;
};

/** \brief Get the default connection that will be 
 * used if no custom connection is supplied. */
eiAPI eiConnection *ei_get_default_connection();

#ifdef __cplusplus
}
#endif

#endif
