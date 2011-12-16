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
 
#ifndef EI_MGR_WORKER_H
#define EI_MGR_WORKER_H

/** \brief This file contains workers for the rendering manager.
 * \file ei_mgr_worker.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

/* forward declarations */
typedef struct eiWorker			eiWorker;
typedef struct eiLocalWorker	eiLocalWorker;
typedef struct eiRemoteWorker	eiRemoteWorker;

/** \brief Get the host ID where jobs are executed. */
typedef eiHostID (*ei_worker_get_job_host_id)(eiWorker *worker);
/** \brief Execute a job by its tag. */
typedef eiInt (*ei_worker_execute)(eiWorker *worker, const eiTag job);

eiLocalWorker *ei_create_local_worker();
void ei_delete_local_worker(eiBaseWorker *base_worker);

eiRemoteWorker *ei_create_remote_worker(eiHost *pHost, const eiThreadID remoteThreadId);
void ei_delete_remote_worker(eiBaseWorker *base_worker);

eiBool ei_remote_worker_connect(eiRemoteWorker *worker);
eiBool ei_remote_worker_disconnect(eiRemoteWorker *worker);

#ifdef __cplusplus
}
#endif

#endif
 