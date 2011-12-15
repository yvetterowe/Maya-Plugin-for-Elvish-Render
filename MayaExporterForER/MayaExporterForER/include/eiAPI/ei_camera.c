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

#include <eiAPI/ei_camera.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_assert.h>

void ei_camera_init(eiNodeSystem *nodesys, eiNode *node)
{
	eiCamera	*cam;

	cam = (eiCamera *)node;

	cam->node.type = EI_ELEMENT_CAMERA;

	cam->window_xmin = eiMIN_INT;
	cam->window_xmax = eiMAX_INT;
	cam->window_ymin = eiMIN_INT;
	cam->window_ymax = eiMAX_INT;

	cam->clip_hither = eiSCALAR_EPS;
	cam->clip_yon = eiMAX_SCALAR;

	cam->outputs = ei_create_data_array(nodesys->m_db, EI_DATA_TYPE_OUTPUT);

	cam->imager_list = eiNULL_TAG;
	cam->volume_list = eiNULL_TAG;
	cam->env_list = eiNULL_TAG;
	cam->lens_list = eiNULL_TAG;
}

void ei_camera_exit(eiNodeSystem *nodesys, eiNode *node)
{
	eiCamera	*cam;

	cam = (eiCamera *)node;

	if (cam->lens_list != eiNULL_TAG)
	{
		ei_delete_data_array(nodesys->m_db, cam->lens_list);
		cam->lens_list = eiNULL_TAG;
	}
	if (cam->env_list != eiNULL_TAG)
	{
		ei_delete_data_array(nodesys->m_db, cam->env_list);
		cam->env_list = eiNULL_TAG;
	}
	if (cam->volume_list != eiNULL_TAG)
	{
		ei_delete_data_array(nodesys->m_db, cam->volume_list);
		cam->volume_list = eiNULL_TAG;
	}
	if (cam->imager_list != eiNULL_TAG)
	{
		ei_delete_data_array(nodesys->m_db, cam->imager_list);
		cam->imager_list = eiNULL_TAG;
	}

	ei_delete_data_array(nodesys->m_db, cam->outputs);
	cam->outputs = eiNULL_TAG;
}

void ei_camera_update_instance(
	eiNodeSystem *nodesys, 
	eiObjectRepCache *cache, 
	eiNode *node, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer)
{
	eiCamera	*cam;

	cam = (eiCamera *)node;

	movm(&cam->camera_to_world, transform);
	movm(&cam->motion_camera_to_world, motion_transform);
	inv(&cam->world_to_camera, &cam->camera_to_world);
	inv(&cam->motion_world_to_camera, &cam->motion_camera_to_world);

	cam->image_center_x = (eiScalar)cam->res_x * 0.5f;
	cam->image_center_y = (eiScalar)cam->res_y * 0.5f;

	cam->camera_to_pixel_x = (eiScalar)cam->res_x / cam->aperture;
	cam->camera_to_pixel_y = - (eiScalar)cam->res_y / (cam->aperture / cam->aspect);
	cam->pixel_to_camera_x = cam->aperture / (eiScalar)cam->res_x;
	cam->pixel_to_camera_y = - (cam->aperture / cam->aspect) / (eiScalar)cam->res_y;

	/* perspective projection */
	if (cam->focal != eiMAX_SCALAR)
	{
		cam->coeffz = 1.0f;
		cam->constz = 0.0f;
		cam->focalz = cam->focal;
	}
	else
	{
		cam->coeffz = 0.0f;
		cam->constz = -1.0f;
		cam->focalz = 1.0f;
	}

	/* z-axis is outward */
	cam->project_near_clip = MIN(-cam->clip_hither, -eiSCALAR_EPS);
}

void ei_camera_add_output(eiCamera *cam, eiDatabase *db, const eiOutput *output)
{
	ei_data_array_push_back(db, cam->outputs, (const void *)output);
}

void ei_camera_clear_outputs(eiCamera *cam, eiDatabase *db)
{
	ei_data_array_clear(db, cam->outputs);
}

void ei_camera_add_imager(eiCamera *cam, eiDatabase *db, const eiTag shader)
{
	if (cam->imager_list == eiNULL_TAG)
	{
		cam->imager_list = ei_create_data_array(db, EI_DATA_TYPE_TAG);
	}
	ei_data_array_push_back(db, cam->imager_list, &shader);
}

void ei_camera_add_volume(eiCamera *cam, eiDatabase *db, const eiTag shader)
{
	if (cam->volume_list == eiNULL_TAG)
	{
		cam->volume_list = ei_create_data_array(db, EI_DATA_TYPE_TAG);
	}
	ei_data_array_push_back(db, cam->volume_list, &shader);
}

void ei_camera_add_environment(eiCamera *cam, eiDatabase *db, const eiTag shader)
{
	if (cam->env_list == eiNULL_TAG)
	{
		cam->env_list = ei_create_data_array(db, EI_DATA_TYPE_TAG);
	}
	ei_data_array_push_back(db, cam->env_list, &shader);
}

void ei_camera_add_lens(eiCamera *cam, eiDatabase *db, const eiTag shader)
{
	if (cam->lens_list == eiNULL_TAG)
	{
		cam->lens_list = ei_create_data_array(db, EI_DATA_TYPE_TAG);
	}
	ei_data_array_push_back(db, cam->lens_list, &shader);
}

void ei_camera_get_ray_pos(
	eiCamera *cam, eiVector *p, const eiScalar sx, const eiScalar sy)
{
	p->x = (sx - cam->image_center_x) * cam->pixel_to_camera_x;
	p->y = (sy - cam->image_center_y) * cam->pixel_to_camera_y;
	p->z = 0.0f;
}

void ei_camera_get_ray_dir(
	eiCamera *cam, eiVector *p, const eiScalar sx, const eiScalar sy)
{
	p->x = (sx - cam->image_center_x) * cam->pixel_to_camera_x;
	p->y = (sy - cam->image_center_y) * cam->pixel_to_camera_y;
	p->z = - cam->focal;
}

void ei_camera_object_to_screen(
	eiCamera *cam, 
	eiVector *p_pos, 
	const eiVector *o_pos, 
	const eiMatrix *object_to_view)
{
	eiVector	v_pos;
	eiScalar	z;
	eiScalar	k;

	/* transform from object space to view space */
	point_transform(&v_pos, o_pos, object_to_view);

	/* prevent from invalid projection division */
	if (v_pos.z > cam->project_near_clip)
	{
		v_pos.z = cam->project_near_clip;
	}

	/* project from view space to screen space */
	z = cam->focalz / v_pos.z;
	p_pos->z = z;
	k = z * cam->coeffz + cam->constz;
	p_pos->x = cam->image_center_x - v_pos.x * k * cam->camera_to_pixel_x;
	p_pos->y = cam->image_center_y - v_pos.y * k * cam->camera_to_pixel_y;
}

void ei_camera_object_to_screen_box(
	eiCamera *cam, 
	eiBound *screen_box, 
	const eiBound *box, 
	const eiMatrix *object_to_view)
{
	eiVector	pos, vx;

	pos.x = box->xmin;
	pos.y = box->ymin;
	pos.z = box->zmin;
	ei_camera_object_to_screen(cam, &vx, &pos, object_to_view);
	setbv(screen_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymin;
	pos.z = box->zmax;
	ei_camera_object_to_screen(cam, &vx, &pos, object_to_view);
	addbv(screen_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymax;
	pos.z = box->zmin;
	ei_camera_object_to_screen(cam, &vx, &pos, object_to_view);
	addbv(screen_box, &vx);

	pos.x = box->xmin;
	pos.y = box->ymax;
	pos.z = box->zmax;
	ei_camera_object_to_screen(cam, &vx, &pos, object_to_view);
	addbv(screen_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymin;
	pos.z = box->zmin;
	ei_camera_object_to_screen(cam, &vx, &pos, object_to_view);
	addbv(screen_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymin;
	pos.z = box->zmax;
	ei_camera_object_to_screen(cam, &vx, &pos, object_to_view);
	addbv(screen_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymax;
	pos.z = box->zmin;
	ei_camera_object_to_screen(cam, &vx, &pos, object_to_view);
	addbv(screen_box, &vx);

	pos.x = box->xmax;
	pos.y = box->ymax;
	pos.z = box->zmax;
	ei_camera_object_to_screen(cam, &vx, &pos, object_to_view);
	addbv(screen_box, &vx);
}

eiNodeObject *ei_create_camera_node_object(void *param)
{
	eiElement	*element;

	element = ei_create_element();

	element->base.base.deletethis = ei_element_deletethis;
	element->base.init_node = ei_camera_init;
	element->base.exit_node = ei_camera_exit;
	element->base.node_changed = NULL;
	element->update_instance = ei_camera_update_instance;

	return ((eiNodeObject *)element);
}

void ei_install_camera_node(eiNodeSystem *nodesys)
{
	eiTag		desc_tag;
	eiNodeDesc	*desc;
	eiTag		default_tag;
	eiInt		default_int;
	eiScalar	default_scalar;

	desc = ei_nodesys_node_desc(nodesys, &desc_tag, "camera");
	if (desc == NULL)
	{
		return;
	}

	default_tag = eiNULL_TAG;
	default_int = 0;
	default_scalar = 0.0f;

	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"outputs", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"imager_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"volume_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"env_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_TAG, 
		"lens_list", 
		&default_tag);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"focal", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"aperture", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"aspect", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"res_x", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"res_y", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"window_xmin", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"window_xmax", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"window_ymin", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_INT, 
		"window_ymax", 
		&default_int);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"clip_hither", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"clip_yon", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_MATRIX, 
		"camera_to_world", 
		&g_IdentityMatrix);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_MATRIX, 
		"motion_camera_to_world", 
		&g_IdentityMatrix);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_MATRIX, 
		"world_to_camera", 
		&g_IdentityMatrix);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_MATRIX, 
		"motion_world_to_camera", 
		&g_IdentityMatrix);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"image_center_x", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"image_center_y", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"camera_to_pixel_x", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"camera_to_pixel_y", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"pixel_to_camera_x", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"pixel_to_camera_y", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"coeffz", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"constz", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"focalz", 
		&default_scalar);
	ei_nodesys_add_parameter(
		nodesys, 
		desc, 
		eiCONSTANT, 
		EI_DATA_TYPE_SCALAR, 
		"project_near_clip", 
		&default_scalar);

	ei_nodesys_end_node_desc(nodesys, desc, desc_tag);
}
