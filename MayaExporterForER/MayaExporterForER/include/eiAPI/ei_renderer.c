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

#include <eiAPI/ei_renderer.h>
#include <eiAPI/ei_state.h>
#include <eiAPI/ei_raytracer.h>
#include <eiAPI/ei_globillum.h>
#include <eiAPI/ei_photon.h>
#include <eiAPI/ei_api.h>
#include <eiAPI/ei_connection.h>
#include <eiAPI/ei_scenemgr.h>
#include <eiAPI/ei_image.h>
#include <eiAPI/ei_texture.h>
#include <eiAPI/ei.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_atomic_ops.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_data_gen.h>
#include <eiCORE/ei_message.h>
#include <eiCORE/ei_timer.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_network.h>
#include <eiCORE/ei_config.h>
#include <eiCORE/ei_assert.h>

#define EI_LIGHT_INSTANCE_SLOT_SIZE		256
#define NUM_PHOTONS_PER_JOB				1000
#define FINALGATHER_DENSITY_SCALE		16.0f
#define FINALGATHER_MIN_SPACING			2.0f
#define FINALGATHER_MAX_PREPASS			3

/** \brief The node for mapping output variable requirements 
 * to frame buffer tags */
typedef struct eiFrameBufferNode {
	ei_btree_node		node;
	/* the output variable, as the key, mutiple output variables 
	   can share the same frame buffer if their variable names 
	   and data types match */
	eiOutputVariable	outvar;
} eiFrameBufferNode;

/** \brief The mapping from output variable requirements to 
 * frame buffer tags */
typedef struct eiFrameBufferMap {
	ei_btree			map;
} eiFrameBufferMap;

static void ei_framebuffer_node_init(
	eiFrameBufferNode *node, 
	const eiOutputVariable *outvar)
{
	ei_btree_node_init(&node->node);
	memcpy(&node->outvar, outvar, sizeof(eiOutputVariable));
}

static void ei_framebuffer_node_exit(eiFrameBufferNode *node)
{
	ei_btree_node_clear(&node->node);
}

static eiFrameBufferNode *ei_create_framebuffer_node(
	const eiOutputVariable *outvar)
{
	eiFrameBufferNode *node = (eiFrameBufferNode *)ei_allocate(sizeof(eiFrameBufferNode));
	
	ei_framebuffer_node_init(node, outvar);

	return node;
}

static eiIntptr ei_framebuffer_map_compare(void *lhs, void *rhs, void *param)
{
	eiFrameBufferNode	*lnode;
	eiFrameBufferNode	*rnode;

	lnode = (eiFrameBufferNode *)lhs;
	rnode = (eiFrameBufferNode *)rhs;

	if (lnode->outvar.datatype == rnode->outvar.datatype)
	{
		return strcmp(lnode->outvar.name, rnode->outvar.name);
	}
	else
	{
		return lnode->outvar.datatype - rnode->outvar.datatype;
	}
}

static void ei_framebuffer_map_delete_node(ei_btree_node *node, void *param)
{
	if (node == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_framebuffer_node_exit((eiFrameBufferNode *)node);

	eiCHECK_FREE(node);
}

void ei_framebuffer_map_init(eiFrameBufferMap *map)
{
	ei_btree_init(&map->map, ei_framebuffer_map_compare, ei_framebuffer_map_delete_node, NULL);
}

void ei_framebuffer_map_exit(eiFrameBufferMap *map)
{
	ei_btree_clear(&map->map);
}

void ei_framebuffer_map_add(
	eiFrameBufferMap *map, 
	const eiOutputVariable *outvar)
{
	eiFrameBufferNode		*node;

	node = ei_create_framebuffer_node(outvar);

	ei_btree_insert(&map->map, &node->node, NULL);
}

eiTag ei_framebuffer_map_find(
	eiFrameBufferMap *map, 
	const eiOutputVariable *outvar)
{
	eiFrameBufferNode		key;
	eiFrameBufferNode		*node;

	ei_framebuffer_node_init(&key, outvar);
	node = (eiFrameBufferNode *)ei_btree_lookup(&map->map, &key.node, NULL);

	if (node == NULL)
	{
		return eiNULL_TAG;
	}

	return node->outvar.framebuffer;
}

typedef enum eiStep {
	eiLEFT, eiRIGHT, eiUP, eiDOWN, 
} eiStep;

typedef struct eiHilbertGuide {
	struct eiHilbertGuide	*next;
	eiTag					buck;
} eiHilbertGuide;

static void ei_hilbert_guide_init(eiHilbertGuide *guide)
{
	guide->next = NULL;
	guide->buck = eiNULL_TAG;
}

#define hlink(x1,y1,x2,y2)	hg[x1][y1].next = &hg[x2][y2];

static void hilbert(
	eiHilbertGuide **hg, 
	eiInt x, 
	eiInt y, 
	eiInt size, 
	eiInt level, 
	eiStep _step)
{
	if (level == 0) {
	} else if (level == 1) {
		switch (_step) {
		case eiLEFT:
			hlink(x, y, x + 1, y);
			hlink(x + 1, y, x + 1, y + 1);
			hlink(x + 1, y + 1, x, y + 1);
			break;
		case eiRIGHT:
			hlink(x, y, x - 1, y);
			hlink(x - 1, y, x - 1, y - 1);
			hlink(x - 1, y - 1, x, y - 1);
			break;
		case eiUP:
			hlink(x, y, x, y + 1);
			hlink(x, y + 1, x + 1, y + 1);
			hlink(x + 1, y + 1, x + 1, y);
			break;
		case eiDOWN:
			hlink(x, y, x, y - 1);
			hlink(x, y - 1, x - 1, y - 1);
			hlink(x - 1, y - 1, x - 1, y);
			break;
		}
	} else {
		eiInt	size2 = size / 2;
		switch (_step) {
		case eiLEFT:
			hilbert(hg, x, y, size2, level - 1, eiUP);
			hlink(x + size2 - 1, y, x + size2, y);
			hilbert(hg, x + size2, y, size2, level - 1, eiLEFT);
			hlink(x + size2, y + size2 - 1, x + size2, y + size2);
			hilbert(hg, x + size2, y + size2, size2, level - 1, eiLEFT);
			hlink(x + size2, y + size - 1, x + size2 - 1, y + size - 1);
			hilbert(hg, x + size2 - 1, y + size - 1, size2, level - 1, eiDOWN);
			break;
		case eiRIGHT:
			hilbert(hg, x, y, size2, level - 1, eiDOWN);
			hlink(x - size2 + 1, y, x - size2, y);
			hilbert(hg, x - size2, y, size2, level - 1, eiRIGHT);
			hlink(x - size2, y - size2 + 1, x - size2, y - size2);
			hilbert(hg, x - size2, y - size2, size2, level - 1, eiRIGHT);
			hlink(x - size2, y - size + 1, x - size2 + 1, y - size + 1);
			hilbert(hg, x - size2 + 1, y - size + 1, size2, level - 1, eiUP);
			break;
		case eiUP:
			hilbert(hg, x, y, size2, level - 1, eiLEFT);
			hlink(x, y + size2 - 1, x, y + size2);
			hilbert(hg, x, y + size2, size2, level - 1, eiUP);
			hlink(x + size2 - 1, y + size2, x + size2, y + size2);
			hilbert(hg, x + size2, y + size2, size2, level - 1, eiUP);
			hlink(x + size - 1, y + size2, x + size - 1, y + size2 - 1);
			hilbert(hg, x + size - 1, y + size2 - 1, size2, level - 1, eiRIGHT);
			break;
		case eiDOWN:
			hilbert(hg, x, y, size2, level - 1, eiRIGHT);
			hlink(x, y - size2 + 1, x, y - size2);
			hilbert(hg, x, y - size2, size2, level - 1, eiDOWN);
			hlink(x - size2 + 1, y - size2, x - size2, y - size2);
			hilbert(hg, x - size2, y - size2, size2, level - 1, eiDOWN);
			hlink(x - size + 1, y - size2, x - size + 1, y - size2 + 1);
			hilbert(hg, x - size + 1, y - size2 + 1, size2, level - 1, eiLEFT);
			break;
		}
	}
}

static void sort_buckets(
	eiInt x1, 
	eiInt y1, 
	eiInt x2, 
	eiInt y2, 
	eiBuffer *buckets, 
	eiMaster *master)
{
	eiHilbertGuide	**hg;
	eiHilbertGuide	*ptr;
	eiInt	dx, dy, max_size, block_size, num_level;
	eiInt	i, j;
	dx = x2 - x1;
	dy = y2 - y1;
	max_size = MAX(dx, dy);
	block_size = 1;
	num_level = 0;
	for (; block_size < max_size; ++ num_level) {
		block_size <<= 1;
	}
	hg = (eiHilbertGuide **)ei_allocate(sizeof(eiHilbertGuide *) * block_size);
	for (i = 0; i < block_size; ++i) {
		hg[i] = (eiHilbertGuide *)ei_allocate(sizeof(eiHilbertGuide) * block_size);
	}
	for (j = 0; j < block_size; ++j) {
		for (i = 0; i < block_size; ++i) {
			ei_hilbert_guide_init(&hg[i][j]);
		}
	}
	for (i = x1; i < x2; ++i) {
		for (j = y1; j < y2; ++j) {
			hg[i][j].buck = *((eiTag *)ei_buffer_getptr(buckets, i, j));
		}
	}
	hilbert(hg, x1, y1, block_size, num_level, eiUP);
	ptr = &hg[x1][y1];
	do {
		if (ptr->buck != eiNULL_TAG) {
			ei_master_add_job(master, ptr->buck);
		}
		ptr = ptr->next;
	} while (ptr != NULL);
	for (i = 0; i < block_size; ++i) {
		ei_free(hg[i]);
	}
	ei_free(hg);
}

/** \brief An internal class encapsulates all rendering 
 * functionalities of this renderer. */
struct eiRenderer {
	eiDatabase			*db;
	eiExecutor			*exec;
	eiMaster			*master;
	eiGlobals			globals;
	eiNodeSystem		*nodesys;
	eiRayTracer			*rt;
	eiConnection		*con;
	eiAttributes		default_attr;
	eiInt				noXBuckets;
	eiInt				noYBuckets;
	eiInt				noXBuckets_1;
	eiInt				noYBuckets_1;
	eiBuffer			buckets;
	eiTag				colorFrameBuffer;
	eiTag				opacityFrameBuffer;
	eiTag				frameBuffers;
	eiFrameBufferMap	frameBufferMap;
	eiTag				lightInstances;
	eiTag				causticMap;
	eiTag				globillumMap;
	eiTag				irradCache;
	ei_array			passIrradBuffers;
};

/** \brief The main rendering process. */
typedef struct eiRenderProcess {
	eiProcess		base;
	eiDatabase		*db;
	eiConnection	*con;
	eiRenderer		*rend;
	eiScalar		last_percent;
} eiRenderProcess;

static eiBool ei_render_process_one_job_started(
	eiProcess *process, 
	const eiTag job, 
	const eiHostID hostId, 
	const eiThreadID threadId, 
	eiMessage *msg)
{
	eiRenderProcess		*pProcess;
	eiBucketJob			*bucketJob;

	pProcess = (eiRenderProcess *)process;
	bucketJob = (eiBucketJob *)ei_db_access(pProcess->db, job);

	/* record our interested information in the message for this event. */
	msg->type = EI_MSG_BUCKET_STARTED;
	msg->bucket_started_params.job = job;
	msg->bucket_started_params.pos_i = bucketJob->pos_i;
	msg->bucket_started_params.pos_j = bucketJob->pos_j;
	msg->bucket_started_params.rect = bucketJob->rect;
	msg->bucket_started_params.host = hostId;

	ei_db_end(pProcess->db, job);

	/* indicate that we are interested in this message, so that this message 
	   will be added to the message queue, and we will handle it in "update" 
	   function later. */
	return eiTRUE;
}

static eiBool ei_render_process_one_job_finished(
	eiProcess *process, 
	const eiTag job, 
	const eiHostID hostId, 
	const eiThreadID threadId, 
	eiMessage *msg)
{
	eiRenderProcess		*pProcess;
	eiBucketJob			*bucketJob;

	pProcess = (eiRenderProcess *)process;
	bucketJob = (eiBucketJob *)ei_db_access(pProcess->db, job);

	/* record our interested information in the message for this event. */
	msg->type = EI_MSG_BUCKET_FINISHED;
	msg->bucket_finished_params.job = job;
	msg->bucket_finished_params.hostId = hostId;
	msg->bucket_finished_params.threadId = threadId;
	msg->bucket_finished_params.pos_i = bucketJob->pos_i;
	msg->bucket_finished_params.pos_j = bucketJob->pos_j;
	msg->bucket_finished_params.rect = bucketJob->rect;

	ei_db_end(pProcess->db, job);

	/* indicate that we are interested in this message, so that this message 
	   will be added to the message queue, and we will handle it in "update" 
	   function later. */
	return eiTRUE;
}

static eiBool ei_render_process_one_worker_finished(
	eiProcess *process, 
	eiBaseWorker *pWorker, 
	eiMessage *msg)
{
	return eiFALSE;
}

static eiBool ei_render_process_progress(
	eiProcess *process, 
	const eiScalar percent)
{
	eiRenderProcess		*pProcess;
	eiBool				to_abort;

	pProcess = (eiRenderProcess *)process;

	/* update the progress and check for abort */
	to_abort = eiFALSE;

	if (pProcess->con != NULL && pProcess->con->progress != NULL && 
		(percent - pProcess->last_percent) > 1.0f)
	{
		to_abort = (!pProcess->con->progress(pProcess->con, percent));
		pProcess->last_percent = percent;
	}

	/* if you want to abort the rendering, just return eiTRUE here. */
	return to_abort;
}

static void ei_renderer_clear_tile(
	eiRenderer *rend, 
	const eiInt left, const eiInt right, 
	const eiInt top, const eiInt bottom, 
	const eiHostID host)
{
	if (rend->con == NULL)
	{
		return;
	}

	rend->con->clear_tile(rend->con, left, right, top, bottom, host);
}

static void ei_renderer_update_tile(
	eiRenderer *rend, 
	const eiInt left, const eiInt right, 
	const eiInt top, const eiInt bottom, 
	const eiInt pos_i, const eiInt pos_j)
{
	eiInt				width;
	eiInt				height;
	eiFrameBufferCache	colorFrameBufferCache;
	eiFrameBufferCache	opacityFrameBufferCache;
	ei_array			frameBufferCaches;
	eiIntptr			i;

	if (rend->con == NULL)
	{
		return;
	}

	width = right - left;
	height = bottom - top;

	/* build frame buffer caches */
	ei_framebuffer_cache_init(
		&colorFrameBufferCache, 
		rend->db, 
		rend->colorFrameBuffer, 
		width, 
		height, 
		pos_i, 
		pos_j);

	ei_framebuffer_cache_init(
		&opacityFrameBufferCache, 
		rend->db, 
		rend->opacityFrameBuffer, 
		width, 
		height, 
		pos_i, 
		pos_j);

	ei_array_init(&frameBufferCaches, sizeof(eiFrameBufferCache));
	ei_array_resize(&frameBufferCaches, ei_data_array_size(rend->db, rend->frameBuffers));

	for (i = 0; i < ei_array_size(&frameBufferCaches); ++i)
	{
		eiTag	frameBuffer;

		frameBuffer = *((eiTag *)ei_data_array_read(rend->db, rend->frameBuffers, i));
		ei_data_array_end(rend->db, rend->frameBuffers, i);

		ei_framebuffer_cache_init(
			(eiFrameBufferCache *)ei_array_get(&frameBufferCaches, i), 
			rend->db, 
			frameBuffer, 
			width, 
			height, 
			pos_i, 
			pos_j);
	}

	rend->con->update_tile(
		rend->con, 
		&colorFrameBufferCache, 
		&opacityFrameBufferCache, 
		&frameBufferCaches, 
		left, 
		right, 
		top, 
		bottom);

	for (i = 0; i < ei_array_size(&frameBufferCaches); ++i)
	{
		ei_framebuffer_cache_exit(
			(eiFrameBufferCache *)ei_array_get(&frameBufferCaches, i));
	}
	ei_array_clear(&frameBufferCaches);

	ei_framebuffer_cache_exit(&opacityFrameBufferCache);
	ei_framebuffer_cache_exit(&colorFrameBufferCache);
}

static void ei_render_process_update(
	eiProcess *process, 
	const eiMessage *msg)
{
	eiRenderProcess *pProcess = (eiRenderProcess *)process;

	/* handle messages that we are interested in. */
	switch (msg->type)
	{
	case EI_MSG_BUCKET_STARTED:
		{
			if (pProcess->rend != NULL)
			{
				ei_renderer_clear_tile(pProcess->rend, 
					msg->bucket_started_params.rect.left, 
					msg->bucket_started_params.rect.right + 1, 
					msg->bucket_started_params.rect.top, 
					msg->bucket_started_params.rect.bottom + 1, 
					msg->bucket_started_params.host);
			}
		}
		break;

	case EI_MSG_BUCKET_FINISHED:
		{
			if (pProcess->rend != NULL)
			{
				ei_renderer_update_tile(pProcess->rend, 
					msg->bucket_finished_params.rect.left, 
					msg->bucket_finished_params.rect.right + 1, 
					msg->bucket_finished_params.rect.top, 
					msg->bucket_finished_params.rect.bottom + 1, 
					msg->bucket_finished_params.pos_i, 
					msg->bucket_finished_params.pos_j);
			}
		}
		break;

	default:
		/* other messages which we just have no interest, ignore them. */
		break;
	}
}

static void ei_render_process_init(
	eiRenderProcess *process, 
	eiRenderer *rend)
{
	process->base.one_job_started = ei_render_process_one_job_started;
	process->base.one_job_finished = ei_render_process_one_job_finished;
	process->base.one_worker_finished = ei_render_process_one_worker_finished;
	process->base.progress = ei_render_process_progress;
	process->base.update = ei_render_process_update;
	process->db = rend->db;
	process->con = rend->con;
	process->rend = rend;
	process->last_percent = 0.0f;
}

static void ei_render_process_exit(eiRenderProcess *process)
{
}

/** \brief The photon emission process. */
typedef struct eiPhotonProcess {
	eiProcess		base;
	eiDatabase		*db;
	eiConnection	*con;
	eiRenderer		*rend;
	eiScalar		last_percent;
} eiPhotonProcess;

static eiBool ei_photon_process_one_job_started(
	eiProcess *process, 
	const eiTag job, 
	const eiHostID hostId, 
	const eiThreadID threadId, 
	eiMessage *msg)
{
	/* indicate that we are not interested in this message. */
	return eiFALSE;
}

static eiBool ei_photon_process_one_job_finished(
	eiProcess *process, 
	const eiTag job, 
	const eiHostID hostId, 
	const eiThreadID threadId, 
	eiMessage *msg)
{
	/* indicate that we are not interested in this message. */
	return eiFALSE;
}

static eiBool ei_photon_process_one_worker_finished(
	eiProcess *process, 
	eiBaseWorker *pWorker, 
	eiMessage *msg)
{
	return eiFALSE;
}

static eiBool ei_photon_process_progress(
	eiProcess *process, 
	const eiScalar percent)
{
	eiPhotonProcess		*pProcess;
	eiBool				to_abort;

	pProcess = (eiPhotonProcess *)process;

	/* update the progress and check for abort */
	to_abort = eiFALSE;

	if (pProcess->con != NULL && pProcess->con->progress != NULL && 
		(percent - pProcess->last_percent) > 1.0f)
	{
		to_abort = (!pProcess->con->progress(pProcess->con, percent));
		pProcess->last_percent = percent;
	}

	/* if you want to abort the rendering, just return eiTRUE here. */
	return to_abort;
}

static void ei_photon_process_update(
	eiProcess *process, 
	const eiMessage *msg)
{
	/* we don't need to update client applications for photon emission */
}

static void ei_photon_process_init(
	eiPhotonProcess *process, 
	eiRenderer *rend)
{
	process->base.one_job_started = ei_photon_process_one_job_started;
	process->base.one_job_finished = ei_photon_process_one_job_finished;
	process->base.one_worker_finished = ei_photon_process_one_worker_finished;
	process->base.progress = ei_photon_process_progress;
	process->base.update = ei_photon_process_update;
	process->db = rend->db;
	process->con = rend->con;
	process->rend = rend;
	process->last_percent = 0.0f;
}

static void ei_photon_process_exit(eiPhotonProcess *process)
{
}

void ei_init_globals(eiGlobals *globals, eiDatabase *db)
{
	eiRayTracer		*rt;
	eiNodeSystem	*nodesys;

	globals->interfaces = (eiInterface *)ei_allocate(sizeof(eiInterface) * EI_INTERFACE_TYPE_COUNT);
	globals->num_interfaces = EI_INTERFACE_TYPE_COUNT;

	globals->interfaces[ EI_INTERFACE_TYPE_NONE ] = NULL;

	globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ] = (eiInterface)ei_allocate(sizeof(eiRayTracer));
	ei_rt_init((eiRayTracer *)globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ]);

	globals->interfaces[ EI_INTERFACE_TYPE_NODE_SYSTEM ] = (eiInterface)ei_allocate(sizeof(eiNodeSystem));
	ei_nodesys_init((eiNodeSystem *)globals->interfaces[ EI_INTERFACE_TYPE_NODE_SYSTEM ], 
		db);

	/* setup callbacks for the ray-tracer */
	rt = (eiRayTracer *)globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ];

	rt->get_prim_var = ei_get_prim_var;
	rt->procedural_intersect = ei_procedural_intersect;

	/* call ei_nodesys_register_creator to register 
	   all built-in creators of node objects here */
	nodesys = (eiNodeSystem *)globals->interfaces[ EI_INTERFACE_TYPE_NODE_SYSTEM ];

	ei_nodesys_register_creator(
		nodesys, 
		"camera", 
		ei_create_camera_node_object);
	ei_nodesys_register_creator(
		nodesys, 
		"instance", 
		ei_create_instance_node_object);
	ei_nodesys_register_creator(
		nodesys, 
		"instgroup", 
		ei_create_instgroup_node_object);
	ei_nodesys_register_creator(
		nodesys, 
		"light", 
		ei_create_light_node_object);
	ei_nodesys_register_creator(
		nodesys, 
		"material", 
		ei_create_material_node_object);
	ei_nodesys_register_creator(
		nodesys, 
		"texture", 
		ei_create_texture_node_object);
	ei_nodesys_register_creator(
		nodesys, 
		"options", 
		ei_create_options_node_object);
	ei_nodesys_register_creator(
		nodesys, 
		"poly", 
		ei_create_poly_object_node_object);
	ei_nodesys_register_creator(
		nodesys, 
		"hair", 
		ei_create_hair_object_node_object);

	/* setup global context for shader parsing */
	if (!ei_db_net_is_master(db))
	{
		ei_context(ei_create_server_context(globals));
	}
}

void ei_exit_globals(eiGlobals *globals, eiDatabase *db)
{
	if (globals->interfaces == NULL || globals->num_interfaces == 0)
	{
		return;
	}

	if (!ei_db_net_is_master(db))
	{
		ei_delete_server_context(ei_context(NULL));
	}

	ei_nodesys_exit((eiNodeSystem *)globals->interfaces[ EI_INTERFACE_TYPE_NODE_SYSTEM ]);
	eiCHECK_FREE(globals->interfaces[ EI_INTERFACE_TYPE_NODE_SYSTEM ]);

	ei_rt_exit((eiRayTracer *)globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ]);
	eiCHECK_FREE(globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ]);

	eiCHECK_FREE(globals->interfaces);
	globals->num_interfaces = 0;
}

void ei_init_tls(eiTLS *tls)
{
	ei_tls_allocate_interfaces(tls, EI_TLS_TYPE_COUNT);

	ei_tls_set_interface(tls, EI_TLS_TYPE_NONE, NULL);

	ei_tls_set_interface(tls, EI_TLS_TYPE_RAYTRACER, (eiInterface)ei_allocate(sizeof(eiRayTLS)));
	ei_ray_tls_init((eiRayTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_RAYTRACER), EI_MAX_BSP_DEPTH);

	ei_tls_set_interface(tls, EI_TLS_TYPE_GLOBILLUM, (eiInterface)ei_allocate(sizeof(eiGlobillumTLS)));
	ei_globillum_tls_init((eiGlobillumTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_GLOBILLUM));
}

void ei_exit_tls(eiTLS *tls)
{
	ei_globillum_tls_exit((eiGlobillumTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_GLOBILLUM));
	ei_tls_free_interface(tls, EI_TLS_TYPE_GLOBILLUM);

	ei_ray_tls_exit((eiRayTLS *)ei_tls_get_interface(tls, EI_TLS_TYPE_RAYTRACER));
	ei_tls_free_interface(tls, EI_TLS_TYPE_RAYTRACER);

	ei_tls_clear_interfaces(tls);
}

eiBool ei_set_scene_callback(eiGlobals *globals, eiDatabase *db, const eiTag scene_tag)
{
	eiRayTracer		*rt;

	/* get ray-tracer interface from globals */
	rt = (eiRayTracer *)globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ];
	eiDBG_ASSERT(rt != NULL);

	rt->scene_tag = scene_tag;

	/* begin editing to the ray-traceable scene */
	rt->db = db;

	rt->scene = (eiRayScene *)ei_db_access(db, rt->scene_tag);

	return eiTRUE;
}

eiBool ei_end_scene_callback(eiGlobals *globals, eiDatabase *db)
{
	eiRayTracer		*rt;

	/* get ray-tracer interface from globals */
	rt = (eiRayTracer *)globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ];
	eiDBG_ASSERT(rt != NULL);

	/* end editing to the ray-traceable scene */
	ei_db_end(db, rt->scene_tag);
	rt->scene = NULL;

	return eiTRUE;
}

eiBool ei_update_scene_callback(eiGlobals *globals, eiDatabase *db)
{
	eiRayTracer		*rt;

	/* get ray-tracer interface from globals */
	rt = (eiRayTracer *)globals->interfaces[ EI_INTERFACE_TYPE_RAYTRACER ];
	eiDBG_ASSERT(rt != NULL);

	/* re-read the ray-traceable scene */
	ei_db_end(db, rt->scene_tag);
	rt->scene = (eiRayScene *)ei_db_access(db, rt->scene_tag);

	return eiTRUE;
}

eiBool ei_link_callback(eiGlobals *globals, eiDatabase *db, const char *module_name)
{
	eiNodeSystem	*nodesys;
	eiPluginSystem	*plugsys;

	/* get node system interface from globals */
	nodesys = (eiNodeSystem *)globals->interfaces[ EI_INTERFACE_TYPE_NODE_SYSTEM ];
	eiDBG_ASSERT(nodesys != NULL);

	plugsys = ei_nodesys_plugin_system(nodesys);

	ei_plugsys_link(plugsys, module_name);

	return eiTRUE;
}

static void ei_renderer_verbose_print(
	const eiInt severity, 
	const char *message, 
	void *params)
{
	eiConnection *con = (eiConnection *)params;
	
	if (con != NULL && con->print != NULL)
	{
		con->print(con, severity, message);
	}
}

static void ei_renderer_init_stats(eiRenderer *rend)
{
}

static void print_cache_hit_rate(eiTLS *pTls, void *param)
{
	ei_info("thread %d cache hit rate: %f %%\n", ei_tls_get_thread_id(pTls), ei_tls_get_cache_hit_rate(pTls) * 100.0);
}

static void ei_renderer_finish_stats(eiRenderer *rend)
{
	ei_exec_traverse_tls(ei_db_executor(rend->db), print_cache_hit_rate, NULL);

	ei_info("db page file peak: %f MB\n", (eiGeoScalar)ei_db_pagefile_peak(rend->db) / (eiGeoScalar)(1024 * 1024));
	ei_info("db memory peak: %f MB\n", (eiGeoScalar)ei_db_mem_peak(rend->db) / (eiGeoScalar)(1024 * 1024));
	ei_info("db virtual memory peak: %f MB\n", (eiGeoScalar)ei_db_virtual_mem_peak(rend->db) / (eiGeoScalar)(1024 * 1024));
}

/** \brief Initialize frame buffers, calculate user output size. */
static void ei_renderer_init_framebuffers(
	eiRenderer *rend, 
	eiOptions *opt, 
	eiCamera *cam, 
	eiUint * const user_output_size)
{
	eiInt				region_width, region_height;
	eiOutputVariable	colorOutvar;
	eiOutputVariable	opacityOutvar;
	eiInt				num_outputs;
	eiInt				i;

	region_width = MIN(cam->res_x, cam->window_xmax) - MAX(0, cam->window_xmin);
	region_height = MIN(cam->res_y, cam->window_ymax) - MAX(0, cam->window_ymin);

	ei_framebuffer_map_init(&rend->frameBufferMap);

	/* add standard color output */
	rend->colorFrameBuffer = ei_create_framebuffer(
		rend->db, 
		"color", 
		EI_DATA_TYPE_VECTOR, 
		EI_SAMPLE_INFO_COLOR_OFFSET, 
		region_width, 
		region_height, 
		opt->bucket_size);

	strncpy(colorOutvar.name, "color", EI_MAX_PARAM_NAME_LEN - 1);
	colorOutvar.datatype = EI_DATA_TYPE_VECTOR;
	colorOutvar.framebuffer = rend->colorFrameBuffer;

	ei_framebuffer_map_add(
		&rend->frameBufferMap, 
		&colorOutvar);

	/* add standard opacity output per color channel */
	rend->opacityFrameBuffer = ei_create_framebuffer(
		rend->db, 
		"opacity", 
		EI_DATA_TYPE_VECTOR, 
		EI_SAMPLE_INFO_OPACITY_OFFSET, 
		region_width, 
		region_height, 
		opt->bucket_size);

	strncpy(opacityOutvar.name, "opacity", EI_MAX_PARAM_NAME_LEN - 1);
	opacityOutvar.datatype = EI_DATA_TYPE_VECTOR;
	opacityOutvar.framebuffer = rend->opacityFrameBuffer;

	ei_framebuffer_map_add(
		&rend->frameBufferMap, 
		&opacityOutvar);

	/* add user outputs */
	rend->frameBuffers = ei_create_data_array(rend->db, EI_DATA_TYPE_TAG);

	num_outputs = ei_data_array_size(rend->db, cam->outputs);

	for (i = 0; i < num_outputs; ++i)
	{
		eiOutput	*output;
		eiInt		num_output_variables;
		eiInt		j;

		output = (eiOutput *)ei_data_array_read(rend->db, cam->outputs, i);

		num_output_variables = ei_data_array_size(rend->db, output->variables);

		/* associate a frame buffer for each output variable */
		for (j = 0; j < num_output_variables; ++j)
		{
			eiOutputVariable	*outvar;

			outvar = (eiOutputVariable *)ei_data_array_write(rend->db, output->variables, j);

			outvar->framebuffer = ei_framebuffer_map_find(
				&rend->frameBufferMap, 
				outvar);

			/* create a frame buffer for the output variable requirement 
			   if it has not been created before */
			if (outvar->framebuffer == eiNULL_TAG)
			{
				eiFrameBuffer	*fb;

				outvar->framebuffer = ei_create_framebuffer(
					rend->db, 
					outvar->name, 
					outvar->datatype, 
					sizeof(eiSampleInfo) + (*user_output_size), 
					region_width, 
					region_height, 
					opt->bucket_size);

				ei_framebuffer_map_add(
					&rend->frameBufferMap, 
					outvar);

				/* increment the user output size */
				fb = (eiFrameBuffer *)ei_db_access(rend->db, outvar->framebuffer);
				(*user_output_size) += ei_db_type_size(rend->db, fb->m_type);
				ei_db_end(rend->db, outvar->framebuffer);

				ei_data_array_push_back(rend->db, rend->frameBuffers, &outvar->framebuffer);
			}

			ei_data_array_end(rend->db, output->variables, j);
		}

		ei_data_array_end(rend->db, cam->outputs, i);
	}
}

static void ei_renderer_delete_framebuffers(eiRenderer *rend)
{
	eiInt	numFrameBuffers;
	eiInt	i;

	numFrameBuffers = ei_data_array_size(rend->db, rend->frameBuffers);

	for (i = 0; i < numFrameBuffers; ++i)
	{
		eiTag	fb;

		fb = *((eiTag *)ei_data_array_read(rend->db, rend->frameBuffers, i));
		ei_data_array_end(rend->db, rend->frameBuffers, i);

		ei_delete_framebuffer(rend->db, fb);
	}
	ei_delete_data_array(rend->db, rend->frameBuffers);

	ei_delete_framebuffer(rend->db, rend->opacityFrameBuffer);
	ei_delete_framebuffer(rend->db, rend->colorFrameBuffer);

	ei_framebuffer_map_exit(&rend->frameBufferMap);
}

static void ei_renderer_init_light_instances(eiRenderer *rend)
{
	rend->lightInstances = ei_create_data_table(
		rend->db, EI_DATA_TYPE_LIGHT_INST, EI_LIGHT_INSTANCE_SLOT_SIZE);
}

static void ei_renderer_delete_light_instances(eiRenderer *rend)
{
	eiInt				numLightInstances;
	eiDataTableIterator	lightInstancesIter;
	eiInt				i;

	/* call destructor for all light instances */
	ei_data_table_begin(rend->db, rend->lightInstances, &lightInstancesIter);
	numLightInstances = lightInstancesIter.tab->item_count;

	for (i = 0; i < numLightInstances; ++i)
	{
		eiLightInstance		*inst;

		inst = (eiLightInstance *)ei_data_table_write(&lightInstancesIter, i);
		
		ei_light_instance_exit(inst);
	}

	ei_data_table_end(&lightInstancesIter);

	ei_delete_data_table(rend->db, rend->lightInstances);
}

static void ei_renderer_init_photon_maps(
	eiRenderer *rend, 
	eiOptions *opt)
{
	if (opt->caustic && opt->caustic_photons != 0)
	{
		rend->causticMap = ei_create_map(rend->db, EI_DATA_TYPE_PHOTON, opt->caustic_photons);
	}

	if (opt->globillum && opt->globillum_photons != 0)
	{
		rend->globillumMap = ei_create_map(rend->db, EI_DATA_TYPE_PHOTON, opt->globillum_photons);
	}
}

static void ei_renderer_delete_photon_maps(
	eiRenderer *rend)
{
	if (rend->causticMap != eiNULL_TAG)
	{
		ei_delete_map(rend->db, rend->causticMap);
		rend->causticMap = eiNULL_TAG;
	}
	if (rend->globillumMap != eiNULL_TAG)
	{
		ei_delete_map(rend->db, rend->globillumMap);
		rend->globillumMap = eiNULL_TAG;
	}
}

static void ei_renderer_init_irrad_cache(eiRenderer *rend)
{
	rend->irradCache = ei_create_map(rend->db, EI_DATA_TYPE_IRRADIANCE, eiMAX_INT);
}

typedef struct eiIrradianceProcParams {
	eiConnection	*con;
	eiCamera		*cam;
} eiIrradianceProcParams;

static eiBool ei_irradiance_proc(
	const eiMapNode *node, 
	void *param)
{
	static const eiVector	FGPointColor = { 0.0f, 0.0f, 1.0f };

	eiIrradianceProcParams	*params;
	eiVector				screen_pos;

	params = (eiIrradianceProcParams *)param;
	
	ei_camera_object_to_screen(params->cam, &screen_pos, &node->pos, &g_IdentityMatrix);

	params->con->draw_pixel(
		params->con, 
		(eiInt)screen_pos.x, 
		(eiInt)screen_pos.y, 
		&FGPointColor);

	return eiTRUE;
}

static void ei_renderer_delete_irrad_cache(
	eiRenderer *rend, 
	eiOptions *opt, 
	eiCamera *cam)
{
	/* visualize final gather points for diagnostics */
	if (opt->diagnostic_mode == EI_DIAGNOSTIC_MODE_FINALGATHER_POINTS)
	{
		eiIrradianceProcParams	params;

		params.cam = cam;
		params.con = rend->con;

		ei_map_traverse(
			rend->db, 
			rend->irradCache, 
			ei_irradiance_proc, 
			(void *)(&params));
	}

	ei_delete_map(rend->db, rend->irradCache);
	rend->irradCache = eiNULL_TAG;
}

/** \brief Transform all light instances. */
static void ei_renderer_transform_light_instances(
	eiRenderer *rend, 
	const eiMatrix *transform, 
	const eiMatrix *motion_transform)
{
	eiInt				numLightInstances;
	eiDataTableIterator	lightInstancesIter;
	eiInt				i;

	ei_data_table_begin(rend->db, rend->lightInstances, &lightInstancesIter);
	numLightInstances = lightInstancesIter.tab->item_count;

	for (i = 0; i < numLightInstances; ++i)
	{
		eiLightInstance		*inst;

		inst = (eiLightInstance *)ei_data_table_write(&lightInstancesIter, i);
		
		ei_light_instance_transform(inst, transform, motion_transform);
	}

	ei_data_table_end(&lightInstancesIter);
}

static void ei_renderer_precompute(eiRenderer *rend, eiOptions *opt, eiCamera *cam)
{
	eiInt	region_width, region_height;

	/* initialize buckets */
	region_width = MIN(cam->res_x, cam->window_xmax) - MAX(0, cam->window_xmin);
	region_height = MIN(cam->res_y, cam->window_ymax) - MAX(0, cam->window_ymin);

	rend->noXBuckets = region_width / opt->bucket_size;
	rend->noYBuckets = region_height / opt->bucket_size;
	if (rend->noXBuckets == 0) {
		rend->noXBuckets = 1;
	}
	if (rend->noYBuckets == 0) {
		rend->noYBuckets = 1;
	}
	rend->noXBuckets_1 = rend->noXBuckets - 1;
	rend->noYBuckets_1 = rend->noYBuckets - 1;
}

static void ei_renderer_create_buckets(
	eiRenderer *rend, 
	eiOptions *opt, 
	eiCamera *cam, 
	const eiTag opt_tag, 
	const eiTag cam_tag, 
	const eiUint user_output_size, 
	const eiInt pass_mode, 
	const eiScalar point_spacing)
{
	eiRect4i		brect;
	eiInt			brect_top, brect_left;
	eiInt			bucket_id = 0;
	eiTag			job_tag;
	eiBucketJob		*job;
	eiInt			i, j;

	ei_buffer_init(
		&rend->buckets, 
		sizeof(eiTag), 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL);
	ei_buffer_allocate(&rend->buckets, rend->noXBuckets, rend->noYBuckets);

	ei_array_init(&rend->passIrradBuffers, sizeof(eiTag));

	brect.left   = MAX(0, cam->window_xmin);
	brect.right  = brect.left + opt->bucket_size - 1;
	brect.top    = MAX(0, cam->window_ymin);
	brect.bottom = brect.top + opt->bucket_size - 1;

	for (i = 0; i < rend->noXBuckets_1; ++i)
	{
		brect.top    = MAX(0, cam->window_ymin);
	    brect.bottom = brect.top + opt->bucket_size - 1;

		for (j = 0; j < rend->noYBuckets_1; ++j)
		{
			job = (eiBucketJob *)ei_db_create(
				rend->db, 
				&job_tag, 
				EI_DATA_TYPE_JOB_BUCKET, 
				sizeof(eiBucketJob), 
				EI_DB_FLUSHABLE);

			job->pos_i = i;
			job->pos_j = j;
			ei_rect4i_copy(&job->rect, &brect);
			job->user_output_size = user_output_size;
			job->opt = opt_tag;
			job->cam = cam_tag;
			job->colorFrameBuffer = rend->colorFrameBuffer;
			job->opacityFrameBuffer = rend->opacityFrameBuffer;
			job->frameBuffers = rend->frameBuffers;
			job->lightInstances = rend->lightInstances;
			job->causticMap = rend->causticMap;
			job->globillumMap = rend->globillumMap;
			job->irradCache = rend->irradCache;
			job->pass_mode = pass_mode;
			job->point_spacing = point_spacing;
			job->passIrradBuffer = eiNULL_TAG;
			if (pass_mode == EI_PASS_FINALGATHER_INITIAL || 
				pass_mode == EI_PASS_FINALGATHER_REFINE)
			{
				job->passIrradBuffer = ei_create_data_array(rend->db, EI_DATA_TYPE_IRRADIANCE);
				ei_array_push_back(&rend->passIrradBuffers, &job->passIrradBuffer);
			}
			job->bucket_id = bucket_id ++;

			ei_db_end(rend->db, job_tag);

			ei_buffer_set(&rend->buckets, i, j, &job_tag);

			brect.top		+= opt->bucket_size;
		    brect.bottom	+= opt->bucket_size;
		}

		brect.left	+= opt->bucket_size;
		brect.right	+= opt->bucket_size;
	}

	brect_top		= brect.top;
	brect_left		= brect.left;
	brect.left		= MAX(0, cam->window_xmin);
	brect.right		= brect.left + opt->bucket_size - 1;
	brect.bottom	= MIN(cam->res_y, cam->window_ymax) - 1;

	for (i = 0; i < rend->noXBuckets_1; ++i)
	{
		job = (eiBucketJob *)ei_db_create(
			rend->db, 
			&job_tag, 
			EI_DATA_TYPE_JOB_BUCKET, 
			sizeof(eiBucketJob), 
			EI_DB_FLUSHABLE);

		job->pos_i = i;
		job->pos_j = rend->noYBuckets_1;
		ei_rect4i_copy(&job->rect, &brect);
		job->user_output_size = user_output_size;
		job->opt = opt_tag;
		job->cam = cam_tag;
		job->colorFrameBuffer = rend->colorFrameBuffer;
		job->opacityFrameBuffer = rend->opacityFrameBuffer;
		job->frameBuffers = rend->frameBuffers;
		job->lightInstances = rend->lightInstances;
		job->causticMap = rend->causticMap;
		job->globillumMap = rend->globillumMap;
		job->irradCache = rend->irradCache;
		job->pass_mode = pass_mode;
		job->point_spacing = point_spacing;
		job->passIrradBuffer = eiNULL_TAG;
		if (pass_mode == EI_PASS_FINALGATHER_INITIAL || 
			pass_mode == EI_PASS_FINALGATHER_REFINE)
		{
			job->passIrradBuffer = ei_create_data_array(rend->db, EI_DATA_TYPE_IRRADIANCE);
			ei_array_push_back(&rend->passIrradBuffers, &job->passIrradBuffer);
		}
		job->bucket_id = bucket_id ++;

		ei_db_end(rend->db, job_tag);

		ei_buffer_set(&rend->buckets, i, rend->noYBuckets_1, &job_tag);

		brect.left  += opt->bucket_size;
		brect.right += opt->bucket_size;
	}

	brect.top		= MAX(0, cam->window_ymin);
	brect.bottom	= brect.top + opt->bucket_size - 1;
	brect.left		= brect_left;
	brect.right		= MIN(cam->res_x, cam->window_xmax) - 1;

	for (j = 0; j < rend->noYBuckets - 1; ++j)
	{
		job = (eiBucketJob *)ei_db_create(
			rend->db, 
			&job_tag, 
			EI_DATA_TYPE_JOB_BUCKET, 
			sizeof(eiBucketJob), 
			EI_DB_FLUSHABLE);

		job->pos_i = rend->noXBuckets_1;
		job->pos_j = j;
		ei_rect4i_copy(&job->rect, &brect);
		job->user_output_size = user_output_size;
		job->opt = opt_tag;
		job->cam = cam_tag;
		job->colorFrameBuffer = rend->colorFrameBuffer;
		job->opacityFrameBuffer = rend->opacityFrameBuffer;
		job->frameBuffers = rend->frameBuffers;
		job->lightInstances = rend->lightInstances;
		job->causticMap = rend->causticMap;
		job->globillumMap = rend->globillumMap;
		job->irradCache = rend->irradCache;
		job->pass_mode = pass_mode;
		job->point_spacing = point_spacing;
		job->passIrradBuffer = eiNULL_TAG;
		if (pass_mode == EI_PASS_FINALGATHER_INITIAL || 
			pass_mode == EI_PASS_FINALGATHER_REFINE)
		{
			job->passIrradBuffer = ei_create_data_array(rend->db, EI_DATA_TYPE_IRRADIANCE);
			ei_array_push_back(&rend->passIrradBuffers, &job->passIrradBuffer);
		}
		job->bucket_id = bucket_id ++;

		ei_db_end(rend->db, job_tag);

		ei_buffer_set(&rend->buckets, rend->noXBuckets_1, j, &job_tag);

		brect.top		+= opt->bucket_size;
		brect.bottom	+= opt->bucket_size;
	}

	brect.top		= brect_top;
	brect.bottom	= MIN(cam->res_y, cam->window_ymax) - 1;
	brect.left		= brect_left;
	brect.right		= MIN(cam->res_x, cam->window_xmax) - 1;

	job = (eiBucketJob *)ei_db_create(
		rend->db, 
		&job_tag, 
		EI_DATA_TYPE_JOB_BUCKET, 
		sizeof(eiBucketJob), 
		EI_DB_FLUSHABLE);

	job->pos_i = rend->noXBuckets_1;
	job->pos_j = rend->noYBuckets_1;
	ei_rect4i_copy(&job->rect, &brect);
	job->user_output_size = user_output_size;
	job->opt = opt_tag;
	job->cam = cam_tag;
	job->colorFrameBuffer = rend->colorFrameBuffer;
	job->opacityFrameBuffer = rend->opacityFrameBuffer;
	job->frameBuffers = rend->frameBuffers;
	job->lightInstances = rend->lightInstances;
	job->causticMap = rend->causticMap;
	job->globillumMap = rend->globillumMap;
	job->irradCache = rend->irradCache;
	job->pass_mode = pass_mode;
	job->point_spacing = point_spacing;
	job->passIrradBuffer = eiNULL_TAG;
	if (pass_mode == EI_PASS_FINALGATHER_INITIAL || 
		pass_mode == EI_PASS_FINALGATHER_REFINE)
	{
		job->passIrradBuffer = ei_create_data_array(rend->db, EI_DATA_TYPE_IRRADIANCE);
		ei_array_push_back(&rend->passIrradBuffers, &job->passIrradBuffer);
	}
	job->bucket_id = bucket_id ++;

	ei_db_end(rend->db, job_tag);

	ei_buffer_set(&rend->buckets, rend->noXBuckets_1, rend->noYBuckets_1, &job_tag);

	sort_buckets(0, 0, rend->noXBuckets, rend->noYBuckets, &rend->buckets, rend->master);
}

static void ei_renderer_delete_buckets(eiRenderer *rend)
{
	eiIntptr	i;

	for (i = 0; i < ei_array_size(&rend->passIrradBuffers); ++i)
	{
		eiTag	passIrradBuffer;

		passIrradBuffer = *((eiTag *)ei_array_get(&rend->passIrradBuffers, i));

		if (passIrradBuffer == eiNULL_TAG)
		{
			continue;
		}

		ei_delete_data_array(rend->db, passIrradBuffer);
	}

	ei_array_clear(&rend->passIrradBuffers);
	ei_buffer_clear(&rend->buckets);
}

static void ei_renderer_output_images(
	eiRenderer *rend, 
	eiOptions *opt, 
	eiCamera *cam)
{
	eiPluginSystem	*plugsys;
	eiInt			num_outputs;
	eiInt			i;

	plugsys = ei_nodesys_plugin_system(rend->nodesys);

	num_outputs = ei_data_array_size(rend->db, cam->outputs);

	for (i = 0; i < num_outputs; ++i)
	{
		eiOutput		*output;
		eiImageWriter	*writer;

		output = (eiOutput *)ei_data_array_read(rend->db, cam->outputs, i);

		writer = ei_create_image_writer(
			plugsys, 
			output->fileformat, 
			output->filename);

		if (writer == NULL)
		{
			ei_data_array_end(rend->db, cam->outputs, i);
			continue;
		}

		switch (output->datatype)
		{
		case EI_IMG_DATA_RGB:
			{
				eiOutputVariable	*outvar;

				outvar = (eiOutputVariable *)ei_data_array_read(rend->db, output->variables, 0);
				
				if (outvar != NULL && outvar->framebuffer != eiNULL_TAG)
				{
					eiFrameBuffer	*fb;

					fb = (eiFrameBuffer *)ei_db_access(rend->db, outvar->framebuffer);

					if (fb != NULL)
					{
						writer->output_rgb(writer, rend->db, fb, opt, cam);
					}

					ei_db_end(rend->db, outvar->framebuffer);
				}

				ei_data_array_end(rend->db, output->variables, 0);
			}
			break;

		case EI_IMG_DATA_RGBA:
			{
				eiOutputVariable	*outvar1;
				eiOutputVariable	*outvar2;

				outvar1 = (eiOutputVariable *)ei_data_array_read(rend->db, output->variables, 0);
				outvar2 = (eiOutputVariable *)ei_data_array_read(rend->db, output->variables, 1);
				
				if (outvar1 != NULL && outvar1->framebuffer != eiNULL_TAG && 
					outvar2 != NULL && outvar2->framebuffer != eiNULL_TAG)
				{
					eiFrameBuffer	*fb1;
					eiFrameBuffer	*fb2;

					fb1 = (eiFrameBuffer *)ei_db_access(rend->db, outvar1->framebuffer);
					fb2 = (eiFrameBuffer *)ei_db_access(rend->db, outvar2->framebuffer);

					if (fb1 != NULL && fb2 != NULL)
					{
						writer->output_rgba(writer, rend->db, fb1, fb2, opt, cam);
					}

					ei_db_end(rend->db, outvar2->framebuffer);
					ei_db_end(rend->db, outvar1->framebuffer);
				}

				ei_data_array_end(rend->db, output->variables, 1);
				ei_data_array_end(rend->db, output->variables, 0);
			}
			break;

		case EI_IMG_DATA_RGBAZ:
			{
				eiOutputVariable	*outvar1;
				eiOutputVariable	*outvar2;
				eiOutputVariable	*outvar3;

				outvar1 = (eiOutputVariable *)ei_data_array_read(rend->db, output->variables, 0);
				outvar2 = (eiOutputVariable *)ei_data_array_read(rend->db, output->variables, 1);
				outvar3 = (eiOutputVariable *)ei_data_array_read(rend->db, output->variables, 2);
				
				if (outvar1 != NULL && outvar1->framebuffer != eiNULL_TAG && 
					outvar2 != NULL && outvar2->framebuffer != eiNULL_TAG && 
					outvar3 != NULL && outvar3->framebuffer != eiNULL_TAG)
				{
					eiFrameBuffer	*fb1;
					eiFrameBuffer	*fb2;
					eiFrameBuffer	*fb3;

					fb1 = (eiFrameBuffer *)ei_db_access(rend->db, outvar1->framebuffer);
					fb2 = (eiFrameBuffer *)ei_db_access(rend->db, outvar2->framebuffer);
					fb3 = (eiFrameBuffer *)ei_db_access(rend->db, outvar3->framebuffer);

					if (fb1 != NULL && fb2 != NULL && fb3 != NULL)
					{
						writer->output_rgbaz(writer, rend->db, fb1, fb2, fb3, opt, cam);
					}

					ei_db_end(rend->db, outvar3->framebuffer);
					ei_db_end(rend->db, outvar2->framebuffer);
					ei_db_end(rend->db, outvar1->framebuffer);
				}

				ei_data_array_end(rend->db, output->variables, 2);
				ei_data_array_end(rend->db, output->variables, 1);
				ei_data_array_end(rend->db, output->variables, 0);
			}
			break;

		default:
			/* error */
			break;
		}

		ei_delete_image_writer(plugsys, writer);
		ei_data_array_end(rend->db, cam->outputs, i);
	}
}

static eiBool scene_message_proc(eiDatabase *db, SOCKET sock, void *param)
{
	eiData		*pData;
	eiMessage	msg;
	eiMessage	inf;

	/* monitor the job execution and do any requested 
	   supplements for the server thread. */
	while (eiTRUE)
	{
		ei_msg_init(&msg);

		ei_net_recv(sock, (eiByte *)&msg, sizeof(eiMessage));

		switch (msg.type)
		{
		case EI_MSG_INF_JOB_FINISHED:
			{
				/* server has finished the job execution, quit this loop. */
				return msg.job_finished_params.result;
			}
			break;

		case EI_MSG_REQ_SEND_DATA:
			{
				/* server is requesting a chunk of data from manager,
				   we should send the data back to the server. */
				/* defer the data generation for balancing workload, generate it 
				   on the requesting host, and avoid recursive network messaging */
				if (msg.send_data_params.defer_init)
				{
					pData = ei_db_access_info_defer_init(db, msg.send_data_params.data);
				}
				else
				{
					/* some data types, for instance, texture tile with non-local mode, 
					   may require the data within data generator, in which we are 
					   supposed to send initialized data. */
					pData = ei_db_access_info(db, msg.send_data_params.data);
				}

				if (pData != NULL && pData->ptr != NULL && pData->size != 0)
				{
					/* first, let it know the current size of the data. */
					inf.type = EI_MSG_INF_DATA_INFO;
					inf.data_info_params.size = pData->size;
					inf.data_info_params.inited = (ei_atomic_read(&pData->flag) & EI_DB_INITED) ? eiTRUE : eiFALSE;

					ei_net_send(sock, (eiByte *)&inf, sizeof(eiMessage));

					/* now we can send the actual data content. */
					ei_net_send(sock, (eiByte *)pData->ptr, pData->size);
				}
				else
				{
					/* failed to lookup the data requested by server, 
					   currently treat it as fatal error. */
					ei_fatal("Failed to lookup data %d\n", msg.send_data_params.data);

					/* abort the rendering, cleanups will be done. */
					return eiFALSE;
				}

				ei_db_end(db, msg.send_data_params.data);
			}
			break;
		}
	}

	return eiTRUE;
}

static void ei_renderer_startup(eiRenderer *rend)
{
	char			cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char			config_filename[ EI_MAX_FILE_NAME_LEN ];
	eiConfig		config;
	eiMessage		req;
	eiIntptr		i;

	/* read host settings from configuration file */
	ei_get_current_directory(cur_dir);
	ei_append_filename(config_filename, cur_dir, "manager.ini");

	ei_config_init(&config);
	ei_config_load(&config, config_filename);

	/* general initialization steps for rendering manager. */
	rend->master = ei_create_master();
	rend->exec = ei_create_exec(EI_EXECUTOR_TYPE_MANAGER, g_InitTLS);
	ei_master_set_executor(rend->master, rend->exec);
	rend->db = ei_create_db(config.memlimit, 
		EI_DEFAULT_FILE_SIZE_LIMIT, 
		EI_DEFAULT_PURGE_RATE, rend->exec, rend->master);
	ei_master_set_database(rend->master, rend->db);

	/* set user data generators immediately for this renderer. */
	ei_db_data_gen_table(rend->db, &g_DataGenTable);

	/* add all rendering hosts for distributed rendering. */
	if (config.distributed)
	{
		for (i = 0; i < ei_array_size(&config.servers); ++i)
		{
			eiHostDesc	*host_desc;

			host_desc = (eiHostDesc *)ei_array_get(&config.servers, i);

			ei_master_add_host(rend->master, host_desc->host_name, host_desc->port_number);
		}
	}

	/* create workers for processing, connect to hosts. */
	ei_master_create_workers(rend->master, config.nthreads, config.distributed, g_InitTLS);

	/* create global object and set it to database. */
	/* must initialize global objects after the rendering threads 
	   have been created, since global objects might create some 
	   data in the multi-threaded database. */
	ei_globals_init(&rend->globals);

	if (g_InitGlobals != NULL)
	{
		g_InitGlobals(&rend->globals, rend->db);
	}
	else
	{
		ei_warning("Init globals is NULL.\n");
	}
	ei_db_globals(rend->db, &rend->globals);

	/* get ray-tracer interface from globals */
	rend->rt = (eiRayTracer *)rend->globals.interfaces[ EI_INTERFACE_TYPE_RAYTRACER ];
	/* get node system interface from globals */
	rend->nodesys = (eiNodeSystem *)rend->globals.interfaces[ EI_INTERFACE_TYPE_NODE_SYSTEM ];

	/* add search pathes for node system */
	for (i = 0; i < ei_array_size(&config.searchpaths); ++i)
	{
		eiSearchPath	*searchpath;

		searchpath = (eiSearchPath *)ei_array_get(&config.searchpaths, i);

		ei_plugsys_add_search_path(ei_nodesys_plugin_system(rend->nodesys), searchpath->path);
	}

	ei_config_exit(&config);

	/* install all built-in nodes here */
	ei_install_camera_node(rend->nodesys);
	ei_install_instance_node(rend->nodesys);
	ei_install_instgroup_node(rend->nodesys);
	ei_install_light_node(rend->nodesys);
	ei_install_material_node(rend->nodesys);
	ei_install_texture_node(rend->nodesys);
	ei_install_options_node(rend->nodesys);
	ei_install_poly_object_node(rend->nodesys);
	ei_install_hair_object_node(rend->nodesys);

	/* begin editing to the ray-traceable scene */
	ei_rt_scene(rend->rt, rend->db);

	/* synchronize the beginning of scene editing */
	req.type = EI_MSG_REQ_SET_SCENE;
	req.set_scene_params.scene_tag = rend->rt->scene_tag;

	ei_master_broadcast(rend->master, &req, 0, scene_message_proc, NULL);
}

static void ei_renderer_shutdown(eiRenderer *rend)
{
	eiMessage		req;

	/* synchronize the end of scene editing */
	req.type = EI_MSG_REQ_END_SCENE;

	ei_master_broadcast(rend->master, &req, 0, scene_message_proc, NULL);

	/* end editing to the ray-traceable scene */
	ei_rt_end_scene(rend->rt);

	/* garbage collection, release all data */
	ei_db_gc(rend->db);

	/* delete global object before database being deleted. */
	if (g_ExitGlobals != NULL)
	{
		g_ExitGlobals(&rend->globals, rend->db);
	}
	else
	{
		ei_warning("Exit globals is NULL.\n");
	}

	ei_globals_exit(&rend->globals);

	/* general shutdown steps for rendering manager. */
	ei_delete_db(rend->db);
	ei_delete_exec(rend->exec, g_ExitTLS);
	/* delete master after executor because executor may 
	   have some dependencies on master. */
	ei_delete_master(rend->master);
}

static void ei_renderer_run_process(eiRenderer *rend)
{
	eiRenderProcess		process;

	/* initialize rendering process. */
	ei_render_process_init(&process, rend);
	ei_master_set_process(rend->master, &process.base);

	/* done preprocessing, flush all dirty data with all hosts. */
	ei_db_flush_dirty(rend->db);

	/* run multi-threaded and distributed rendering. */
	ei_master_run_process(rend->master, eiFALSE);

	/* cleanup rendering process. */
	ei_render_process_exit(&process);
}

static void ei_renderer_run_photon_process(eiRenderer *rend)
{
	eiPhotonProcess		process;

	/* initialize photon process. */
	ei_photon_process_init(&process, rend);
	ei_master_set_process(rend->master, &process.base);

	/* done preprocessing, flush all dirty data with all hosts. */
	ei_db_flush_dirty(rend->db);

	/* run multi-threaded and distributed rendering. */
	ei_master_run_process(rend->master, eiFALSE);

	/* cleanup rendering process. */
	ei_photon_process_exit(&process);
}

typedef struct eiPhotonJobResult {
	/* the data array of photons */
	eiTag			photons;
	/* the count of shot photons */
	eiTag			count;
} eiPhotonJobResult;

static void merge_photon_job_result(
	eiDatabase *db, 
	const eiTag photonMap, 
	const eiTag photonBuffer)
{
	eiInt	numPhotons;

	if (photonMap == eiNULL_TAG)
	{
		return;
	}

	numPhotons = ei_data_array_size(db, photonBuffer);

	if (numPhotons <= 0)
	{
		return;
	}

	ei_map_store_points(
		db, 
		photonMap, 
		(eiByte *)ei_data_array_read(db, photonBuffer, 0), 
		numPhotons);

	ei_data_array_end(db, photonBuffer, 0);
}

static void ei_renderer_generate_photon_maps(
	eiRenderer *rend, 
	eiOptions *opt, 
	const eiTag opt_tag, 
	const eiTag cam_tag)
{
	eiTag					light_flux_histogram;
	eiScalar				acc = 0.0f;
	eiUint					caustic_count = 0;
	eiUint					globillum_count = 0;
	eiInt					halton_num = 0;
	eiDataTableIterator		light_inst_iter;
	eiInt					i;

	light_flux_histogram = ei_create_data_array(rend->db, EI_DATA_TYPE_LIGHT_FLUX);

	ei_data_table_begin(rend->db, rend->lightInstances, &light_inst_iter);

	/* build the light histogram */
	for (i = 0; i < light_inst_iter.tab->item_count; ++i)
	{
		eiLightInstance		*light_inst;
		eiScalar			max_flux;

		light_inst = (eiLightInstance *)ei_data_table_read(&light_inst_iter, i);

		max_flux = ei_light_instance_get_max_flux(light_inst, rend->db);
		
		if (max_flux > 0.0f)
		{
			eiLightFlux		light_flux;

			acc += max_flux;
			light_flux.energy = acc;
			light_flux.light_index = i;

			ei_data_array_push_back(rend->db, light_flux_histogram, &light_flux);
		}
	}

	ei_data_table_end(&light_inst_iter);

	if (rend->globillumMap != eiNULL_TAG)
	{
		eiUint				num_jobs;
		ei_array			photonBuffers;
		eiTag				job_tag = eiNULL_TAG;
		eiPhotonJob			*job;
		eiPhotonJobResult	result;
		eiInt				*count;
		eiUint				i;

		ei_info("Emitting global illumination photons...\n");

		num_jobs = opt->globillum_photons / NUM_PHOTONS_PER_JOB + (((opt->globillum_photons % NUM_PHOTONS_PER_JOB) != 0) ? 1 : 0);

		/* create result array */
		ei_array_init(&photonBuffers, sizeof(eiPhotonJobResult));

		/* create job result */
		result.photons = ei_create_data_array(rend->db, EI_DATA_TYPE_PHOTON);
		
		count = (eiInt *)ei_db_create(
			rend->db, 
			&result.count, 
			EI_DATA_TYPE_INT, 
			sizeof(eiInt), 
			EI_DB_FLUSHABLE);
		(*count) = 0;
		ei_db_end(rend->db, result.count);

		ei_array_push_back(&photonBuffers, &result);

		/* create job */
		job = (eiPhotonJob *)ei_db_create(
			rend->db, 
			&job_tag, 
			EI_DATA_TYPE_JOB_PHOTON, 
			sizeof(eiPhotonJob), 
			EI_DB_FLUSHABLE);

		job->opt = opt_tag;
		job->cam = cam_tag;
		job->photon_type = eiPHOTON_EMIT_GI;
		job->caustic_photons = eiNULL_TAG;
		job->globillum_photons = result.photons;
		job->count = result.count;
		job->light_flux_histogram = light_flux_histogram;
		job->acc = acc;
		job->num_target_photons = opt->globillum_photons - NUM_PHOTONS_PER_JOB * (num_jobs - 1);
		job->halton_num = halton_num;
		job->lightInstances = rend->lightInstances;

		ei_db_end(rend->db, job_tag);

		ei_master_add_job(rend->master, job_tag);
		halton_num += (opt->globillum_photons - NUM_PHOTONS_PER_JOB * (num_jobs - 1));

		for (i = 1; i < num_jobs; ++i)
		{
			/* create job result */
			result.photons = ei_create_data_array(rend->db, EI_DATA_TYPE_PHOTON);
		
			count = (eiInt *)ei_db_create(
				rend->db, 
				&result.count, 
				EI_DATA_TYPE_INT, 
				sizeof(eiInt), 
				EI_DB_FLUSHABLE);
			(*count) = 0;
			ei_db_end(rend->db, result.count);

			ei_array_push_back(&photonBuffers, &result);

			/* create job */
			job = (eiPhotonJob *)ei_db_create(
				rend->db, 
				&job_tag, 
				EI_DATA_TYPE_JOB_PHOTON, 
				sizeof(eiPhotonJob), 
				EI_DB_FLUSHABLE);

			job->opt = opt_tag;
			job->cam = cam_tag;
			job->photon_type = eiPHOTON_EMIT_GI;
			job->caustic_photons = eiNULL_TAG;
			job->globillum_photons = result.photons;
			job->count = result.count;
			job->light_flux_histogram = light_flux_histogram;
			job->acc = acc;
			job->num_target_photons = NUM_PHOTONS_PER_JOB;
			job->halton_num = halton_num;
			job->lightInstances = rend->lightInstances;

			ei_db_end(rend->db, job_tag);

			ei_master_add_job(rend->master, job_tag);
			halton_num += NUM_PHOTONS_PER_JOB;
		}

		/* shoot rays to generate global illumination photon map */
		ei_renderer_run_photon_process(rend);

		/* merge results and delete result array */
		for (i = 0; i < (eiUint)ei_array_size(&photonBuffers); ++i)
		{
			eiPhotonJobResult	*pResult;

			pResult = (eiPhotonJobResult *)ei_array_get(&photonBuffers, i);

			merge_photon_job_result(rend->db, rend->globillumMap, pResult->photons);

			count = (eiInt *)ei_db_access(rend->db, pResult->count);
			globillum_count += (*count);
			ei_db_end(rend->db, pResult->count);

			ei_delete_data_array(rend->db, pResult->photons);
			ei_db_delete(rend->db, pResult->count);
		}
		ei_array_clear(&photonBuffers);

		ei_photon_map_scale_photons(
			rend->db, 
			rend->globillumMap, 
			0, 
			acc / (eiScalar)globillum_count);
	}

	if (rend->causticMap != eiNULL_TAG)
	{
		eiUint				num_jobs;
		ei_array			photonBuffers;
		eiTag				job_tag = eiNULL_TAG;
		eiPhotonJob			*job;
		eiPhotonJobResult	result;
		eiInt				*count;
		eiUint				i;

		ei_info("Emitting caustic photons...\n");

		num_jobs = opt->caustic_photons / NUM_PHOTONS_PER_JOB + (((opt->caustic_photons % NUM_PHOTONS_PER_JOB) != 0) ? 1 : 0);

		/* create result array */
		ei_array_init(&photonBuffers, sizeof(eiPhotonJobResult));

		/* create job result */
		result.photons = ei_create_data_array(rend->db, EI_DATA_TYPE_PHOTON);
		
		count = (eiInt *)ei_db_create(
			rend->db, 
			&result.count, 
			EI_DATA_TYPE_INT, 
			sizeof(eiInt), 
			EI_DB_FLUSHABLE);
		(*count) = 0;
		ei_db_end(rend->db, result.count);

		ei_array_push_back(&photonBuffers, &result);

		/* create job */
		job = (eiPhotonJob *)ei_db_create(
			rend->db, 
			&job_tag, 
			EI_DATA_TYPE_JOB_PHOTON, 
			sizeof(eiPhotonJob), 
			EI_DB_FLUSHABLE);

		job->opt = opt_tag;
		job->cam = cam_tag;
		job->photon_type = eiPHOTON_EMIT_CAUSTIC;
		job->caustic_photons = result.photons;
		job->globillum_photons = eiNULL_TAG;
		job->count = result.count;
		job->light_flux_histogram = light_flux_histogram;
		job->acc = acc;
		job->num_target_photons = opt->caustic_photons - NUM_PHOTONS_PER_JOB * (num_jobs - 1);
		job->halton_num = halton_num;
		job->lightInstances = rend->lightInstances;

		ei_db_end(rend->db, job_tag);

		ei_master_add_job(rend->master, job_tag);
		halton_num += (opt->caustic_photons - NUM_PHOTONS_PER_JOB * (num_jobs - 1));

		for (i = 1; i < num_jobs; ++i)
		{
			/* create job result */
			result.photons = ei_create_data_array(rend->db, EI_DATA_TYPE_PHOTON);
		
			count = (eiInt *)ei_db_create(
				rend->db, 
				&result.count, 
				EI_DATA_TYPE_INT, 
				sizeof(eiInt), 
				EI_DB_FLUSHABLE);
			(*count) = 0;
			ei_db_end(rend->db, result.count);

			ei_array_push_back(&photonBuffers, &result);

			/* create job */
			job = (eiPhotonJob *)ei_db_create(
				rend->db, 
				&job_tag, 
				EI_DATA_TYPE_JOB_PHOTON, 
				sizeof(eiPhotonJob), 
				EI_DB_FLUSHABLE);

			job->opt = opt_tag;
			job->cam = cam_tag;
			job->photon_type = eiPHOTON_EMIT_CAUSTIC;
			job->caustic_photons = result.photons;
			job->globillum_photons = eiNULL_TAG;
			job->count = result.count;
			job->light_flux_histogram = light_flux_histogram;
			job->acc = acc;
			job->num_target_photons = NUM_PHOTONS_PER_JOB;
			job->halton_num = halton_num;
			job->lightInstances = rend->lightInstances;

			ei_db_end(rend->db, job_tag);

			ei_master_add_job(rend->master, job_tag);
			halton_num += NUM_PHOTONS_PER_JOB;
		}

		/* shoot rays to generate caustic photon map */
		ei_renderer_run_photon_process(rend);

		/* merge results and delete result array */
		for (i = 0; i < (eiUint)ei_array_size(&photonBuffers); ++i)
		{
			eiPhotonJobResult	*pResult;

			pResult = (eiPhotonJobResult *)ei_array_get(&photonBuffers, i);

			merge_photon_job_result(rend->db, rend->causticMap, pResult->photons);

			count = (eiInt *)ei_db_access(rend->db, pResult->count);
			caustic_count += (*count);
			ei_db_end(rend->db, pResult->count);

			ei_delete_data_array(rend->db, pResult->photons);
			ei_db_delete(rend->db, pResult->count);
		}
		ei_array_clear(&photonBuffers);

		ei_photon_map_scale_photons(
			rend->db, 
			rend->causticMap, 
			0, 
			acc / (eiScalar)caustic_count);
	}

	ei_delete_data_array(rend->db, light_flux_histogram);

	if (rend->globillumMap != eiNULL_TAG)
	{
		ei_info("Balancing global illumination photon map...\n");

		ei_map_balance(
			rend->db, 
			rend->globillumMap);

		if (opt->finalgather)
		{
			eiScalar	scene_diag;
			eiScalar	globillum_radius;

			scene_diag = ei_rt_scene_diag(rend->rt);

			globillum_radius = opt->globillum_radius;
			if (globillum_radius == 0.0f)
			{
				globillum_radius = scene_diag * 0.1f;
			}

			/* NOTICE: the cone kernel is hard-coded to 1.0f here */
			ei_photon_map_precompute_irrad(
				rend->db, 
				rend->globillumMap, 
				globillum_radius, 
				opt->globillum_samples, 
				EI_CAUSTIC_FILTER_BOX, 
				1.0f);
		}

		ei_info("Num stored global illumination photons: %d", ei_map_size(rend->db, rend->globillumMap));
		ei_info("Num shot global illumination photons: %d", globillum_count);
	}

	if (rend->causticMap != eiNULL_TAG)
	{
		ei_info("Balancing caustic photon map...\n");

		ei_map_balance(
			rend->db, 
			rend->causticMap);

		ei_info("Num stored caustic photons: %d", ei_map_size(rend->db, rend->causticMap));
		ei_info("Num shot caustic photons: %d", caustic_count);
	}
}

static void merge_irrad_buffer(eiRenderer *rend)
{
	eiIntptr	i;

	for (i = 0; i < ei_array_size(&rend->passIrradBuffers); ++i)
	{
		eiTag	passIrradBuffer;
		eiInt	numIrrads;

		passIrradBuffer = *((eiTag *)ei_array_get(&rend->passIrradBuffers, i));

		if (passIrradBuffer == eiNULL_TAG)
		{
			continue;
		}

		numIrrads = ei_data_array_size(rend->db, passIrradBuffer);

		if (numIrrads <= 0)
		{
			continue;
		}

		ei_map_store_points(
			rend->db, 
			rend->irradCache, 
			(eiByte *)ei_data_array_read(rend->db, passIrradBuffer, 0), 
			numIrrads);

		ei_data_array_end(rend->db, passIrradBuffer, 0);
	}
}

static void ei_renderer_generate_finalgather_points(
	eiRenderer *rend, 
	eiOptions *opt, 
	eiCamera *cam, 
	const eiTag opt_tag, 
	eiInstance *cam_inst, 
	const eiUint user_output_size)
{
	eiScalar	point_spacing;
	eiInt		pass_count;

	point_spacing = FINALGATHER_DENSITY_SCALE / opt->finalgather_density;

	do
	{
		/* create final gather buckets */
		ei_renderer_create_buckets(
			rend, opt, cam, opt_tag, cam_inst->element, user_output_size, 
			EI_PASS_FINALGATHER_INITIAL, point_spacing);

		/* shoot rays to generate final gather points */
		ei_renderer_run_process(rend);

		merge_irrad_buffer(rend);

		/* delete final gather buckets */
		ei_renderer_delete_buckets(rend);

		ei_map_balance(rend->db, rend->irradCache);
	}
	while (0);

	/* run progressive refinement for generating more FG points */
	pass_count = 1;

	while (point_spacing > FINALGATHER_MIN_SPACING && pass_count < FINALGATHER_MAX_PREPASS)
	{
		point_spacing *= 0.5f;

		/* create final gather buckets */
		ei_renderer_create_buckets(
			rend, opt, cam, opt_tag, cam_inst->element, user_output_size, 
			EI_PASS_FINALGATHER_REFINE, point_spacing);

		/* shoot rays to generate final gather points */
		ei_renderer_run_process(rend);

		merge_irrad_buffer(rend);

		/* delete final gather buckets */
		ei_renderer_delete_buckets(rend);

		ei_map_balance(rend->db, rend->irradCache);

		++ pass_count;
	}
}

/** \brief Construct the renderer, most default 
 * values are set here. */
static void ei_renderer_init(eiRenderer *rend)
{
	memset(rend, 0, sizeof(eiRenderer));

	rend->con = ei_get_default_connection();

	/* set global callbacks */
	g_InitGlobals = ei_init_globals;
	g_ExitGlobals = ei_exit_globals;
	g_InitTLS = ei_init_tls;
	g_ExitTLS = ei_exit_tls;
	g_SetSceneCallback = ei_set_scene_callback;
	g_EndSceneCallback = ei_end_scene_callback;
	g_UpdateSceneCallback = ei_update_scene_callback;
	g_LinkCallback = ei_link_callback;

	g_DataGenTable.data_gens = (eiDataGen *)ei_allocate(sizeof(eiDataGen) * EI_DATA_TYPE_COUNT);
	g_DataGenTable.num_data_gens = EI_DATA_TYPE_COUNT;

	/* initialize the libraries we are linking to */
	ei_core_init();
	ei_api_init();

	ei_renderer_startup(rend);

	ei_attr_init(&rend->default_attr);

	rend->causticMap = eiNULL_TAG;
	rend->globillumMap = eiNULL_TAG;
	rend->irradCache = eiNULL_TAG;
}

/** \brief Destruct the renderer. */
static void ei_renderer_exit(eiRenderer *rend)
{
	ei_attr_exit(&rend->default_attr, rend->db);

	ei_renderer_shutdown(rend);

	/* exit the libraries we are linking to */
	ei_api_exit();
	ei_core_exit();

	eiCHECK_FREE(g_DataGenTable.data_gens);
	g_DataGenTable.num_data_gens = 0;
}

eiRenderer *ei_create_renderer()
{
	eiRenderer *rend = (eiRenderer *)ei_allocate(sizeof(eiRenderer));

	ei_renderer_init(rend);

	return rend;
}

void ei_delete_renderer(eiRenderer *rend)
{
	if (rend == NULL)
	{
		eiASSERT(0);
		return;
	}

	ei_renderer_exit(rend);

	eiCHECK_FREE(rend);
}

void ei_renderer_set_connection(
	eiRenderer *rend, 
	eiConnection *con)
{
	rend->con = con;
}

eiNodeSystem * const ei_renderer_node_system(eiRenderer *rend)
{
	return rend->nodesys;
}

void ei_renderer_render(
	eiRenderer *rend, 
	const eiTag root_instgrp_tag, 
	const eiTag cam_inst_tag, 
	const eiTag opt_tag)
{
	eiInt			hours, minutes;
	eiScalar		seconds;
	eiTimer			timer;
	eiTimer			local_timer;
	eiOptions		*opt;
	eiInstance		*cam_inst;
	eiCamera		*cam;
	eiUint			user_output_size;
	eiRayOptions	*ray_opt;
	eiRayCamera		*ray_cam;
	eiMessage		req;
	eiBool			need_irrad_cache;

	/* override verbosity callback */
	ei_verbose_callback(ei_renderer_verbose_print, (void *)rend->con);

	ei_renderer_init_stats(rend);
	ei_info("\n");
	ei_info("Start rendering...\n");

	ei_timer_reset(&timer);
	ei_timer_start(&timer);

	/* begin accessing global options */
	opt = (eiOptions *)ei_db_access(rend->db, opt_tag);

	/* begin accessing camera instance and camera */
	cam_inst = (eiInstance *)ei_db_access(rend->db, cam_inst_tag);
	cam = (eiCamera *)ei_db_access(rend->db, cam_inst->element);

	/* build frame buffers */
	user_output_size = 0;

	ei_renderer_init_framebuffers(rend, opt, cam, &user_output_size);

	/* build light instances */
	ei_renderer_init_light_instances(rend);
	
	/* pre-processing the scene */
	ei_info("Pre-processing...\n");

	ei_timer_reset(&local_timer);
	ei_timer_start(&local_timer);

	/* traverse the scene graph and update all instances, merge 
	   down DAG attributes, create any instance that is not created */
	ei_attr_set_defaults(&rend->default_attr, opt, rend->db);

	ei_scene_update_instances(
		rend->master, 
		rend->con, 
		rend->nodesys, 
		cam_inst->element, 
		ei_rt_scene_root(rend->rt), 
		rend->lightInstances, 
		root_instgrp_tag, 
		&rend->default_attr, 
		&g_IdentityMatrix, 
		&g_IdentityMatrix, 
		NULL);

	/* edit the ray-traceable options */
	ray_opt = ei_rt_options(rend->rt);
	ray_opt->acceleration = opt->acceleration;
	ray_opt->bsp_size = opt->bsp_size;
	ray_opt->bsp_depth = opt->bsp_depth;
	ei_rt_end_options(rend->rt);

	/* edit the ray-traceable camera */
	ray_cam = ei_rt_camera(rend->rt);
	movm(&ray_cam->camera_to_world, &cam->camera_to_world);
	movm(&ray_cam->motion_camera_to_world, &cam->motion_camera_to_world);
	ei_rt_end_camera(rend->rt);

	/* synchronize the update of scene editing */
	req.type = EI_MSG_REQ_UPDATE_SCENE;

	ei_master_broadcast(rend->master, &req, 0, scene_message_proc, NULL);

	/* precompute parameters, ei_scene_update_instance must be called before this, 
	   because this function depends on some precomputed camera parameters. */
	ei_renderer_precompute(rend, opt, cam);

	ei_renderer_transform_light_instances(
		rend, &cam->world_to_camera, &cam->motion_world_to_camera);

	ei_timer_stop(&local_timer);
	ei_timer_format(&local_timer, &hours, &minutes, &seconds);
	ei_info("Finished pre-processing.\n");
	ei_info("Elapsed time: %d hours %d minutes %f seconds.\n", 
		hours, minutes, seconds);

	ei_info("Building initial acceleration structures...\n");

	ei_timer_reset(&local_timer);
	ei_timer_start(&local_timer);

	ei_rt_tracing(rend->rt);

	ei_timer_stop(&local_timer);
	ei_timer_format(&local_timer, &hours, &minutes, &seconds);
	ei_info("Finished building initial acceleration structures.\n");
	ei_info("Elapsed time: %d hours %d minutes %f seconds.\n", 
		hours, minutes, seconds);

	/* run photon emission pass to generate photon maps */
	if (opt->caustic || opt->globillum)
	{
		/* allocate global photon maps */
		ei_renderer_init_photon_maps(rend, opt);

		ei_info("Generating photon maps...\n");

		ei_timer_reset(&local_timer);
		ei_timer_start(&local_timer);

		ei_renderer_generate_photon_maps(
			rend, 
			opt, 
			opt_tag, 
			cam_inst->element);

		ei_timer_stop(&local_timer);
		ei_timer_format(&local_timer, &hours, &minutes, &seconds);
		ei_info("Finished generating photon maps.\n");
		ei_info("Elapsed time: %d hours %d minutes %f seconds.\n", 
			hours, minutes, seconds);
	}

	/* run prepasses to generate final gather points */
	/* only build irradiance cache when users want interpolation */
	need_irrad_cache = (opt->finalgather && opt->finalgather_samples > 0);

	if (need_irrad_cache)
	{
		/* allocate global irradiance cache */
		ei_renderer_init_irrad_cache(rend);

		ei_info("Generating final gather points...\n");

		ei_timer_reset(&local_timer);
		ei_timer_start(&local_timer);

		ei_renderer_generate_finalgather_points(
			rend, 
			opt, 
			cam, 
			opt_tag, 
			cam_inst, 
			user_output_size);

		ei_timer_stop(&local_timer);
		ei_timer_format(&local_timer, &hours, &minutes, &seconds);
		ei_info("Finished generating final gather points.\n");
		ei_info("Elapsed time: %d hours %d minutes %f seconds.\n", 
			hours, minutes, seconds);
	}

	/* buckets must be created after frame buffers */
	ei_renderer_create_buckets(
		rend, opt, cam, opt_tag, cam_inst->element, user_output_size, 
		EI_PASS_FRAME, 0.0f);

	/* shoot rays to render this frame */
	ei_info("Rendering the frame...\n");

	ei_timer_reset(&local_timer);
	ei_timer_start(&local_timer);

	/* run the main rendering process */
	ei_renderer_run_process(rend);

	ei_timer_stop(&local_timer);
	ei_timer_format(&local_timer, &hours, &minutes, &seconds);
	ei_info("Finished frame rendering.\n");
	ei_info("Elapsed time: %d hours %d minutes %f seconds.\n", 
		hours, minutes, seconds);

	ei_rt_end_tracing(rend->rt);

	/* delete irradiance cache */
	if (need_irrad_cache)
	{
		ei_renderer_delete_irrad_cache(rend, opt, cam);
	}

	/* delete photon maps */
	if (opt->caustic || opt->globillum)
	{
		ei_renderer_delete_photon_maps(rend);
	}

	/* delete buckets */
	ei_renderer_delete_buckets(rend);

	/* delete light instances */
	ei_renderer_delete_light_instances(rend);

	/* output images */
	ei_info("Outputing images...\n");

	ei_timer_reset(&local_timer);
	ei_timer_start(&local_timer);

	ei_renderer_output_images(rend, opt, cam);

	ei_timer_stop(&local_timer);
	ei_timer_format(&local_timer, &hours, &minutes, &seconds);
	ei_info("Finished outputing images.\n");
	ei_info("Elapsed time: %d hours %d minutes %f seconds.\n", 
		hours, minutes, seconds);

	/* delete frame buffers */
	ei_renderer_delete_framebuffers(rend);

	/* end accessing camera instance and camera */
	ei_db_end(rend->db, cam_inst->element);
	ei_db_end(rend->db, cam_inst_tag);

	/* end accessing global options */
	ei_db_end(rend->db, opt_tag);

	ei_timer_stop(&timer);
	ei_timer_format(&timer, &hours, &minutes, &seconds);
	ei_info("Finished rendering.\n");
	ei_info("Elapsed time: %d hours %d minutes %f seconds.\n", 
		hours, minutes, seconds);

	ei_renderer_finish_stats(rend);

	/* clear verbosity callback */
	ei_verbose_callback(NULL, NULL);
}
