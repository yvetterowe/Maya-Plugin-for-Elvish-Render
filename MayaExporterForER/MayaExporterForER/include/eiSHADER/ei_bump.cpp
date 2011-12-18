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

SURFACE(bump)

	PARAM(eiTag, shader);
	PARAM(scalar, factor);

	void parameters(int pid)
	{
		DECLARE_TAG(shader, eiNULL_TAG);
		DECLARE_SCALAR(factor, 1.0f);
	}

	void init()
	{
	}

	void exit()
	{
	}

	void main()
	{
		if (shader() == eiNULL_TAG)
		{
			return;
		}

		eiVector4	C, Cx, Cy;
		scalar		Dudx, Dudy, Dvdx, Dvdy;

		Dxy(u, Dudx, Dudy);
		Dxy(v, Dvdx, Dvdy);

		call_shader(&C, shader(), NULL);

		u() += Dudx;
		v() += Dvdx;
		call_shader(&Cx, shader(), NULL);
		u() -= Dudx;
		v() -= Dvdx;

		u() += Dudy;
		v() += Dvdy;
		call_shader(&Cy, shader(), NULL);
		u() -= Dudy;
		v() -= Dvdy;

		scalar s = average(&C.xyz);
		scalar sx = average(&Cx.xyz);
		scalar sy = average(&Cy.xyz);

		N() = normalize(N() + factor() * (dPdx() * (sx - s) + dPdy() * (sy - s)));
	}

END(bump)
