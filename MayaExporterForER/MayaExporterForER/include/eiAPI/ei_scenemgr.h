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
 
#ifndef EI_SCENEMGR_H
#define EI_SCENEMGR_H

/** \brief The scene DAG Layer for interactions between the interface 
 * and the rendering core.
 * \file ei_scenemgr.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_options.h>
#include <eiAPI/ei_camera.h>
#include <eiAPI/ei_attributes.h>
#include <eiAPI/ei_instance.h>
#include <eiAPI/ei_instgroup.h>
#include <eiAPI/ei_object.h>
#include <eiAPI/ei_poly_object.h>
#include <eiAPI/ei_hair_object.h>
#include <eiAPI/ei_disc_object.h>
#include <eiAPI/ei_proc_object.h>
#include <eiAPI/ei_light.h>
#include <eiAPI/ei_material.h>
#include <eiAPI/ei_connection.h>
#include <eiCORE/ei_matrix.h>

#ifdef __cplusplus
extern "C" {
#endif

eiAPI void ei_scene_update_instances(
	eiMaster *master, 
	eiConnection *con, 
	eiNodeSystem *nodesys, 
	const eiTag cam_tag, 
	const eiTag scene_tag, 
	const eiTag light_insts, 
	const eiTag root_instgrp_tag, 
	const eiAttributes *attr, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform, 
	eiNode *instancer);

#ifdef __cplusplus
}
#endif

#endif
