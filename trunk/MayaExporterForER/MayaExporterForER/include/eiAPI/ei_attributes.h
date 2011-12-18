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
 
#ifndef EI_ATTRIBUTES_H
#define EI_ATTRIBUTES_H

/** \brief The attributes of instances.
 * \file ei_attributes.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_options.h>
#include <eiAPI/ei_approx.h>
#include <eiCORE/ei_dataflow.h>

#ifdef __cplusplus
extern "C" {
#endif

/* visible to all ray types */
#define EI_ATTR_VISIBLE					(1 << 0)
/* visible to secondary(reflection or refraction) rays */
#define EI_ATTR_VISIBLE_SECONDARY		(1 << 1)
/* visible to shadow rays(cast shadows) */
#define EI_ATTR_CAST_SHADOW				(1 << 2)
/* whether this object receives shadows */
#define EI_ATTR_RECV_SHADOW				(1 << 3)
/* visible to reflection rays(cast reflection) */
#define EI_ATTR_CAST_REFLECTION			(1 << 4)
/* whether this object receives reflections */
#define EI_ATTR_RECV_REFLECTION			(1 << 5)
/* visible to refraction rays(cast refraction) */
#define EI_ATTR_CAST_REFRACTION			(1 << 6)
/* whether this object receives refractions */
#define EI_ATTR_RECV_REFRACTION			(1 << 7)
/* visible to transparency rays(cast transparency) */
#define EI_ATTR_CAST_TRANSPARENCY		(1 << 8)
/* whether this object receives transparency */
#define EI_ATTR_RECV_TRANSPARENCY		(1 << 9)
/* enable/disable motion blur */
#define EI_ATTR_MOTION_BLUR				(1 << 10)
/* enable/disable displacement mapping */
#define EI_ATTR_DISPLACE				(1 << 11)
/* visible to caustic photon rays(cast caustics) */
#define EI_ATTR_CAST_CAUSTIC			(1 << 12)
/* whether this object receives caustics */
#define EI_ATTR_RECV_CAUSTIC			(1 << 13)
/* visible to global illumination photon rays(cast global illumination) */
#define EI_ATTR_CAST_GLOBILLUM			(1 << 14)
/* whether this object receives global illumination */
#define EI_ATTR_RECV_GLOBILLUM			(1 << 15)

/** \brief The attributes of instances */
struct eiAttributes {
	/* attribute flags */
	eiShort					flags;
	eiShort					face;
	eiTag					approx;
	eiScalar				max_displace;
	eiShort					min_samples;
	eiShort					max_samples;
	/* a data array of tags, each tag points to a material */
	eiTag					material_list;
};

void ei_attr_init(eiAttributes *attr);
void ei_attr_exit(eiAttributes *attr, eiDatabase *db);

/** \brief Set all attributes to nulls. */
eiAPI void ei_attr_set_nulls(eiAttributes *attr);

/** \brief Set approximation for this attribute. */
void ei_attr_set_approx(eiAttributes *attr, const eiApprox *src, eiDatabase *db);

/** \brief Set all attributes to default settings, inherit some 
 * attributes from global options. */
void ei_attr_set_defaults(eiAttributes *attr, const eiOptions *opt, eiDatabase *db);

/** \brief Copy the attributes. */
void ei_attr_copy(eiAttributes *dst, const eiAttributes *src);
/** \brief Compare two attributes. */
eiInt ei_attr_compare(const eiAttributes *attr1, const eiAttributes *attr2);

/** \brief Inherit from parent attributes, if an attribute is not given (null) 
 * by the child, the corresponding value from its parent will be used. */
void ei_attr_inherit(eiAttributes *attr, const eiAttributes *parent);

void ei_attr_set_flag(eiAttributes *attr, const eiInt flag, const eiBool value);
eiBool ei_attr_get_flag(const eiAttributes *attr, const eiInt flag);

/* for internal use only */
void ei_byteswap_attr(eiAttributes *attr);

#ifdef __cplusplus
}
#endif

#endif
