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

#include <eiCORE/ei_message.h>
#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_assert.h>

void ei_msg_init(eiMessage *msg)
{
	msg->type = EI_MSG_UNKNOWN;
}

void ei_msg_set(eiMessage *msg, const eiInt type)
{
	msg->type = type;
}

void ei_byteswap_msg(eiMessage * const msg)
{
	switch (msg->type)
	{
	case EI_MSG_REQ_DISCONNECT:
		{
			ei_byteswap_msg_req_disconnect(&msg->disconnect_params);
		}
		break;

	case EI_MSG_REQ_CREATE_THREADS:
		{
			ei_byteswap_msg_req_create_threads(&msg->create_threads_params);
		}
		break;

	case EI_MSG_REQ_LINK:
		{
			ei_byteswap_msg_req_link(&msg->link_params);
		}
		break;

	case EI_MSG_REQ_SET_SCENE:
		{
			ei_byteswap_msg_req_set_scene(&msg->set_scene_params);
		}
		break;

	case EI_MSG_REQ_END_SCENE:
		{
			ei_byteswap_msg_req_end_scene(&msg->end_scene_params);
		}
		break;

	case EI_MSG_REQ_UPDATE_SCENE:
		{
			ei_byteswap_msg_req_update_scene(&msg->update_scene_params);
		}
		break;

	case EI_MSG_REQ_ALLOCATE_TAG:
		{
			ei_byteswap_msg_req_allocate_tag(&msg->allocate_tag_params);
		}
		break;

	case EI_MSG_REQ_PROCESS_JOB:
		{
			ei_byteswap_msg_req_process_job(&msg->process_job_params);
		}
		break;

	case EI_MSG_REQ_CREATE_DATA:
		{
			ei_byteswap_msg_req_create_data(&msg->create_data_params);
		}
		break;

	case EI_MSG_REQ_DELETE_DATA:
		{
			ei_byteswap_msg_req_delete_data(&msg->delete_data_params);
		}
		break;

	case EI_MSG_REQ_SEND_DATA:
		{
			ei_byteswap_msg_req_send_data(&msg->send_data_params);
		}
		break;

	case EI_MSG_REQ_FLUSH_DATA:
		{
			ei_byteswap_msg_req_flush_data(&msg->flush_data_params);
		}
		break;

	case EI_MSG_REQ_CHECK_ABORT:
		{
			ei_byteswap_msg_req_check_abort(&msg->check_abort_params);
		}
		break;

	case EI_MSG_REQ_STEP_PROGRESS:
		{
			ei_byteswap_msg_req_step_progress(&msg->step_progress_params);
		}
		break;

	case EI_MSG_INF_GENERIC:
		{
			ei_byteswap_msg_inf_generic(&msg->generic_params);
		}
		break;
	
	case EI_MSG_INF_HOST_ALLOCATED:
		{
			ei_byteswap_msg_inf_host_allocated(&msg->host_allocated_params);
		}
		break;

	case EI_MSG_INF_HOST_AUTHORIZED:
		{
			ei_byteswap_msg_inf_host_authorized(&msg->host_authorized_params);
		}
		break;

	case EI_MSG_INF_TAG_ALLOCATED:
		{
			ei_byteswap_msg_inf_tag_allocated(&msg->tag_allocated_params);
		}
		break;

	case EI_MSG_INF_DATA_GENERATED:
		{
			ei_byteswap_msg_inf_data_generated(&msg->data_generated_params);
		}
		break;

	case EI_MSG_INF_THREAD_CREATED:
		{
			ei_byteswap_msg_inf_thread_created(&msg->thread_created_params);
		}
		break;

	case EI_MSG_INF_JOB_FINISHED:
		{
			ei_byteswap_msg_inf_job_finished(&msg->job_finished_params);
		}
		break;

	case EI_MSG_INF_IS_ABORTED:
		{
			ei_byteswap_msg_inf_is_aborted(&msg->is_aborted_params);
		}
		break;

	case EI_MSG_INF_DATA_INFO:
		{
			ei_byteswap_msg_inf_data_info(&msg->data_info_params);
		}
		break;

	default:
		/* other messages do not need byte-swap, just return. */
		return;
	}

	ei_byteswap_int(&msg->type);
}

void ei_byteswap_msg_req_disconnect(eiMsgReqDisconnectParams * const params)
{
}

void ei_byteswap_msg_req_create_threads(eiMsgReqCreateThreadsParams * const params)
{
}

void ei_byteswap_msg_req_link(eiMsgReqLinkParams * const params)
{
}

void ei_byteswap_msg_req_set_scene(eiMsgReqSetSceneParams * const params)
{
	ei_byteswap_int(&params->scene_tag);
}

void ei_byteswap_msg_req_end_scene(eiMsgReqEndSceneParams * const params)
{
}

void ei_byteswap_msg_req_update_scene(eiMsgReqUpdateSceneParams * const params)
{
}

void ei_byteswap_msg_req_allocate_tag(eiMsgReqAllocateTagParams * const params)
{
	ei_byteswap_int(&params->host);
}

void ei_byteswap_msg_req_process_job(eiMsgReqProcessJobParams * const params)
{
	ei_byteswap_int(&params->job);
}

void ei_byteswap_msg_req_create_data(eiMsgReqCreateDataParams * const params)
{
	ei_byteswap_int(&params->type);
	ei_byteswap_int(&params->size);
	ei_byteswap_int(&params->flag);
	ei_byteswap_int(&params->data_tag);
	ei_byteswap_int(&params->host);
}

void ei_byteswap_msg_req_delete_data(eiMsgReqDeleteDataParams * const params)
{
	ei_byteswap_int(&params->data);
	ei_byteswap_int(&params->host);
}

void ei_byteswap_msg_req_send_data(eiMsgReqSendDataParams * const params)
{
	ei_byteswap_int(&params->data);
	ei_byteswap_int(&params->defer_init);
}

void ei_byteswap_msg_req_flush_data(eiMsgReqFlushDataParams * const params)
{
	ei_byteswap_int(&params->data);
	ei_byteswap_int(&params->host);
}

void ei_byteswap_msg_req_check_abort(eiMsgReqCheckAbortParams * const params)
{
}

void ei_byteswap_msg_req_step_progress(eiMsgReqStepProgressParams * const params)
{
	ei_byteswap_int(&params->count);
}

void ei_byteswap_msg_inf_generic(eiMsgInfGenericParams * const params)
{
	ei_byteswap_int(&params->result);
}

void ei_byteswap_msg_inf_host_allocated(eiMsgInfHostAllocatedParams * const params)
{
	ei_byteswap_int(&params->checksum1);
	ei_byteswap_int(&params->host);
	ei_byteswap_int(&params->mgr_endian);
}

void ei_byteswap_msg_inf_host_authorized(eiMsgInfHostAuthorizedParams * const params)
{
	ei_byteswap_int(&params->checksum2);
	ei_byteswap_int(&params->result);
	ei_byteswap_int(&params->need_byteswap);
}

void ei_byteswap_msg_inf_tag_allocated(eiMsgInfTagAllocatedParams * const params)
{
	ei_byteswap_int(&params->tag);
}

void ei_byteswap_msg_inf_data_generated(eiMsgInfDataGeneratedParams * const params)
{
	ei_byteswap_int(&params->data);
	ei_byteswap_int(&params->host);
}

void ei_byteswap_msg_inf_thread_created(eiMsgInfThreadCreatedParams * const params)
{
	ei_byteswap_int(&params->num_threads);
}

void ei_byteswap_msg_inf_job_finished(eiMsgInfJobFinishedParams * const params)
{
	ei_byteswap_int(&params->result);
}

void ei_byteswap_msg_inf_is_aborted(eiMsgInfIsAbortedParams * const params)
{
	ei_byteswap_int(&params->abort);
}

void ei_byteswap_msg_inf_data_info(eiMsgInfDataInfoParams * const params)
{
	ei_byteswap_int(&params->size);
	ei_byteswap_int(&params->inited);
}
