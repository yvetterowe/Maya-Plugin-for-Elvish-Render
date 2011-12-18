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
 
#ifndef EI_SHADER_H
#define EI_SHADER_H

/** \brief The header for users of the shading language interface, 
 * for consistence, please do not change this file.
 * \file ei_shader.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_nodesys.h>
#include <eiAPI/ei_state.h>

#ifdef __cplusplus
extern "C" {
#endif

/* exporting */
#ifdef _MSC_VER
#	define eiSHADER_API			__declspec(dllexport)
#	define eiSHADER_EXTERN		extern __declspec(dllexport)
#elif __GNUC__ >= 4
#	define eiSHADER_API			__attribute__((visibility("default")))
#	define eiSHADER_EXTERN		extern
#else
#	define eiSHADER_API		
#	define eiSHADER_EXTERN		extern
#endif

/** \brief The shader types */
typedef enum eiShaderType {
	eiSHADER_TYPE_NONE = 0, 
	eiSHADER_LENS, 
	eiSHADER_SURFACE, 
	eiSHADER_LIGHT, 
	eiSHADER_SHADOW, 
	eiSHADER_ENVIRONMENT, 
	eiSHADER_VOLUME, 
	eiSHADER_TEXTURE, 
	eiSHADER_DISPLACE, 
	eiSHADER_IMAGER, 
	eiSHADER_TYPE_COUNT, 
} eiShaderType;

/* forward declarations */
typedef struct eiShader				eiShader;

typedef eiBool (*ei_shader_main_func)(
	eiShader *shader, 
	eiVector4 * const result, 
	eiState * const state, 
	void *arg);

/** \brief The base class for all shader classes. */
struct eiShader {
	/* the base node object */
	eiNodeObject			base;
	/* the main function */
	ei_shader_main_func		main;
	/* the size of the shader class */
	eiUint					size;
	/* the pointer for holding current state */
	eiState					*state;
};

/** \brief Initialize the base shader. */
eiAPI void ei_shader_init(eiShader *shader);
/** \brief Cleanup the base shader. */
eiAPI void ei_shader_exit(eiShader *shader);

/** \brief Call a shader by tag. the return value of the 
 * shader call will be returned. the shader can be either 
 * one shader or a list of shaders. */
eiAPI eiBool ei_call_shader(
	eiVector4 * const result, 
	eiState * const state, 
	const eiTag shader, 
	void *arg);

/** \brief Evaluate the value of a shader parameter. */
eiAPI void *ei_eval(eiState *state, const eiIndex param_index);
/** \brief Evaluate the value of a shader parameter without 
 * affecting the shader cache. */
eiAPI void *ei_call(eiState *state, const eiIndex param_index);
/** \brief Evaluate the derivatives of a shader parameter along 
 * surface parametric coordinates without affecting the shader cache. */
eiAPI void *ei_call_uv(eiState *state, const eiIndex param_index, eiByte * const dXdu, eiByte * const dXdv);
/** \brief Evaluate the derivatives of a shader parameter along 
 * raster coordinates without affecting the shader cache. */
eiAPI void *ei_call_xy(eiState *state, const eiIndex param_index, eiByte * const dXdx, eiByte * const dXdy);

/** \brief Sample is an array with dimension members that the new sample 
 * point will be stored into. instance_number is the current sample number, 
 * it must point to an integer that is initialized to 0 before the first 
 * loop iteration. if there is no loop, a null pointer may be passed. 
 * dimension is the dimension of the sample. n must point to an integer 
 * that specifies the number of samples to take. if only a single sample 
 * is taken, n may be a null pointer. the body of ei_sample loop 
 * may not write to "sample" or "n" or vary "dimension" between any 
 * subsequent calls in the same loop, because ei_sample computes 
 * the next sample incrementally from the previous.
 * 
 * taking a single sample looks like:
 * 
 * double sample[2];
 * 
 * sample(sample, NULL, 2, NULL);
 * 
 * adaptive sampling looks like:
 * 
 * double sample[2];
 * int instance_number = 0;
 * 
 * while (sample(sample, &instance_number, 2, NULL))
 * {
 *     if (enough samples taken)
 *     {
 *         break;
 *     }
 * }
 * 
 * adaptive sampling is indicated by passing a null pointer as the number 
 * of samples n. the while loop is then controlled by the sample call, 
 * which always returns TRUE in adaptive sampling mode.
 * 
 * when the number of samples is known in advance, it looks like:
 * 
 * const uint samples = 16;
 * double sample[2];
 * int instance_number = 0;
 * 
 * while (sample(sample, &instance_number, 2, &samples))
 * {
 * }
 * 
 * sample returns TRUE for samples iterations of the loop, until finally 
 * FALSE is returned to terminate the loop. */
eiAPI eiBool ei_sample(
	eiState * const state, 
	eiGeoScalar *samples, 
	eiUint *instance_number, 
	const eiUint dimension, 
	const eiUint *n);

/** \brief The sample operations which should only be used in lens shader. */
eiAPI eiSampleInfo *ei_new_sample(eiState * const state);
eiAPI void ei_reset_sample(eiState * const state, eiSampleInfo * const sample);
eiAPI void ei_add_sample(eiState * const state, eiSampleInfo * const c1, eiSampleInfo * const c2);
eiAPI void ei_mul_sample(eiState * const state, eiSampleInfo * const c, const eiScalar a);
eiAPI void ei_delete_sample(eiState * const state, eiSampleInfo * const sample);

/** \brief illuminance is used to control the integration of incoming light over a 
 * hemisphere centered on a surface in a surface shader.
 * The illuminance statement controls integration of a procedural reflectance 
 * model over the incoming light. Inside the illuminance block two additional 
 * variables are defined: Cl or light color, and L or light direction. The 
 * vector L points towards the light source, but may not be normalized. The 
 * arguments to the illuminance statement specify a three-dimensional solid 
 * cone.
 * This cone is specified by giving its center line, and the angle between 
 * the side and the axis in radians.
 * Light shaders can specify "categories" to which they belong by ei_add_category 
 * calls. When the illuminance statement contains an optional string parameter 
 * category, the loop will only consider lights for which the category is among 
 * those listed in its category list. If the illuminance category begins with 
 * a - character, then only lights not containing that category will be 
 * considered. */
eiAPI eiBool ei_illuminance(
	eiState * const state, 
	const eiVector *position, 
	const eiVector *axis, 
	const eiScalar angle, 
	const char *category);
/** \brief Take a sample from current lightsource. */
eiAPI eiBool ei_sample_light(eiState * const state);
/** \brief Test if two colors are close enough to be declaimed as converged. */
eiAPI eiBool ei_converged(eiState * const state, const eiVector *c1, const eiVector *c2);

/** \brief The illuminate and solar statements are inverses of the illuminance 
 * statement. They control the casting of light in different directions. 
 * The point variable L corresponding to a particular light direction is 
 * available inside this block. This vector points outward from the light 
 * source. The color variable Cl corresponds to the color in this direction 
 * and should be set. Like the illuminance statements, illuminate and solar 
 * statements can not be nested.
 * The illuminate statement is used to specify light cast by local light 
 * sources. The arguments to the illuminate statement specify a 
 * three-dimensional solid cone. */
eiAPI eiBool ei_illuminate(
	eiState * const state, 
	const eiVector *position, 
	const eiVector *axis, 
	const eiScalar angle);
/** \brief The solar statement is used to specify light cast by distant light sources. 
 * The arguments to the solar statement specify a three-dimensional cone. 
 * Light is cast from distant directions inside this cone. Since this cone 
 * specifies only directions, its apex need not be given. */
eiAPI eiBool ei_solar(
	eiState * const state, 
	const eiVector *axis, 
	const eiScalar angle);

/** \brief Calculate ray bias, for internal use only. */
eiScalar calc_bias(
	const eiVector *N, 
	const eiVector *dir, 
	const eiScalar bias, 
	const eiScalar bias_scale);

/** \brief Call volume shaders for all instances in current volume list */
void ei_call_current_volume_list(
	eiNodeSystem *nodesys, 
	eiVector4 * const result, 
	eiState * const state, 
	void *arg);

eiAPI void ei_trace_eye(
	eiSampleInfo * const color, 
	const eiVector *src, 
	const eiVector *dir, 
	eiState * const state);

eiAPI void ei_trace_shadow(
	eiVector * const color, 
	const eiVector *src, 
	const eiVector *dir, 
	eiState * const state);

eiAPI void ei_trace_reflection(
	eiVector * const color, 
	const eiVector *dir, 
	eiState * const state);

eiAPI void ei_trace_transmission(
	eiVector * const color, 
	const eiVector *dir, 
	eiState * const state);

eiAPI void ei_trace_transparent(
	eiVector * const color, 
	eiState * const state);

eiAPI void ei_trace_environment(
	eiVector * const color, 
	eiState * const state, 
	const eiVector *dir);

eiAPI eiBool ei_trace_probe(
	const eiVector *src, 
	const eiVector *dir, 
	const eiScalar max_dist, 
	eiState * const state);

/** \brief Compute irradiance of indirect illumination at a shading point. */
eiAPI void ei_compute_irradiance(eiVector * const irrad, eiState * const state);

/** \brief Compute the cosine between half-angle vector and wi */
eiAPI eiScalar ei_halfangle_cosine(
	const eiVector *wo, 
	const eiVector *wi);
/** \brief Compute the Fresnel term for conductors. */
eiAPI void ei_fresnel_conductor(
	eiVector * const result, 
	eiScalar cosi, 
	const eiVector *eta, 
	const eiVector *k);
/** \brief Compute the Fresnel term for dielectrics. */
eiAPI void ei_fresnel_dielectric(
	eiVector * const result, 
	eiScalar cosi, 
	const eiScalar eta_i, 
	const eiScalar eta_t);

/** \brief The generic micro-facet distribution callbacks */
typedef eiScalar (*eiMicrofacet_bsdf)(
	const eiVector *wh, 
	void *param);
typedef void (*eiMicrofacet_sample_bsdf)(
	const eiVector *wo, 
	eiVector * const wi, 
	void *param, 
	eiScalar u1, eiScalar u2);
typedef eiScalar (*eiMicrofacet_pdf)(
	const eiVector *wo, 
	const eiVector *wi, 
	void *param);

/** \brief Fresnel blend model, for simulating the Fresnel 
 * effect when a specular material is painted over a 
 * diffuse material */
eiAPI void ei_fresnelblend_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiVector *Rd, 
	const eiVector *Rs, 
	eiMicrofacet_bsdf dist, 
	void *dist_param);
eiAPI eiBool ei_fresnelblend_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	eiMicrofacet_sample_bsdf dist, 
	void *dist_param, 
	eiScalar u1, eiScalar u2);
eiAPI eiScalar ei_fresnelblend_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	eiMicrofacet_pdf dist, 
	void *dist_param);

/** \brief Lambertian model */
eiAPI void ei_lambert_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi);
eiAPI eiBool ei_lambert_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	eiScalar u1, eiScalar u2);
eiAPI eiScalar ei_lambert_pdf(
	const eiVector *wo, 
	const eiVector *wi);

/** \brief Oren-Nayar model */
eiAPI void ei_orennayar_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar sig);
eiAPI eiBool ei_orennayar_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	const eiScalar sig, 
	eiScalar u1, eiScalar u2);
eiAPI eiScalar ei_orennayar_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar sig);

/** \brief Specular reflection model */
eiAPI void ei_specularreflection_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi);
eiAPI eiBool ei_specularreflection_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi);
eiAPI eiScalar ei_specularreflection_pdf(
	const eiVector *wo, 
	const eiVector *wi);

/** \brief Specular transmission model */
eiAPI void ei_speculartransmission_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar etai, 
	const eiScalar etat);
eiAPI eiBool ei_speculartransmission_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	const eiScalar etai, 
	const eiScalar etat);
eiAPI eiScalar ei_speculartransmission_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar etai, 
	const eiScalar etat);

/** \brief Micro-facet term */
eiAPI void ei_microfacet_term(
	eiVector * const result, 
	const eiVector *wo, 
	const eiVector *wi);

/** \brief Blinn model */
typedef struct eiBlinnParams {
	eiScalar	exponent;
} eiBlinnParams;

eiAPI eiScalar ei_blinn_microfacet_bsdf(
	const eiVector *wh, 
	void *param);
eiAPI void ei_blinn_microfacet_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	void *param, 
	eiScalar u1, eiScalar u2);
eiAPI eiScalar ei_blinn_microfacet_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	void *param);

eiAPI void ei_blinn_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar exponent);
eiAPI eiBool ei_blinn_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	const eiScalar exponent, 
	eiScalar u1, eiScalar u2);
eiAPI eiScalar ei_blinn_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar exponent);

/** \brief Ward anisotropic model */
typedef struct eiWardParams {
	eiScalar	shiny_u;
	eiScalar	shiny_v;
} eiWardParams;

eiAPI eiScalar ei_ward_microfacet_bsdf(
	const eiVector *wh, 
	void *param);
eiAPI void ei_ward_microfacet_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	void *param, 
	eiScalar u1, eiScalar u2);
eiAPI eiScalar ei_ward_microfacet_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	void *param);

eiAPI void ei_ward_bsdf(
	eiVector * const f, 
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar shiny_u, const eiScalar shiny_v);
eiAPI eiBool ei_ward_sample_bsdf(
	const eiVector *wo, 
	eiVector * const wi, 
	const eiScalar shiny_u, const eiScalar shiny_v, 
	eiScalar u1, eiScalar u2);
eiAPI eiScalar ei_ward_pdf(
	const eiVector *wo, 
	const eiVector *wi, 
	const eiScalar shiny_u, const eiScalar shiny_v);

eiAPI void ei_store_photon(eiState *state);
eiAPI eiInt ei_choose_simple_scatter_type(
	eiState *state, 
	const eiScalar transp, 
	eiVector *diffuse, 
	eiVector *specular);
eiAPI eiBool ei_photon_reflection_specular(
	const eiVector *color, 
	eiState *state, 
	const eiVector *dir);
eiAPI eiBool ei_photon_reflection_diffuse(
	const eiVector *color, 
	eiState *state, 
	const eiVector *dir);
eiAPI eiBool ei_photon_transparent(
	const eiVector *color, 
	eiState *state);
eiAPI eiBool ei_photon_transmission_specular(
	const eiVector *color, 
	eiState *state, 
	const eiVector *dir);

/** \brief Lookup a scalar texture at one point. */
eiAPI void ei_scalar_texture(
	eiState * const state, 
	eiScalar *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t);
/** \brief Lookup a vector texture at one point. */
eiAPI void ei_vector_texture(
	eiState * const state, 
	eiVector *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s, const eiScalar t);
/** \brief Lookup a scalar texture by filtering over 
 * a quad. */
eiAPI void ei_scalar_texture_filtered(
	eiState * const state, 
	eiScalar *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4);
/** \brief Lookup a vector texture by filtering over 
 * a quad. */
eiAPI void ei_vector_texture_filtered(
	eiState * const state, 
	eiVector *value, 
	const eiTag tag, 
	const eiUint channel, 
	const eiScalar s1, const eiScalar t1, 
	const eiScalar s2, const eiScalar t2, 
	const eiScalar s3, const eiScalar t3, 
	const eiScalar s4, const eiScalar t4);

/** \brief Compute a deterministic random number from current state. */
eiAPI eiGeoScalar ei_state_random(eiState * const state);

/** \brief Get the transformation matrix from internal space to camera space. */
eiAPI void ei_to_camera(eiMatrix *mat, eiState * const state);
/** \brief Get the transformation matrix from camera space to internal space. */
eiAPI void ei_from_camera(eiMatrix *mat, eiState * const state);
/** \brief Get the transformation matrix from internal space to object space. */
eiAPI void ei_to_object(eiMatrix *mat, eiState * const state);
/** \brief Get the transformation matrix from object space to internal space. */
eiAPI void ei_from_object(eiMatrix *mat, eiState * const state);
/** \brief Get the transformation matrix from internal space to light space. */
eiAPI void ei_to_light(eiMatrix *mat, eiState * const state);
/** \brief Get the transformation matrix from light space to internal space. */
eiAPI void ei_from_light(eiMatrix *mat, eiState * const state);
/** \brief Get the transformation matrix from internal space to world space. */
eiAPI void ei_to_world(eiMatrix *mat, eiState * const state);
/** \brief Get the transformation matrix from world space to internal space. */
eiAPI void ei_from_world(eiMatrix *mat, eiState * const state);
/** \brief Transform a vector from internal space to local space which is 
 * used for defining BSDFs. */
eiAPI void ei_to_local(eiVector * const r, const eiVector *v, eiState * const state);
/** \brief Transform a vector from local space(which is used for defining BSDFs) 
 * to internal space. */
eiAPI void ei_from_local(eiVector * const r, const eiVector *v, eiState * const state);

/** \brief Cook-Torrance BRDF.
 * @param r Roughness.
 */
eiAPI eiScalar ei_brdf_cook_torrance(
	const eiVector *in, 
	const eiVector *out, 
	const eiVector *normal, 
	eiScalar r);

#ifdef __cplusplus
}
#endif

#endif
