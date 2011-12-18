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
 
#ifndef EI_CAMERA_H
#define EI_CAMERA_H

/** \brief The camera representation
 * \file ei_camera.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_output.h>
#include <eiCORE/ei_matrix.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The class encapsulates the information of a camera. */
#pragma pack(push, 1)
typedef struct eiCamera {
	eiNode						node;
	/* a data array of eiOutput */
	eiTag						outputs;
	eiTag						imager_list;
	eiTag						volume_list;
	eiTag						env_list;
	eiTag						lens_list;
	eiScalar					focal;
	eiScalar					aperture;
	eiScalar					aspect;
	eiInt						res_x;
	eiInt						res_y;
	eiInt						window_xmin;
	eiInt						window_xmax;
	eiInt						window_ymin;
	eiInt						window_ymax;
	eiScalar					clip_hither;
	eiScalar					clip_yon;
	/* transformation matrices */
	eiMatrix					camera_to_world;
	eiMatrix					motion_camera_to_world;
	eiMatrix					world_to_camera;
	eiMatrix					motion_world_to_camera;
	/* precomputed parameters */
	eiScalar					image_center_x;
	eiScalar					image_center_y;
	eiScalar					camera_to_pixel_x;
	eiScalar					camera_to_pixel_y;
	eiScalar					pixel_to_camera_x;
	eiScalar					pixel_to_camera_y;
	eiScalar					coeffz;
	eiScalar					constz;
	eiScalar					focalz;
	eiScalar					project_near_clip;
} eiCamera;
#pragma pack(pop)

void ei_camera_init(eiNodeSystem *nodesys, eiNode *node);
void ei_camera_exit(eiNodeSystem *nodesys, eiNode *node);

/** \brief Instance this element into global scene database or update 
 * the existing element */
void ei_camera_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

void ei_camera_add_output(eiCamera *cam, eiDatabase *db, const eiOutput *output);
void ei_camera_clear_outputs(eiCamera *cam, eiDatabase *db);

void ei_camera_add_imager(eiCamera *cam, eiDatabase *db, const eiTag shader);
void ei_camera_add_volume(eiCamera *cam, eiDatabase *db, const eiTag shader);
void ei_camera_add_environment(eiCamera *cam, eiDatabase *db, const eiTag shader);
void ei_camera_add_lens(eiCamera *cam, eiDatabase *db, const eiTag shader);

/** \brief Get ray position for orthogonal projection. */
void ei_camera_get_ray_pos(
	eiCamera *cam, eiVector *p, const eiScalar sx, const eiScalar sy);
/** \brief Get ray direction for perspective projection. */
void ei_camera_get_ray_dir(
	eiCamera *cam, eiVector *p, const eiScalar sx, const eiScalar sy);

/** \brief Project a point from object space to screen space. */
void ei_camera_object_to_screen(
	eiCamera *cam, 
	eiVector *p_pos, 
	const eiVector *o_pos, 
	const eiMatrix *object_to_view);
/** \brief Project a bounding box from object space to screen space. */
void ei_camera_object_to_screen_box(
	eiCamera *cam, 
	eiBound *screen_box, 
	const eiBound *box, 
	const eiMatrix *object_to_view);

eiNodeObject *ei_create_camera_node_object(void *param);
/** \brief Install the node into node system */
void ei_install_camera_node(eiNodeSystem *nodesys);

#ifdef __cplusplus
}
#endif

#endif
