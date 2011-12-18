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

#include <eiAPI/ei_shaderx.h>

LENS(simple_dof)

	PARAM(scalar, fstop);
	PARAM(scalar, fplane);

	void parameters(int pid)
	{
		DECLARE_SCALAR(fstop, 1.0f);
		DECLARE_SCALAR(fplane, 1.0f);
	}

	void make_ray(
		point & org, vector & dir, const vector & ray_dir, 
		const scalar dof_dx0, const scalar dof_dy0, const scalar dx, const scalar dy, 
		const scalar fstop, const scalar fplane)
	{
		scalar dof_dx, dof_dy;

		ei_concentric_sample_disk(
			&dof_dx, &dof_dy, 
			fmodf(dof_dx0 + dx, 1.0f), 
			fmodf(dof_dy0 + dy, 1.0f));

		org = point(
			dof_dx * fstop * 0.5f, 
			dof_dy * fstop * 0.5f, 
			0.0f);

		dir = normalize(ray_dir * fplane - org);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		// we don't support depth-of-field in orthogonal projection
		if (get_state()->cam->focal == eiMAX_SCALAR)
		{
			trace_eye(get_state()->result, point(0.0f, 0.0f, 0.0f), normalize(I()));
			return;
		}

		geoscalar samples[2];
		sample(samples, NULL, 2, NULL);
		scalar dof_dx0 = (scalar)samples[0];
		scalar dof_dy0 = (scalar)samples[1];

		point org;
		vector dir;
		eiSampleInfo *sub_c = new_sample();

		make_ray(org, dir, I(), dof_dx0, dof_dy0, 0.2932f, 0.8249f, fstop(), fplane());
		reset_sample(sub_c);
		trace_eye(sub_c, org, dir);
		add_sample(get_state()->result, sub_c);

		make_ray(org, dir, I(), dof_dx0, dof_dy0, 0.7976f, 0.7234f, fstop(), fplane());
		reset_sample(sub_c);
		trace_eye(sub_c, org, dir);
		add_sample(get_state()->result, sub_c);

		make_ray(org, dir, I(), dof_dx0, dof_dy0, 0.7076f, 0.1742f, fstop(), fplane());
		reset_sample(sub_c);
		trace_eye(sub_c, org, dir);
		add_sample(get_state()->result, sub_c);

		make_ray(org, dir, I(), dof_dx0, dof_dy0, 0.2026f, 0.2831f, fstop(), fplane());
		reset_sample(sub_c);
		trace_eye(sub_c, org, dir);
		add_sample(get_state()->result, sub_c);
		
		mul_sample(get_state()->result, 1.0f / 4.0f);
		delete_sample(sub_c);
	}

END(simple_dof)
