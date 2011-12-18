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

#include <eiAPI/ei_scenemgr.h>
#include <eiAPI/ei_element.h>
#include <eiAPI/ei_raytracer.h>
#include <eiCORE/ei_assert.h>

/** \brief The object splitting process. */
typedef struct eiSplitProcess {
	eiProcess		base;
	eiConnection	*con;
	eiScalar		last_percent;
} eiSplitProcess;

static eiBool ei_split_process_one_job_started(
	eiProcess *process, 
	const eiTag job, 
	const eiHostID hostId, 
	const eiThreadID threadId, 
	eiMessage *msg)
{
	/* indicate that we are not interested in this message. */
	return eiFALSE;
}

static eiBool ei_split_process_one_job_finished(
	eiProcess *process, 
	const eiTag job, 
	const eiHostID hostId, 
	const eiThreadID threadId, 
	eiMessage *msg)
{
	/* indicate that we are not interested in this message. */
	return eiFALSE;
}

static eiBool ei_split_process_one_worker_finished(
	eiProcess *process, 
	eiBaseWorker *pWorker, 
	eiMessage *msg)
{
	return eiFALSE;
}

static eiBool ei_split_process_progress(
	eiProcess *process, 
	const eiScalar percent)
{
	eiSplitProcess		*pProcess;
	eiBool				to_abort;

	pProcess = (eiSplitProcess *)process;

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

static void ei_split_process_update(
	eiProcess *process, 
	const eiMessage *msg)
{
	/* we don't need to update client applications for object splitting */
}

static void ei_split_process_init(
	eiSplitProcess *process, 
	eiConnection *con)
{
	process->base.one_job_started = ei_split_process_one_job_started;
	process->base.one_job_finished = ei_split_process_one_job_finished;
	process->base.one_worker_finished = ei_split_process_one_worker_finished;
	process->base.progress = ei_split_process_progress;
	process->base.update = ei_split_process_update;
	process->con = con;
	process->last_percent = 0.0f;
}

static void ei_split_process_exit(eiSplitProcess *process)
{
}

static eiInt schedule_job(const eiTag job, void *param)
{
	eiMaster	*master;

	master = (eiMaster *)param;

	return ei_master_add_job(master, job);
}

void ei_scene_update_instances(
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
	eiNode *instancer)
{
	eiInstgroup			*root_instgrp;
	eiElement			*element;
	eiObjectRepCache	cache;
	eiRayTracer			*rt;

	/* initialize the object representation cache */
	ei_object_rep_cache_init(&cache, nodesys->m_db, cam_tag, light_insts);

	root_instgrp = (eiInstgroup *)ei_db_access(nodesys->m_db, root_instgrp_tag);

	element = (eiElement *)root_instgrp->node.object;

	/* traverse the scene DAG to update instances and schedule tessellation jobs */
	element->update_instance(nodesys, &cache, (eiNode *)root_instgrp, attr, transform, motion_transform, instancer);

	ei_db_end(nodesys->m_db, root_instgrp_tag);

	/* get ray-tracer interface */
	rt = (eiRayTracer *)ei_db_globals_interface(
		nodesys->m_db, 
		EI_INTERFACE_TYPE_RAYTRACER);

	/* release references to object representations */
	/* this must be done before deleting object representations, to make 
	   sure all unreferenced objects will be deleted later. */
	ei_rt_instances_unref_objects(rt, scene_tag);

	/* traverse all collected objects, delete all unreferenced object representations 
	   to lower the memory usage before the execution of tessellation jobs */
	ei_object_rep_cache_clear_unref_object_reps(&cache, nodesys->m_db);

	/* delete old ray-traceable object instances from the ray-tracer to 
	   lower the memory usage before the execution of tessellation jobs. */
	/* this must be done after deleting object representations, because 
	   while deleting object representations, the information of object 
	   instances will still be used as key. */
	ei_rt_remove_instances(rt, scene_tag);

	/* execute all previously scheduled tessellation jobs */
	if (ei_rt_scene_root(rt) == scene_tag)
	{
		eiSplitProcess	process;

		/* add all tessellation jobs to global job queue */
		ei_job_queue_traverse(cache.object_rep_jobs, schedule_job, (void *)master);

		/* initialize splitting process. */
		ei_split_process_init(&process, con);
		ei_master_set_process(master, &process.base);

		/* we are updating the root scene, use multi-threaded job processing. 
		   we don't need to flush dirty data to other hosts since currently 
		   we don't distribute tessellation jobs to remote hosts. */
		ei_master_run_process(master, eiTRUE);

		/* cleanup splitting process. */
		ei_split_process_exit(&process);
	}
	else
	{
		/* we are updating a sub-scene, which means we are already in mult-threaded 
		   processing, so use current thread to process tessellation jobs */
		ei_db_execute_jobs_on_current_thread(
			nodesys->m_db, 
			cache.object_rep_jobs, 
			cache.object_rep_jobs);
	}

	/* set new ray-traceable object instances to the ray-tracer */
	ei_rt_set_instances(rt, scene_tag, cache.object_instances);
	/* clear the object instances from cache, they are now managed by 
	   the ray-tracer */
	cache.object_instances = eiNULL_TAG;

	/* cleanup the object representation cache */
	ei_object_rep_cache_exit(&cache, nodesys->m_db);
}
