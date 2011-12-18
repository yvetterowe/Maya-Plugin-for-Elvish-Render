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
 
#ifndef EI_H
#define EI_H

/** \brief The interface header for all API users. this file contains 
 * the C binding version of the scene description language and shading 
 * language interface. for consistency, please do not modify this file.
 * \file ei.h
 * \author Elvic Liang
 */

#include <eiAPI/ei_api.h>
#include <eiAPI/ei_connection.h>
#include <eiCORE/ei_verbose.h>
#include <eiAPI/ei_image.h>
#include <eiAPI/ei_shader.h>
#include <eiAPI/ei_attributes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_VERSION			"0.8.0.0"

/* constants */
#define on			eiTRUE
#define off			eiFALSE

/** \brief The filter types */
enum {
	EI_FILTER_NONE = 0, 
	EI_FILTER_BOX, 
	EI_FILTER_TRIANGLE, 
	EI_FILTER_CATMULLROM, 
	EI_FILTER_GAUSSIAN, 
	EI_FILTER_SINC, 
	EI_FILTER_COUNT, 
};

/** \brief The back-face culling mode */
enum {
	EI_FACE_NONE = 0, 
	EI_FACE_FRONT, 
	EI_FACE_BACK, 
	EI_FACE_BOTH, 
	EI_FACE_COUNT, 
};

/** \brief The ray-tracing acceleration mode */
enum {
	EI_ACCEL_NONE = 0, 
	EI_ACCEL_BSP, 
	EI_ACCEL_LARGE_BSP, 
	EI_ACCEL_COUNT, 
};

/** \brief The image data types */
enum {
	EI_IMG_DATA_NONE = 0, 
	EI_IMG_DATA_RGB, 
	EI_IMG_DATA_RGBA, 
	EI_IMG_DATA_RGBAZ, 
	EI_IMG_DATA_TYPE_COUNT, 
};

/** \brief The caustic filter types */
enum {
	EI_CAUSTIC_FILTER_NONE = 0, 
	EI_CAUSTIC_FILTER_BOX, 
	EI_CAUSTIC_FILTER_CONE, 
	EI_CAUSTIC_FILTER_GAUSSIAN, 
	EI_CAUSTIC_FILTER_COUNT, 
};

/* the rendering context handle */
typedef struct eiContext	eiContext;

/* parser */
eiAPI void ei_parse(const char *filename);

/* client application connection */
eiAPI void ei_connection(eiConnection *con);

/* rendering contexts */
eiAPI eiContext *ei_create_context();
eiAPI void ei_delete_context(eiContext *context);
eiAPI eiContext *ei_create_server_context(eiGlobals *globals);
eiAPI void ei_delete_server_context(eiContext *context);
eiAPI eiContext *ei_current_context();
eiAPI eiContext *ei_context(eiContext *context);
eiAPI eiDatabase *ei_context_database(eiContext *context);

/* shader declarations */
eiAPI void ei_declare_shader(const char *shader_name);
eiAPI void ei_declare_shader_param(
	const eiInt param_type, 
	const char *param_name, 
	const void *default_value);
eiAPI void ei_declare_shader_param_int(
	const char *param_name, 
	const eiInt default_value);
eiAPI void ei_declare_shader_param_scalar(
	const char *param_name, 
	const eiScalar default_value);
eiAPI void ei_declare_shader_param_vector(
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z);
eiAPI void ei_declare_shader_param_vector4(
	const char *param_name, 
	const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w);
eiAPI void ei_declare_shader_param_tag(
	const char *param_name, 
	const eiTag default_value);
eiAPI void ei_declare_shader_param_index(
	const char *param_name, 
	const eiIndex default_value);
eiAPI void ei_declare_shader_param_bool(
	const char *param_name, 
	const eiBool default_value);
eiAPI void ei_end_declare_shader();

/* shader definitions */
eiAPI void ei_shader(const char *instance_name);
eiAPI void ei_shader_param(
	const char *param_name, 
    const void *param_value);
eiAPI void ei_shader_param_string(
	const char *param_name, 
    const char *param_value);
eiAPI void ei_shader_param_int(
	const char *param_name, 
    const eiInt param_value);
eiAPI void ei_shader_param_scalar(
	const char *param_name, 
    const eiScalar param_value);
eiAPI void ei_shader_param_vector(
	const char *param_name, 
    const eiScalar x, const eiScalar y, const eiScalar z);
eiAPI void ei_shader_param_vector4(
	const char *param_name, 
    const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w);
eiAPI void ei_shader_param_tag(
	const char *param_name, 
	const eiTag param_value);
eiAPI void ei_shader_param_texture(
	const char *param_name, 
	const char *texture_name);
eiAPI void ei_shader_param_index(
	const char *param_name, 
	const eiIndex param_value);
eiAPI void ei_shader_param_bool(
	const char *param_name, 
	const eiBool param_value);
eiAPI void ei_shader_link_param(
    const char *param_name, 
    const char *src_shader_name, 
    const char *src_param_name);
eiAPI void ei_end_shader();

/* plug-in linking */
eiAPI void ei_link(const char *filename);

/* commands */
eiAPI void ei_verbose(const eiInt level);
eiAPI void ei_delete(const char *element_name);
/** \brief Render the scene.
 * @param root_instgroup The root instance group, used to specify a sub-scene to render.
 * @param camera_inst The camera instance element (which must also have been attached 
 * to the root instance group).
 * @param options The name of an options element which must be given.
 */
eiAPI void ei_render(const char *root_instgroup, const char *camera_inst, const char *options);

/* nodes */
eiAPI void ei_declare(const char *name, const eiInt storage_class, const eiInt type, const void *default_value);
eiAPI void ei_variable(const char *name, const void *value);

eiAPI void ei_node(const char *name, const char *type);
eiAPI void ei_end_node();

/* tabs */
eiAPI eiTag ei_tab(const eiInt type, const eiInt items_per_slot);

	eiAPI void ei_tab_add(const void *value);
	eiAPI void ei_tab_add_int(const eiInt value);
	eiAPI void ei_tab_add_scalar(const eiScalar value);
	eiAPI void ei_tab_add_vector(const eiScalar x, const eiScalar y, const eiScalar z);
	eiAPI void ei_tab_add_vector4(const eiScalar x, const eiScalar y, const eiScalar z, const eiScalar w);
	eiAPI void ei_tab_add_tag(const eiTag value);
	eiAPI void ei_tab_add_index(const eiIndex value);
	eiAPI void ei_tab_add_bool(const eiBool value);

eiAPI void ei_end_tab();

/* options */
eiAPI void ei_options(const char *name);

	/* sampling quality */
	/** \brief The contrast controls oversampling. If neighboring samples differ by more 
	 * than the color, supersampling is done as specified by the sampling parameters.
	 */
	eiAPI void ei_contrast(eiScalar r, eiScalar g, eiScalar b, eiScalar a);
	/** \brief This statement determines the minimum and maximum sample rate.
	 */
	eiAPI void ei_samples(eiInt min, eiInt max);
	/** \brief Set the bucket size of tiled rendering.
	 */
	eiAPI void ei_bucket_size(eiInt size);
	/** \brief The filter statement specifies how multiple samples are to be combined 
	 * into a single pixel value.
	 */
	eiAPI void ei_filter(eiInt filter, eiScalar size);

	/* approximation quality */
	eiAPI void ei_approx(const eiApprox *approx);
	/** \brief This statement overrides all max displace statements in 
	 * displacement-mapped objects with the maximum displacement distance dist.
	 */
	eiAPI void ei_max_displace(eiScalar dist);

	/* motion blur */
	/** \brief This statement controls motion blurring. The camera shutter opens at time 
	 * "open" and closes at time "close".
	 */
	eiAPI void ei_shutter(eiScalar open, eiScalar close);
	eiAPI void ei_motion(eiInt type);
	/** \brief This option specifies how many motion path segments should be created for 
	 * all motion transforms in the scene.
	 */
	eiAPI void ei_motion_segments(eiInt num);

	/* trace depth */
	/** \brief The reflect parameter limits the number of recursive reflection rays. 
	 * refract controls the maximum depth of refraction and transparency rays. 
	 * Sum limits the sum of reflection and refraction rays.
	 */
	eiAPI void ei_trace_depth(eiInt reflect, eiInt refract, eiInt sum);

	/* rendering algorithms */
	eiAPI void ei_acceleration(eiInt type);
	/** \brief The maximum number of primitives in a leaf of the BSP tree.
	 */
	eiAPI void ei_bsp_size(eiInt size);
	/** \brief The maximum number of levels in the BSP tree.
	 */
	eiAPI void ei_bsp_depth(eiInt depth);

	/* feature disabling */
	eiAPI void ei_lens(eiInt type);
	eiAPI void ei_shadow(eiInt type);
	eiAPI void ei_volume(eiInt type);
	eiAPI void ei_geometry(eiInt type);
	eiAPI void ei_displace(eiInt type);
	eiAPI void ei_imager(eiInt type);

	/* caustics */
	eiAPI void ei_caustic(eiInt type);
	/** \brief Sets number of caustic photons to be stored.
	 */
	eiAPI void ei_caustic_photons(eiInt photons);
	/** \brief This option controls how caustics are estimated from the photon maps 
	 * during rendering. The photon map is searched outwards from the 
	 * intersection point and the photons that are encountered are examined. 
	 * "samples" specifies the maximum number of photons that should be examined, 
	 * and "radius" specifies the maximum radius that is searched for photons. 
	 * If "samples" is zero, the number of photons is only limited by "radius", 
	 * and the renderer will pick an appropriate default.
	 * If "radius" is zero, a scene-size dependent radius is used instead. 
	 */
	eiAPI void ei_caustic_accuracy(eiInt samples, eiScalar radius);
	/** \brief Caustics are multiplied by the specified color. Factors greater than 1 
	 * increase the brightness of the effect.
	 */
	eiAPI void ei_caustic_scale(eiScalar r, eiScalar g, eiScalar b);
	/** \brief Filtering controls the sharpness of the caustics. The filter_const must 
	 * be larger than 1.0.
	 */
	eiAPI void ei_caustic_filter(eiInt filter, eiScalar filter_const);
	/** \brief This option is similar to the trace depth option except that it applies to 
	 * photons, not rays. The reflect parameter limits the number of recursive 
	 * reflection photons. If it is set to 0, no photons will be reflected, if it 
	 * is set to 1, one level is allowed but a photon cannot be reflected again, 
	 * and so on. Similarly, refract controls the maximum depth of refracted 
	 * photons. Additionally, it is possible to limit the sum of reflected and 
	 * refracted photon levels with sum. Note that custom shaders may override 
	 * these values.
	 */
	eiAPI void ei_photon_trace_depth(eiInt reflect, eiInt refract, eiInt sum);
	/** \brief Sets the falloff exponent for both caustics and global illumination.
	 */
	eiAPI void ei_photon_decay(eiScalar decay);

	/* global Illumination */
	eiAPI void ei_globillum(eiInt type);
	/** \brief Sets number of global illumination photons to be stored.
	 */
	eiAPI void ei_globillum_photons(eiInt photons);
	/** \brief This option controls how the photon map is used to estimate the intensity 
	 * of global illumination. For a more detailed discussion of how this works, 
	 * see the caustic accuracy option above.
	 */
	eiAPI void ei_globillum_accuracy(eiInt samples, eiScalar radius);
	/** \brief The irradiance part obtained from the globillum photonmap lookup is 
	 * multiplied by the specified color. Factors greater than 1 increase the 
	 * brightness of the effect.
	 */
	eiAPI void ei_globillum_scale(eiScalar r, eiScalar g, eiScalar b);
	/** \brief This option controls how the photon map is used to estimate the intensity 
	 * of caustics or global illumination within a participating medium. It 
	 * applies to photon volume shaders, which compute light patterns in 3D space, 
	 * such as volume caustics created by focused shafts of light cast by objects 
	 * acting as lenses. The details are similar to what is described for the 
	 * caustic accuracy option above.
	 */
	eiAPI void ei_photonvol_accuracy(eiInt samples, eiScalar radius);

	/* final gather */
	eiAPI void ei_finalgather(eiInt type);
	/** \brief "rays" controls how many rays should be used in each final gathering 
	 * step to compute the indirect illumination. "samples" 
	 * controls how many final gather points should be used for irradiance 
	 * interpolations with gradients. "density" controls the initial density of 
	 * final gather points in image space, defined by the number of final gather 
	 * points per pixel unit. "radius" controls the maximum searching radius for 
	 * interpolations.
	 */
	eiAPI void ei_finalgather_accuracy(eiInt rays, eiInt samples, eiScalar density, eiScalar radius);
	eiAPI void ei_finalgather_falloff(eiInt type);
	/** \brief Limits the length of final gather rays to a distance of stop in world space. 
	 * If no object is found within a distance of stop, the ray defaults to the 
	 * environment color. Objects farther away than stop from the illuminated 
	 * point will not cast light. Effectively this limits the reach of indirect 
	 * light for final gathering (but not photons). The start parameter defines 
	 * the beginning of a linear falloff range; objects at a distance between 
	 * start and stop will fade towards the environment color.
	 */
	eiAPI void ei_finalgather_falloff_range(eiScalar start, eiScalar stop);
	/** \brief Final gathering uses an speckle elimination filter that prevents samples 
	 * with extreme brightness from skewing the overall energy stored in a 
	 * finalgather hemisphere. This is done by filtering neighboring samples such 
	 * that extreme values are discarded in the filter size.
	 */
	eiAPI void ei_finalgather_filter(eiScalar size);
	/** \brief This option is similar to trace_depth but applies only to finalgather rays. 
	 * The defaults are all 0, which prevents finalgather rays from spawning 
	 * subrays.
	 */
	eiAPI void ei_finalgather_trace_depth(eiInt reflect, eiInt refract, eiInt diffuse, eiInt sum);
	/** \brief The irradiance part obtained from finalgather is multiplied by the 
	 * specified color. Factors greater than 1 increase the brightness of the 
	 * effect.
	 */
	eiAPI void ei_finalgather_scale(eiScalar r, eiScalar g, eiScalar b);

	/* frame buffer control */
	/** \brief Gamma correction can be applied to rendered and quantized color pixels to 
	 * compensate for output devices with a nonlinear color response.
	 */
	eiAPI void ei_exposure(eiScalar gain, eiScalar gamma);
	/** \brief The value one defines the mapping from floating-point values to fixed 
	 * point values. Dithering is performed by adding a random number to the 
	 * floating-point values before they are rounded to the nearest integer. 
	 * The added value is scaled to lie between plus and minus the 
	 * dither_amplitude. If dither_amplitude is 0, dithering is turned off.
	 */
	eiAPI void ei_quantize(eiScalar one, eiScalar min, eiScalar max, eiScalar dither_amplitude);

	/* miscellaneous */
	/** \brief Whether the front side, the back side or both of a face is visible.
	 */
	eiAPI void ei_face(eiInt type);

eiAPI void ei_end_options();

/* cameras */
eiAPI void ei_camera(const char *name);

	/* output statements */
	eiAPI void ei_output(const char *filename, const char *fileformat, const eiInt datatype);
		eiAPI void ei_output_variable(const char *name, const eiInt datatype);
	eiAPI void ei_end_output();

	/* other camera statements */
	/** \brief The focal distance is set to distance. The focal distance is the distance 
	 * from the camera to the viewing plane. If infinity is used in place of the 
	 * distance, an orthographic view is rendered.
	 */
	eiAPI void ei_focal(eiScalar distance);
	/** \brief The aperture is the width of the viewing plane.
	 */
	eiAPI void ei_aperture(eiScalar aperture);
	/** \brief This is the aspect ratio of the camera.
	 */
	eiAPI void ei_aspect(eiScalar aspect);
	/** \brief Specifies the width and height of the output image in pixels.
	 */
	eiAPI void ei_resolution(eiInt x, eiInt y);
	/** \brief Only the sub-rectangle of the image specified by the four bounds will be 
	 * rendered.
	 */
	eiAPI void ei_window(eiInt xmin, eiInt xmax, eiInt ymin, eiInt ymax);
	/** \brief The hither (near) and yon (far) planes are planes parallel to the viewing 
	 * plane that delimit the rendered scene.
	 */
	eiAPI void ei_clip(eiScalar hither, eiScalar yon);

	eiAPI void ei_add_lens(const char *shader_name);
	eiAPI void ei_add_imager(const char *shader_name);

eiAPI void ei_end_camera();

/* materials */
eiAPI void ei_material(const char *name);

	eiAPI void ei_add_surface(const char *shader_name);
	eiAPI void ei_add_displace(const char *shader_name);
	eiAPI void ei_add_shadow(const char *shader_name);
	eiAPI void ei_add_volume(const char *shader_name);
	eiAPI void ei_add_environment(const char *shader_name);
	eiAPI void ei_add_photon(const char *shader_name);

eiAPI void ei_end_material();

/* textures */
eiAPI void ei_make_texture(
	const char *picturename, const char *texturename, 
	eiInt swrap, eiInt twrap, eiInt filter, eiScalar swidth, eiScalar twidth);

eiAPI void ei_texture(const char *name);

	eiAPI void ei_file_texture(const char *filename, const eiBool local);

eiAPI void ei_end_texture();

/* lights */
eiAPI void ei_light(const char *name);

	eiAPI void ei_add_light(const char *shader_name);
	eiAPI void ei_add_emitter(const char *shader_name);

	eiAPI void ei_origin(eiScalar x, eiScalar y, eiScalar z);
	eiAPI void ei_energy(eiScalar r, eiScalar g, eiScalar b);
	eiAPI void ei_area_samples(eiInt u_samples, eiInt v_samples, 
		eiInt low_level, eiInt low_u_samples, eiInt low_v_samples);

eiAPI void ei_end_light();

/* objects */
eiAPI void ei_object(const char *name, const char *type);

	eiAPI void ei_box(eiScalar xmin, eiScalar ymin, eiScalar zmin, 
		eiScalar xmax, eiScalar ymax, eiScalar zmax);
	eiAPI void ei_motion_box(eiScalar xmin, eiScalar ymin, eiScalar zmin, 
		eiScalar xmax, eiScalar ymax, eiScalar zmax);

	/* procedural objects */
	eiAPI void ei_add_geometry(const char *shader_name);

	/* polygon objects */
	eiAPI void ei_pos_list(const eiTag tab);
	eiAPI void ei_motion_pos_list(const eiTag tab);
	eiAPI void ei_triangle_list(const eiTag tab);

	/* disc objects */
	eiAPI void ei_disc_list(const eiTag tab);

	/* hair objects */
	eiAPI void ei_degree(eiInt degree);

	eiAPI void ei_vertex_list(const eiTag tab);
	eiAPI void ei_motion_vertex_list(const eiTag tab);
	eiAPI void ei_hair_list(const eiTag tab);

eiAPI void ei_end_object();

/* instances */
eiAPI void ei_instance(const char *name);

	/** \brief instance a scene element into this instance. Instances place cameras, 
	 * lights, objects, and instance groups into the scene. Without instances, 
	 * these entities have no effect; they are not tessellated and are not 
	 * scheduled for processing. An instance has a name that identifies the 
	 * instance when it is placed into an instance group. Every instance 
	 * references exactly one element element, which must be the name of a 
	 * camera, a light, an object, or an instance group. If the instanced item 
	 * is a geometry shader function, the scene element created by this special 
	 * shader is actually used as the instanced item.
	 */
	eiAPI void ei_element(const char *element_name);

	/** \brief The transform statement is followed by 16 numbers that define a 4x4 matrix 
	 * in row-major order. The matrix establishes the transformation from the 
	 * object space of the instanced element to the parent coordinate space. If 
	 * the instance is directly attached to the root instance group, the parent 
	 * coordinate space is world space.
	 */
	eiAPI void ei_transform(eiScalar m1, eiScalar m2, eiScalar m3, eiScalar m4, 
		eiScalar m5, eiScalar m6, eiScalar m7, eiScalar m8, 
		eiScalar m9, eiScalar m10, eiScalar m11, eiScalar m12, 
		eiScalar m13, eiScalar m14, eiScalar m15, eiScalar m16);
	/** \brief The motion transform matrix specifies a transformation from local space to 
	 * parent space for motion blurred geometry. If not specified, the instance 
	 * transformation is used for the motion blur transformation. In this case t
	 * he parent instance determines whether motion blur is active or not. Motion 
	 * blur is activated by specifying a motion transformation in the scene DAG. 
	 * This transformation is propagated through the scene DAG in the same way 
	 * as the instance transformations. The motion off statement turns off all 
	 * motion information inherited up to this point, as if the camera and all 
	 * instances above did not have motion transforms.
	 */
	eiAPI void ei_motion_transform(eiScalar m1, eiScalar m2, eiScalar m3, eiScalar m4, 
		eiScalar m5, eiScalar m6, eiScalar m7, eiScalar m8, 
		eiScalar m9, eiScalar m10, eiScalar m11, eiScalar m12, 
		eiScalar m13, eiScalar m14, eiScalar m15, eiScalar m16);

	eiAPI void ei_add_material(const char *material_name);
	eiAPI void ei_attribute(const eiInt flag, const eiBool value);

eiAPI void ei_end_instance();

/* instance groups */
eiAPI void ei_instgroup(const char *name);

	eiAPI void ei_add_instance(const char *name);

eiAPI void ei_end_instgroup();

#ifdef __cplusplus
}
#endif

#endif
