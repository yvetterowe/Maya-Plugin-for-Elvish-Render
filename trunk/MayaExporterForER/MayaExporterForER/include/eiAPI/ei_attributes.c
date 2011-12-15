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

#include <eiAPI/ei_attributes.h>
#include <eiCORE/ei_assert.h>

void ei_attr_init(eiAttributes *attr)
{
	ei_attr_set_nulls(attr);
}

void ei_attr_exit(eiAttributes *attr, eiDatabase *db)
{
	if (attr->approx != eiNULL_TAG)
	{
		ei_db_delete(db, attr->approx);
		attr->approx = eiNULL_TAG;
	}
}

void ei_attr_set_nulls(eiAttributes *attr)
{
	attr->flags = 0xFFFF;
	attr->face = eiMAX_SHORT;
	attr->approx = eiNULL_TAG;
	attr->max_displace = eiMAX_SCALAR;
	attr->min_samples = eiMAX_SHORT;
	attr->max_samples = eiMAX_SHORT;
	attr->material_list = eiNULL_TAG;
}

void ei_attr_set_approx(eiAttributes *attr, const eiApprox *src, eiDatabase *db)
{
	eiApprox	*dst;

	if (attr->approx == eiNULL_TAG)
	{
		dst = (eiApprox *)ei_db_create(
			db, 
			&attr->approx, 
			EI_DATA_TYPE_APPROX, 
			sizeof(eiApprox), 
			EI_DB_FLUSHABLE);
	}
	else
	{
		dst = (eiApprox *)ei_db_access(db, attr->approx);
	}

	ei_approx_copy(dst, src);

	ei_db_end(db, attr->approx);
}

void ei_attr_set_defaults(eiAttributes *attr, const eiOptions *opt, eiDatabase *db)
{
	eiApprox	*opt_approx;

	attr->flags = 0xFFFF;
	ei_attr_set_flag(attr, EI_ATTR_MOTION_BLUR, opt->motion);
	ei_attr_set_flag(attr, EI_ATTR_DISPLACE, opt->displace);
	/* the face statement controls face culling. */
	attr->face = opt->face;
	opt_approx = (eiApprox *)ei_db_access(db, opt->approx);
	ei_attr_set_approx(attr, opt_approx, db);
	ei_db_end(db, opt->approx);
	/* The max displace statement specifies the maximum allowed displacement 
	   applied to object control points in local object space in normal direction. */
	attr->max_displace = opt->max_displace;
	attr->min_samples = opt->min_samples;
	attr->max_samples = opt->max_samples;
	attr->material_list = eiNULL_TAG;
}

void ei_attr_copy(eiAttributes *dst, const eiAttributes *src)
{
	memcpy(dst, src, sizeof(eiAttributes));
}

eiInt ei_attr_compare(const eiAttributes *attr1, const eiAttributes *attr2)
{
	return memcmp(attr1, attr2, sizeof(eiAttributes));
}

void ei_attr_inherit(eiAttributes *attr, const eiAttributes *parent)
{
/* if an option has been turned off in parent, 
   turn it off in children forcibly */
#define INHERIT_FLAGS(a)	{\
								attr->##a &= parent->##a;\
							}

#define INHERIT_INT(a)		if (attr->##a == eiMAX_INT) {\
								attr->##a = parent->##a;\
							}

#define INHERIT_SHORT(a)	if (attr->##a == eiMAX_SHORT) {\
								attr->##a = parent->##a;\
							}

#define INHERIT_SCALAR(a)	if (attr->##a == eiMAX_SCALAR) {\
								attr->##a = parent->##a;\
							}

	INHERIT_FLAGS(flags);
	INHERIT_SHORT(face);
	INHERIT_SCALAR(max_displace);
	INHERIT_SHORT(min_samples);
	INHERIT_SHORT(max_samples);
	INHERIT_INT(material_list);

	if (attr->approx == eiNULL_TAG)
	{
		attr->approx = parent->approx;
	}
}

void ei_attr_set_flag(eiAttributes *attr, const eiInt flag, const eiBool value)
{
	if (value)
	{
		attr->flags |= flag;
	}
	else
	{
		attr->flags &= ~flag;
	}
}

eiBool ei_attr_get_flag(const eiAttributes *attr, const eiInt flag)
{
	return ((attr->flags & flag) != 0);
}

void ei_byteswap_attr(eiAttributes *attr)
{
	ei_byteswap_short(&attr->flags);
	ei_byteswap_short(&attr->face);
	ei_byteswap_int(&attr->approx);
	ei_byteswap_scalar(&attr->max_displace);
	ei_byteswap_short(&attr->min_samples);
	ei_byteswap_short(&attr->max_samples);
	ei_byteswap_int(&attr->material_list);
}
