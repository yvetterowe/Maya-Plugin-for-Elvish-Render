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

#include <eiAPI/ei_approx.h>
#include <eiCORE/ei_assert.h>

#define EI_DEFAULT_MAX_GRID_SIZE		4096
#define EI_DEFAULT_MOTION_FACTOR		16.0f

void ei_approx_set_defaults(eiApprox *approx)
{
	approx->method = EI_APPROX_METHOD_REGULAR;
	approx->any = eiFALSE;
	approx->view_dep = eiFALSE;
	approx->args[0] = 0.0f;
	approx->args[1] = 0.0f;
	approx->args[2] = 0.0f;
	approx->args[3] = 0.0f;
	approx->sharp = 0.0f;
	approx->min_subdiv = 0;
	approx->max_subdiv = 5;
	approx->max_grid_size = EI_DEFAULT_MAX_GRID_SIZE;
	approx->motion_factor = EI_DEFAULT_MOTION_FACTOR;
}

void ei_approx_copy(eiApprox *dst, const eiApprox *src)
{
	memcpy(dst, src, sizeof(eiApprox));
}

eiBool ei_approx_is_enabled(const eiApprox *approx)
{
	if (approx->method == EI_APPROX_METHOD_REGULAR)
	{
		if (approx->args[EI_APPROX_U] == 0.0f)
		{
			return eiFALSE;
		}
		else
		{
			return eiTRUE;
		}
	}
	else if (approx->method == EI_APPROX_METHOD_LENGTH)
	{
		if (approx->args[EI_APPROX_LENGTH] == 0.0f)
		{
			return eiFALSE;
		}
		else
		{
			return eiTRUE;
		}
	}

	return eiFALSE;
}

eiBool ei_approx_is_view_dep(const eiApprox *approx)
{
	/* regular approximation is always view-independent */
	if (approx->method == EI_APPROX_METHOD_REGULAR)
	{
		return eiFALSE;
	}

	return approx->view_dep;
}

void byteswap_approx(eiDatabase *db, void *data, const eiUint size)
{
	eiApprox *approx = (eiApprox *)data;

	ei_byteswap_short(&approx->method);
	ei_byteswap_scalar(&approx->args[0]);
	ei_byteswap_scalar(&approx->args[1]);
	ei_byteswap_scalar(&approx->args[2]);
	ei_byteswap_scalar(&approx->args[3]);
	ei_byteswap_scalar(&approx->sharp);
	ei_byteswap_int(&approx->min_subdiv);
	ei_byteswap_int(&approx->max_subdiv);
	ei_byteswap_int(&approx->max_grid_size);
	ei_byteswap_scalar(&approx->motion_factor);
}
