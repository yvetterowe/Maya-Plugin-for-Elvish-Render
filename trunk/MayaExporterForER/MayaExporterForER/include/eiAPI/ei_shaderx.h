/*
 * Copyright 2010 elvish render Team
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either expres sor implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef EI_SHADERX_H
#define EI_SHADERX_H

/** \brief The header for users of the shading language interface, 
 * for consistence, please do not change this file.
 * \file ei_shaderx.h
 * \author Elvic Liang
 */

#include <eiAPI/ei.h>
#include <eiAPI/ei_shaderx_util.h>
#include <eiAPI/ei_noise.h>
#include <eiCORE/ei_random.h>

#define DECLARE_PARAM(param_type, param_name, default_value) \
	if (pid >= 0) {\
		E_##param_name = pid++;\
	} else {\
		ei_declare_shader_param(param_type, #param_name, default_value);\
	}

#define DECLARE_INT(param_name, default_value) \
	if (pid >= 0) {\
		E_##param_name = pid++;\
	} else {\
		ei_declare_shader_param_int(#param_name, default_value);\
	}

#define DECLARE_SCALAR(param_name, default_value) \
	if (pid >= 0) {\
		E_##param_name = pid++;\
	} else {\
		ei_declare_shader_param_scalar(#param_name, default_value);\
	}

#define DECLARE_VECTOR(param_name, x, y, z) \
	if (pid >= 0) {\
		E_##param_name = pid++;\
	} else {\
		ei_declare_shader_param_vector(#param_name, x, y, z);\
	}

#define DECLARE_VECTOR4(param_name, x, y, z, w) \
	if (pid >= 0) {\
		E_##param_name = pid++;\
	} else {\
		ei_declare_shader_param_vector4(#param_name, x, y, z, w);\
	}

#define DECLARE_TAG(param_name, default_value) \
	if (pid >= 0) {\
		E_##param_name = pid++;\
	} else {\
		ei_declare_shader_param_tag(#param_name, default_value);\
	}

#define DECLARE_INDEX(param_name, default_value) \
	if (pid >= 0) {\
		E_##param_name = pid++;\
	} else {\
		ei_declare_shader_param_index(#param_name, default_value);\
	}

#define DECLARE_BOOL(param_name, default_value) \
	if (pid >= 0) {\
		E_##param_name = pid++;\
	} else {\
		ei_declare_shader_param_bool(#param_name, default_value);\
	}

#define PARAM(T, name) \
	eiIndex E_##name;\
	\
	T& name()\
	{\
		return *((T *)ei_eval(get_state(), E_##name));\
	}

#define LENS(name) \
	class name : public lens {\
	public:

#define SURFACE(name) \
	class name : public surface {\
	public:

#define LIGHT(name) \
	class name : public light {\
	public:

#define SHADOW(name) \
	class name : public shadow {\
	public:

#define VOLUME(name) \
	class name : public volume {\
	public:

#define DISPLACEMENT(name) \
	class name : public displacement {\
	public:

#define ENVIRONMENT(name) \
	class name : public environment {\
	public:

#define IMAGER(name) \
	class name : public imager {\
	public:

#define PHOTON(name) \
	class name : public photon {\
	public:

#define EMITTER(name) \
	class name : public emitter {\
	public:

#define GEOMETRY(name) \
	class name : public geometry {\
	public:

#define END(name) \
	};\
	\
	static eiBool name##_main(\
		eiShader *shader, \
		eiVector4 * const result, \
		eiState * const state, \
		void *arg)\
	{\
		name	*pShader;\
		\
		pShader = (name *)shader;\
		\
		pShader->set_state(state);\
		pShader->main();\
		\
		return eiTRUE;\
	}\
	\
	static void name##_deletethis(eiPluginObject *object)\
	{\
		name	*pShader;\
		\
		if (object == NULL)\
		{\
			eiASSERT(0);\
			return;\
		}\
		\
		pShader = (name *)object;\
		\
		pShader->exit();\
		ei_shader_exit((eiShader *)object);\
		\
		ei_free(object);\
	}\
	\
	extern "C" {\
		eiSHADER_API void declare_##name(void *param);\
		eiSHADER_API eiPluginObject *create_##name(void *param);\
	}\
	\
	void declare_##name(void *param)\
	{\
		eiShader	*shader;\
		name		*pShader;\
		\
		shader = (eiShader *)ei_allocate(sizeof(name));\
		\
		pShader = (name *)shader;\
		\
		ei_declare_shader(#name);\
		pShader->parameters(-1);\
		ei_end_declare_shader();\
		\
		ei_free(shader);\
	}\
	\
	eiPluginObject *create_##name(void *param)\
	{\
		eiShader	*shader;\
		name		*pShader;\
		\
		shader = (eiShader *)ei_allocate(sizeof(name));\
		\
		pShader = (name *)shader;\
		\
		ei_shader_init(shader);\
		pShader->parameters(0);\
		pShader->init();\
		\
		((eiPluginObject *)shader)->deletethis = name##_deletethis;\
		shader->main = name##_main;\
		shader->size = sizeof(name);\
		\
		return ((eiPluginObject *)shader);\
	}

class base_shader {
public:
	inline eiState * const get_state()
	{
		return base.state;
	}

	inline void set_state(eiState * const state)
	{
		base.state = state;
	}

protected:
	eiShader		base;

	inline point & P()
	{
		return *((point *)(&get_state()->P));
	}

	inline vector & dPdu()
	{
		return *((vector *)(&get_state()->dPdu));
	}

	inline vector & dPdv()
	{
		return *((vector *)(&get_state()->dPdv));
	}

	inline normal & N()
	{
		return *((normal *)(&get_state()->N));
	}

	inline normal & Ng()
	{
		return *((normal *)(&get_state()->Ng));
	}

	inline scalar & u()
	{
		return get_state()->u;
	}

	inline scalar & v()
	{
		return get_state()->v;
	}

	inline scalar & du()
	{
		return get_state()->du;
	}

	inline scalar & dv()
	{
		return get_state()->dv;
	}

	inline vector & L()
	{
		return *((vector *)(&get_state()->L));
	}

	inline color & Cl()
	{
		return *((color *)(&get_state()->Cl));
	}

	inline color & Ol()
	{
		return *((color *)(&get_state()->Ol));
	}

	inline point & E()
	{
		return *((point *)(&get_state()->E));
	}

	inline vector & I()
	{
		return *((vector *)(&get_state()->I));
	}

	inline scalar & dtime()
	{
		return get_state()->dtime;
	}

	inline vector & dPdtime()
	{
		return *((vector *)(&get_state()->dPdtime));
	}

	inline point & Ps()
	{
		return *((point *)(&get_state()->org));
	}

	inline point & origin()
	{
		return *((point *)(&get_state()->P));
	}

	inline color & Ci()
	{
		return *((color *)(&get_state()->result->color));
	}

	inline color & Oi()
	{
		return *((color *)(&get_state()->result->opacity));
	}

	inline vector faceforward(const vector & vN, const vector & vI)
	{
		return sign(-vI % Ng()) * vN;
	}

	inline eiBool sample(
		geoscalar *samples, 
		uint *instance_number, 
		const uint dimension, 
		const uint *n)
	{
		return ei_sample(
			get_state(), 
			samples, 
			instance_number, 
			dimension, 
			n);
	}

	inline point sample_sphere(
		const point & center, const scalar radius, 
		const scalar u1, const scalar u2)
	{
		point	result;

		ei_uniform_sample_sphere(&result, u1, u2);

		return point(center + result * radius);
	}

	inline point sample_sphere(const point & center, const scalar radius)
	{
		return sample_sphere(center, radius, get_state()->u1, get_state()->u2);
	}

	inline point sample_hemisphere(
		const point & center, const vector & direction, const scalar radius, 
		const scalar u1, const scalar u2)
	{
		point	result;
		vector	u_axis, v_axis;

		ei_uniform_sample_hemisphere(&result, u1, u2);
		result *= radius;

		ortho_basis(&direction, &u_axis, &v_axis);

		return point(center + result.x * u_axis + result.y * v_axis + result.z * direction);
	}

	inline point sample_hemisphere(const point & center, const vector & direction, const float radius)
	{
		return sample_hemisphere(center, direction, radius, get_state()->u1, get_state()->u2);
	}

	inline point sample_cosine_hemisphere(
		const point & center, const vector & direction, const scalar radius, 
		const scalar u1, const scalar u2)
	{
		point	result;
		vector	u_axis, v_axis;

		ei_cosine_sample_hemisphere(&result, u1, u2);
		result *= radius;

		ortho_basis(&direction, &u_axis, &v_axis);

		return point(center + result.x * u_axis + result.y * v_axis + result.z * direction);
	}

	inline point sample_cosine_hemisphere(const point & center, const vector & direction, const float radius)
	{
		return sample_cosine_hemisphere(center, direction, radius, get_state()->u1, get_state()->u2);
	}

	inline void trace_eye(eiSampleInfo * const color, const point & src, const vector & dir)
	{
		ei_trace_eye(color, &src, &dir, get_state());
	}

	inline color trace_shadow(const point & src, const vector & dir, const color & light_color)
	{
		color	lightcol(light_color);

		ei_trace_shadow(&lightcol, &src, &dir, get_state());

		return lightcol;
	}

	inline color trace_reflection(const vector & dir)
	{
		color	refl_color;

		ei_trace_reflection(&refl_color, &dir, get_state());

		return refl_color;
	}

	inline color trace_transmission(const vector & dir)
	{
		color	refr_color;

		ei_trace_transmission(&refr_color, &dir, get_state());

		return refr_color;
	}

	inline color trace_transparent()
	{
		color	trans_color;

		ei_trace_transparent(&trans_color, get_state());

		return trans_color;
	}

	inline eiBool trace_probe(const vector & src, const vector & dir, const scalar max_dist = eiMAX_SCALAR)
	{
		return ei_trace_probe(&src, &dir, max_dist, get_state());
	}

	inline color irradiance()
	{
		color	irrad;

		ei_compute_irradiance(&irrad, get_state());

		return irrad;
	}

	inline vector reflect_specular()
	{
		vector	wo, wi;

		wo = to_local(-normalize(I()));

		ei_specularreflection_sample_bsdf(
			&wo, 
			(eiVector *)(&wi));

		return from_local(wi);
	}

	inline vector reflect_diffuse(const scalar u1, const scalar u2)
	{
		vector	wo, wi;

		wo = to_local(-normalize(I()));

		ei_lambert_sample_bsdf(
			&wo, 
			(eiVector *)(&wi), 
			u1, u2);

		return from_local(wi);
	}

	inline vector transmit_specular(const scalar ior)
	{
		vector	wo, wi;

		wo = to_local(-normalize(I()));

		ei_speculartransmission_sample_bsdf(
			&wo, 
			(eiVector *)(&wi), 
			1.0f, 
			ior);

		return from_local(wi);
	}

	inline scalar scalar_texture(const eiTag tag, const uint channel, const scalar s, const scalar t)
	{
		scalar	value = 0.0f;

		ei_scalar_texture(
			get_state(), 
			&value, 
			tag, 
			channel, 
			s, t);

		return value;
	}

	inline color color_texture(const eiTag tag, const uint channel, const scalar s, const scalar t)
	{
		color	value(0.0f);

		ei_vector_texture(
			get_state(), 
			(eiVector *)(&value), 
			tag, 
			channel, 
			s, t);

		return value;
	}

	inline scalar scalar_texture(
		const eiTag tag, const uint channel, 
		const scalar s1, const scalar t1, 
		const scalar s2, const scalar t2, 
		const scalar s3, const scalar t3, 
		const scalar s4, const scalar t4)
	{
		scalar	value = 0.0f;

		ei_scalar_texture_filtered(
			get_state(), 
			&value, 
			tag, 
			channel, 
			s1, t1, 
			s2, t2, 
			s3, t3, 
			s4, t4);

		return value;
	}

	inline color color_texture(
		const eiTag tag, const uint channel, 
		const scalar s1, const scalar t1, 
		const scalar s2, const scalar t2, 
		const scalar s3, const scalar t3, 
		const scalar s4, const scalar t4)
	{
		color	value(0.0f);

		ei_vector_texture_filtered(
			get_state(), 
			(eiVector *)(&value), 
			tag, 
			channel, 
			s1, t1, 
			s2, t2, 
			s3, t3, 
			s4, t4);

		return value;
	}

	inline geoscalar random()
	{
		return ei_state_random(get_state());
	}

	inline matrix to_camera()
	{
		matrix	mat;

		ei_to_camera((eiMatrix *)(&mat), get_state());

		return mat;
	}

	inline matrix from_camera()
	{
		matrix	mat;

		ei_from_camera((eiMatrix *)(&mat), get_state());

		return mat;
	}

	inline matrix to_object()
	{
		matrix	mat;

		ei_to_object((eiMatrix *)(&mat), get_state());

		return mat;
	}

	inline matrix from_object()
	{
		matrix	mat;

		ei_from_object((eiMatrix *)(&mat), get_state());

		return mat;
	}

	inline matrix to_light()
	{
		matrix	mat;

		ei_to_light((eiMatrix *)(&mat), get_state());

		return mat;
	}

	inline matrix from_light()
	{
		matrix	mat;

		ei_from_light((eiMatrix *)(&mat), get_state());

		return mat;
	}

	inline matrix to_world()
	{
		matrix	mat;

		ei_to_world((eiMatrix *)(&mat), get_state());

		return mat;
	}

	inline matrix from_world()
	{
		matrix	mat;

		ei_from_world((eiMatrix *)(&mat), get_state());

		return mat;
	}

	inline vector to_local(const vector & vec)
	{
		vector	result;

		ei_to_local((eiVector *)(&result), &vec, get_state());

		return result;
	}

	inline vector from_local(const vector & vec)
	{
		vector	result;

		ei_from_local((eiVector *)(&result), &vec, get_state());

		return result;
	}
};

class lens : public base_shader {
protected:
	inline eiSampleInfo *new_sample()
	{
		return ei_new_sample(get_state());
	}

	inline void reset_sample(eiSampleInfo * const sample)
	{
		ei_reset_sample(get_state(), sample);
	}

	inline void add_sample(eiSampleInfo * const c1, eiSampleInfo * const c2)
	{
		ei_add_sample(get_state(), c1, c2);
	}

	inline void mul_sample(eiSampleInfo * const c, const scalar a)
	{
		ei_mul_sample(get_state(), c, a);
	}

	inline void delete_sample(eiSampleInfo * const sample)
	{
		ei_delete_sample(get_state(), sample);
	}
};

class surface : public base_shader {
protected:
	inline eiBool illuminance(
		const point & position, 
		const vector & axis = vector( 0.0f, 0.0f, 1.0f ), 
		const float angle = PI, 
		const char *category = NULL)
	{
		return ei_illuminance(
			get_state(), 
			(const eiVector *)(&position), 
			(const eiVector *)(&axis), 
			angle, 
			category);
	}

	inline eiBool sample_light()
	{
		return ei_sample_light(get_state());
	}

	inline eiBool converged(const color & c1, const color & c2)
	{
		return ei_converged(get_state(), (const eiVector *)(&c1), (const eiVector *)(&c2));
	}
};

class light : public base_shader {
protected:
	inline eiBool illuminate(
		const vector & position, 
		const vector & axis = vector(0.0f, 0.0f, 1.0f), 
		const float angle = PI)
	{
		return ei_illuminate(
			get_state(), 
			(const eiVector *)(&position), 
			(const eiVector *)(&axis), 
			angle);
	}

	inline eiBool solar(
		const vector & axis = vector(0.0f, 0.0f, 1.0f), 
		const float angle = PI)
	{
		return ei_solar(
			get_state(), 
			(const eiVector *)(&axis), 
			angle);
	}
};

class shadow : public base_shader {
protected:
};

class volume : public base_shader {
protected:
	inline eiBool illuminance(
		const point & position, 
		const vector & axis = vector( 0.0f, 0.0f, 1.0f ), 
		const float angle = PI, 
		const char *category = NULL)
	{
		return ei_illuminance(
			get_state(), 
			(const eiVector *)(&position), 
			(const eiVector *)(&axis), 
			angle, 
			category);
	}

	inline eiBool sample_light()
	{
		return ei_sample_light(get_state());
	}

	inline eiBool converged(const color & c1, const color & c2)
	{
		return ei_converged(get_state(), (const eiVector *)(&c1), (const eiVector *)(&c2));
	}
};

class displacement : public base_shader {
protected:
};

class environment : public base_shader {
protected:
};

class imager : public base_shader {
protected:
};

class photon : public base_shader {
protected:
	inline void store_photon()
	{
		ei_store_photon(get_state());
	}

	inline eiInt choose_simple_scatter_type(const scalar transp, color & diffuse, color & specular)
	{
		return ei_choose_simple_scatter_type(
			get_state(), 
			transp, 
			(eiVector *)(&diffuse), 
			(eiVector *)(&specular));
	}

	inline eiBool photon_reflect_specular(const color & energy, const vector & dir)
	{
		return ei_photon_reflection_specular(
			(const eiVector *)(&energy), 
			get_state(), 
			(const eiVector *)(&dir));
	}

	inline eiBool photon_reflect_diffuse(const color & energy, const vector & dir)
	{
		return ei_photon_reflection_diffuse(
			(const eiVector *)(&energy), 
			get_state(), 
			(const eiVector *)(&dir));
	}

	inline eiBool photon_transparent(const color & energy)
	{
		return ei_photon_transparent(
			(const eiVector *)(&energy), 
			get_state());
	}

	inline eiBool photon_transmit_specular(const color & energy, const vector & dir)
	{
		return ei_photon_transmission_specular(
			(const eiVector *)(&energy), 
			get_state(), 
			(const eiVector *)(&dir));
	}
};

class emitter : public base_shader {
protected:
};

class geometry : public base_shader {
protected:
};

inline scalar halfangle_cosine(const vector & wo, const vector & wi)
{
	return ei_halfangle_cosine(&wo, &wi);
}

inline color fresnel_conductor(scalar cosi, const color & eta, const color & k)
{
	color	result;
	ei_fresnel_conductor((eiVector *)(&result), cosi, &eta, &k);
	return result;
}

inline color fresnel_dielectric(scalar cosi, const scalar eta_i, const scalar eta_t)
{
	color	result;
	ei_fresnel_dielectric((eiVector *)(&result), cosi, eta_i, eta_t);
	return result;
}

class Microfacet {
public:
	Microfacet()
	{
		m_bsdf = NULL;
		m_sample_bsdf = NULL;
		m_pdf = NULL;
		m_param = NULL;
	}

	virtual ~Microfacet()
	{
	}

	virtual scalar bsdf(const vector & wh)
	{
		return m_bsdf(&wh, m_param);
	}

	virtual void sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2)
	{
		m_sample_bsdf(&wo, (eiVector *)(&wi), m_param, u1, u2);
	}

	virtual scalar pdf(const vector & wo, const vector & wi)
	{
		return m_pdf(&wo, &wi, m_param);
	}

	eiMicrofacet_bsdf			m_bsdf;
	eiMicrofacet_sample_bsdf	m_sample_bsdf;
	eiMicrofacet_pdf			m_pdf;
	void						*m_param;
};

class BlinnMicrofacet : public Microfacet {
public:
	BlinnMicrofacet(const scalar exponent)
	{
		m_params.exponent = exponent;
		m_bsdf = ei_blinn_microfacet_bsdf;
		m_sample_bsdf = ei_blinn_microfacet_sample_bsdf;
		m_pdf = ei_blinn_microfacet_pdf;
		m_param = &m_params;
	}

	virtual ~BlinnMicrofacet()
	{
	}

protected:
	eiBlinnParams	m_params;
};

class WardMicrofacet : public Microfacet {
public:
	WardMicrofacet(const scalar shiny_u, const scalar shiny_v)
	{
		m_params.shiny_u = shiny_u;
		m_params.shiny_v = shiny_v;
		m_bsdf = ei_ward_microfacet_bsdf;
		m_sample_bsdf = ei_ward_microfacet_sample_bsdf;
		m_pdf = ei_ward_microfacet_pdf;
		m_param = &m_params;
	}

	virtual ~WardMicrofacet()
	{
	}

protected:
	eiWardParams	m_params;
};

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

class BSDF {
public:
	virtual color bsdf(const vector & wo, const vector & wi) = 0;
	virtual eiBool sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2) = 0;
	virtual scalar pdf(const vector & wo, const vector & wi) = 0;
};

class FresnelBlend : public BSDF {
public:
	FresnelBlend(Microfacet *microfacet, const color & Rd, const color & Rs)
	{
		m_microfacet = microfacet;
		m_Rd = Rd;
		m_Rs = Rs;
	}

	virtual ~FresnelBlend()
	{
	}

	virtual color bsdf(const vector & wo, const vector & wi)
	{
		color	result;
		ei_fresnelblend_bsdf((eiVector *)(&result), &wo, &wi, &m_Rd, &m_Rs, m_microfacet->m_bsdf, m_microfacet->m_param);
		return result;
	}

	virtual eiBool sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2)
	{
		return ei_fresnelblend_sample_bsdf(&wo, (eiVector *)(&wi), m_microfacet->m_sample_bsdf, m_microfacet->m_param, u1, u2);
	}

	virtual scalar pdf(const vector & wo, const vector & wi)
	{
		return ei_fresnelblend_pdf(&wo, &wi, m_microfacet->m_pdf, m_microfacet->m_param);
	}

protected:
	Microfacet		*m_microfacet;
	color			m_Rd;
	color			m_Rs;
};

class Lambert : public BSDF {
public:
	Lambert()
	{
	}

	virtual ~Lambert()
	{
	}

	virtual color bsdf(const vector & wo, const vector & wi)
	{
		color	result;
		ei_lambert_bsdf((eiVector *)(&result), &wo, &wi);
		return result;
	}

	virtual eiBool sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2)
	{
		return ei_lambert_sample_bsdf(&wo, (eiVector *)(&wi), u1, u2);
	}

	virtual scalar pdf(const vector & wo, const vector & wi)
	{
		return ei_lambert_pdf(&wo, &wi);
	}
};

class OrenNayar : public BSDF {
public:
	OrenNayar(const scalar sig)
	{
		m_sig = sig;
	}

	virtual ~OrenNayar()
	{
	}

	virtual color bsdf(const vector & wo, const vector & wi)
	{
		color	result;
		ei_orennayar_bsdf((eiVector *)(&result), &wo, &wi, m_sig);
		return result;
	}

	virtual eiBool sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2)
	{
		return ei_orennayar_sample_bsdf(&wo, (eiVector *)(&wi), m_sig, u1, u2);
	}

	virtual scalar pdf(const vector & wo, const vector & wi)
	{
		return ei_orennayar_pdf(&wo, &wi, m_sig);
	}

protected:
	scalar		m_sig;
};

class SpecularReflection : public BSDF {
public:
	SpecularReflection()
	{
	}

	virtual ~SpecularReflection()
	{
	}

	virtual color bsdf(const vector & wo, const vector & wi)
	{
		color	result;
		ei_specularreflection_bsdf((eiVector *)(&result), &wo, &wi);
		return result;
	}

	virtual eiBool sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2)
	{
		return ei_specularreflection_sample_bsdf(&wo, (eiVector *)(&wi));
	}

	virtual scalar pdf(const vector & wo, const vector & wi)
	{
		return ei_specularreflection_pdf(&wo, &wi);
	}
};

class SpecularTransmission : public BSDF {
public:
	SpecularTransmission(const scalar etai, const scalar etat)
	{
		m_etai = etai;
		m_etat = etat;
	}

	virtual ~SpecularTransmission()
	{
	}

	virtual color bsdf(const vector & wo, const vector & wi)
	{
		color	result;
		ei_speculartransmission_bsdf((eiVector *)(&result), &wo, &wi, m_etai, m_etat);
		return result;
	}

	virtual eiBool sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2)
	{
		return ei_speculartransmission_sample_bsdf(&wo, (eiVector *)(&wi), m_etai, m_etat);
	}

	virtual scalar pdf(const vector & wo, const vector & wi)
	{
		return ei_speculartransmission_pdf(&wo, &wi, m_etai, m_etat);
	}

private:
	scalar		m_etai;
	scalar		m_etat;
};

inline color microfacet_term(const vector & wo, const vector & wi)
{
	color	result;
	ei_microfacet_term((eiVector *)(&result), &wo, &wi);
	return result;
}

class Blinn : public BSDF {
public:
	Blinn(const scalar exponent)
	{
		m_exponent = exponent;
	}

	virtual ~Blinn()
	{
	}

	virtual color bsdf(const vector & wo, const vector & wi)
	{
		color	result;
		ei_blinn_bsdf((eiVector *)(&result), &wo, &wi, m_exponent);
		return result;
	}

	virtual eiBool sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2)
	{
		return ei_blinn_sample_bsdf(&wo, (eiVector *)(&wi), m_exponent, u1, u2);
	}

	virtual scalar pdf(const vector & wo, const vector & wi)
	{
		return ei_blinn_pdf(&wo, &wi, m_exponent);
	}

private:
	scalar		m_exponent;
};

class Ward : public BSDF {
public:
	Ward(const scalar shiny_u, const scalar shiny_v)
	{
		m_shiny_u = shiny_u;
		m_shiny_v = shiny_v;
	}

	virtual ~Ward()
	{
	}

	virtual color bsdf(const vector & wo, const vector & wi)
	{
		color	result;
		ei_ward_bsdf((eiVector *)(&result), &wo, &wi, m_shiny_u, m_shiny_v);
		return result;
	}

	virtual eiBool sample_bsdf(const vector & wo, vector & wi, const scalar u1, const scalar u2)
	{
		return ei_ward_sample_bsdf(&wo, (eiVector *)(&wi), m_shiny_u, m_shiny_v, u1, u2);
	}

	virtual scalar pdf(const vector & wo, const vector & wi)
	{
		return ei_ward_pdf(&wo, &wi, m_shiny_u, m_shiny_v);
	}

private:
	scalar		m_shiny_u;
	scalar		m_shiny_v;
};

#endif
