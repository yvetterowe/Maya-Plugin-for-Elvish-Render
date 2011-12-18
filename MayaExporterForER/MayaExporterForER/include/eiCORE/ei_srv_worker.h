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
 
#ifndef EI_SRV_WORKER_H
#define EI_SRV_WORKER_H

/** \brief This file contains workers for the rendering server.
 * \file ei_srv_worker.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eiServerWorker		eiServerWorker;

eiServerWorker *ei_create_server_worker(const eiUshort port_number, 
										const eiInt max_num_clients, 
										const eiBool need_byteswap);
void ei_delete_server_worker(eiBaseWorker *base_worker);

eiBool ei_server_worker_connect(eiServerWorker *worker);
eiBool ei_server_worker_disconnect(eiServerWorker *worker);

#ifdef __cplusplus
}
#endif

#endif
 