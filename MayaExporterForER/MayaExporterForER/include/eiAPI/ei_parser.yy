%locations
%pure_parser

%{
#include <stdarg.h>
#include <stdio.h>

#include <iostream>
#include <memory>
#include <vector>

#include <eiAPI/ei_parser_context.hpp>

#include "ei_parser.yy.hpp"

void yyerror(char *);
int yylex(YYSTYPE *lvalp, YYLTYPE *llocp);

extern FILE* yyin;
extern int yylineno;

static std::auto_ptr<Context> context;

%}

%union
{
	int   integer;
	float real;
	char* string;
}

%token<integer>	INTEGER
%token<real>	REAL
%token<string>	STRING


%token	OPTIONS
%token	SAMPLES
%token	CONTRAST
%token  BUCKET_SIZE
%token	FILTER
%token	MAX_DISPLACE
%token  SHUTTER
%token  MOTION
%token  MOTION_SEGMENTS
%token  TRACE_DEPTH
%token  CAUSTIC
%token  CAUSTIC_PHOTONS
%token  CAUSTIC_ACCURACY
%token  CAUSTIC_SCALE
%token  CAUSTIC_FILTER
%token  PHOTON_TRACE_DEPTH
%token  PHOTON_DECAY
%token  GLOBILLUM
%token  GLOBILLUM_PHOTONS
%token  GLOBILLUM_ACCURACY
%token  GLOBILLUM_SCALE
%token  FINALGATHER
%token  FINALGATHER_ACCURACY
%token  FINALGATHER_FALLOFF
%token  FINALGATHER_FALLOFF_RANGE
%token  FINALGATHER_FILTER
%token  FINALGATHER_TRACE_DEPTH
%token  FINALGATHER_SCALE
%token  EXPOSURE
%token  QUANTIZE
%token  FACE


%token	CAMERA
%token	OUTPUT
%token	OUTPUT_VARIABLE
%token	FOCAL
%token	APERTURE
%token	ASPECT
%token	RESOLUTION
%token	WINDOW
%token	CLIP
%token	ADD_LENS
%token	ADD_IMAGER


%token	INSTANCE
%token	ADD_MATERIAL
%token	ELEMENT
%token	TRANSFORM
%token  MOTION_TRANSFORM


%token	SHADER
%token	PARAM_SCALAR
%token	PARAM_STRING
%token  PARAM_INT
%token	PARAM_VECTOR
%token  PARAM_VECTOR4
%token  PARAM_TAG
%token  PARAM_TEXTURE
%token  PARAM_INDEX
%token  PARAM_BOOL
%token  LINK_PARAM


%token	LIGHT
%token	ADD_LIGHT
%token  ADD_EMITTER
%token	ORIGIN
%token  ENERGY
%token  AREA_SAMPLES


%token	MATERIAL
%token	ADD_SURFACE
%token  ADD_DISPLACE
%token	ADD_SHADOW
%token  ADD_VOLUME
%token  ADD_ENVIRONMENT
%token  ADD_PHOTON


%token  TEXTURE
%token  FILE_TEXTURE


%token	OBJECT
%token	POS_LIST
%token  MOTION_POS_LIST
%token	NRM_LIST
%token	UV_LIST
%token	TRIANGLE_LIST


%token	INSTGROUP
%token	ADD_INSTANCE


%token  LINK
%token  DELETE_ELEMENT
%token	RENDER


%token	END

%%

commands :
		|
		commands
		command
		;

command :
		options
		|
		camera
		|
		instance
		|
		shader
		|
		light
		|
		material
		|
		texture
		|
		object
		|
		instgroup
		|
		link
		|
		delete
		|
		render
		;
		
////////////////////////////////////////////////////////////////////////////////////

options :
		OPTIONS
		STRING
		optionitems
		END OPTIONS
		{
			context->createOptions($2);
		}
        ;

optionitems :
		|
		optionitems
		optionitem
		;

optionitem :
		samples
		|
		contrast
		|
		bucket_size
		|
		filter
		|
		max_displace
		|
		shutter
		|
		motion
		|
		motion_segments
		|
		trace_depth
		|
		caustic
		|
		caustic_photons
		|
		caustic_accuracy
		|
		caustic_scale
		|
		caustic_filter
		|
		photon_trace_depth
		|
		photon_decay
		|
		globillum
		|
		globillum_photons
		|
		globillum_accuracy
		|
		globillum_scale
		|
		finalgather
		|
		finalgather_accuracy
		|
		finalgather_falloff
		|
		finalgather_falloff_range
		|
		finalgather_filter
		|
		finalgather_trace_depth
		|
		finalgather_scale
		|
		exposure
		|
		quantize
		|
		face
		;

samples :
		SAMPLES
		INTEGER
		INTEGER
		{
			context->setSamples($2, $3);
		}
        ;

contrast :
		CONTRAST
		REAL
		REAL
		REAL
		REAL
		{
			context->setContrast($2, $3, $4, $5);
		}
		;
		
bucket_size :
		BUCKET_SIZE
		INTEGER
		{
			context->setBucketSize($2);
		}
		;

filter :
		FILTER
		STRING
		REAL
		{
			context->setFilter($2, $3);
		}
		;

max_displace :
		MAX_DISPLACE
		REAL
		{
			context->setMaxDisplace($2);
		}
		;

shutter :
		SHUTTER
		REAL
		REAL
		{
			context->setShutter($2, $3);
		}
		;

motion :
		MOTION
		INTEGER
		{
			context->setMotion($2);
		}
		;
		
motion_segments :
		MOTION_SEGMENTS
		INTEGER
		{
			context->setMotionSegments($2);
		}
		;

trace_depth :
		TRACE_DEPTH
		INTEGER
		INTEGER
		INTEGER
		{
			context->setTraceDepth($2, $3, $4);
		}
		;
		
caustic :
		CAUSTIC
		INTEGER
		{
			context->setCaustic($2);
		}
		;
		
caustic_photons :
		CAUSTIC_PHOTONS
		INTEGER
		{
			context->setCausticPhotons($2);
		}
		;
		
caustic_accuracy :
		CAUSTIC_ACCURACY
		INTEGER
		REAL
		{
			context->setCausticAccuracy($2, $3);
		}
		;
		
caustic_scale :
		CAUSTIC_SCALE
		REAL
		REAL
		REAL
		{
			context->setCausticScale($2, $3, $4);
		}
		;
		
caustic_filter :
		CAUSTIC_FILTER
		STRING
		REAL
		{
			context->setCausticFilter($2, $3);
		}
		;
		
photon_trace_depth :
		PHOTON_TRACE_DEPTH
		INTEGER
		INTEGER
		INTEGER
		{
			context->setPhotonTraceDepth($2, $3, $4);
		}
		;
		
photon_decay :
		PHOTON_DECAY
		REAL
		{
			context->setPhotonDecay($2);
		}
		;
		
globillum :
		GLOBILLUM
		INTEGER
		{
			context->setGlobillum($2);
		}
		;
		
globillum_photons :
		GLOBILLUM_PHOTONS
		INTEGER
		{
			context->setGlobillumPhotons($2);
		}
		;
		
globillum_accuracy :
		GLOBILLUM_ACCURACY
		INTEGER
		REAL
		{
			context->setGlobillumAccuracy($2, $3);
		}
		;
		
globillum_scale :
		GLOBILLUM_SCALE
		REAL
		REAL
		REAL
		{
			context->setGlobillumScale($2, $3, $4);
		}
		;
		
finalgather :
		FINALGATHER
		INTEGER
		{
			context->setFinalgather($2);
		}
		;
		
finalgather_accuracy :
		FINALGATHER_ACCURACY
		INTEGER
		INTEGER
		REAL
		REAL
		{
			context->setFinalgatherAccuracy($2, $3, $4, $5);
		}
		;
		
finalgather_falloff :
		FINALGATHER_FALLOFF
		INTEGER
		{
			context->setFinalgatherFalloff($2);
		}
		;
		
finalgather_falloff_range :
		FINALGATHER_FALLOFF_RANGE
		REAL
		REAL
		{
			context->setFinalgatherFalloffRange($2, $3);
		}
		;
		
finalgather_filter :
		FINALGATHER_FILTER
		REAL
		{
			context->setFinalgatherFilter($2);
		}
		;
		
finalgather_trace_depth :
		FINALGATHER_TRACE_DEPTH
		INTEGER
		INTEGER
		INTEGER
		INTEGER
		{
			context->setFinalgatherTraceDepth($2, $3, $4, $5);
		}
		;
		
finalgather_scale :
		FINALGATHER_SCALE
		REAL
		REAL
		REAL
		{
			context->setFinalgatherScale($2, $3, $4);
		}
		;
		
exposure :
		EXPOSURE
		REAL
		REAL
		{
			context->setExposure($2, $3);
		}
		;
		
quantize :
		QUANTIZE
		REAL
		REAL
		REAL
		REAL
		{
			context->setQuantize($2, $3, $4, $5);
		}
		;
		
face :
		FACE
		STRING
		{
			context->setFace($2);
		}
		;

////////////////////////////////////////////////////////////////////////////////////

camera :
		CAMERA
		STRING
		cameraitems
		END CAMERA
		{
			context->createCamera($2);
		}
		;
		
cameraitems :
		|
		cameraitems
		cameraitem
		;
		
cameraitem :
		output
		|
		focal
		|
		aperture
		|
		aspect
		|
		resolution
		|
		window
		|
		clip
		|
		add_lens
		|
		add_imager
		;

output :
		OUTPUT
		STRING
		STRING
		STRING
		outputitems
		END OUTPUT
		{
			context->setOutput($2, $3, $4);
		}
		;
		
outputitems :
		|
		outputitems
		output_variable
		;
		
output_variable :
		OUTPUT_VARIABLE
		STRING
		STRING
		{
			context->setOutputVariable($2, $3);
		}
		;

focal :
		FOCAL
		REAL
		{
			context->setFocal($2);
		}
		;
		
aperture :
		APERTURE
		REAL
		{
			context->setAperture($2);
		}
		;
		
aspect :
		ASPECT
		REAL
		{
			context->setAspect($2);
		}
		;
		
resolution :
		RESOLUTION
		INTEGER
		INTEGER
		{
			context->setResolution($2, $3);
		}
		;
		
window :
		WINDOW
		INTEGER
		INTEGER
		INTEGER
		INTEGER
		{
			context->setWindow($2, $3, $4, $5);
		}
		;
		
clip :
		CLIP
		REAL
		REAL
		{
			context->setClip($2, $3);
		}
		;
		
add_lens :
		ADD_LENS
		STRING
		{
			context->setLens($2);
		}
		;
		
add_imager :
		ADD_IMAGER
		STRING
		{
			context->setImager($2);
		}
		;

////////////////////////////////////////////////////////////////////////////////////

instance :
		INSTANCE
		STRING
		instanceitems
		END INSTANCE
		{
			context->createInstance($2);
		}
		;
		
instanceitems :
		|
		instanceitems
		instanceitem
		;

instanceitem :
		add_material
		|
		element
		|
		transform
		|
		motion_transform
		;
		
add_material :
		ADD_MATERIAL
		STRING
		{
			context->setMaterial($2);
		}
		;

element :
		ELEMENT
		STRING
		{
			context->setElement($2);
		}
		;
		
transform :
		TRANSFORM
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		{
			float transform[16] = {$2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17};
			context->setTransform(transform);
		}
		;
		
motion_transform :
		MOTION_TRANSFORM
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		REAL
		{
			float transform[16] = {$2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17};
			context->setMotionTransform(transform);
		}
		;

////////////////////////////////////////////////////////////////////////////////////

shader :
		SHADER
		STRING
		shader_params
		END SHADER
		{
			context->createShader($2);
		}
		;
		
shader_params :
		|
		shader_params
		shader_param
		;
		
shader_param :
		param_int
		|
		param_scalar
		|
		param_string
		|
		param_vector
		|
		param_vector4
		|
		param_tag
		|
		param_texture
		|
		param_index
		|
		param_bool
		|
		link_param
		;
		
param_int :
		PARAM_INT
		STRING
		INTEGER
		{
			context->setParamInt($2, $3);
		}
		;
		
param_string :
		PARAM_STRING
		STRING
		STRING
		{
			context->setParamString($2, $3);
		}
		;
		
param_scalar :
		PARAM_SCALAR
		STRING
		REAL
		{
			context->setParamScalar($2, $3);
		}
		;

param_vector :
		PARAM_VECTOR
		STRING
		REAL
		REAL
		REAL
		{
			context->setParamVector($2, $3, $4, $5);
		}
		;
		
param_vector4 :
		PARAM_VECTOR4
		STRING
		REAL
		REAL
		REAL
		REAL
		{
			context->setParamVector4($2, $3, $4, $5, $6);
		}
		;
		
param_tag :
		PARAM_TAG
		STRING
		INTEGER
		{
			context->setParamTag($2, $3);
		}
		;
		
param_texture :
		PARAM_TEXTURE
		STRING
		STRING
		{
			context->setParamTexture($2, $3);
		}
		;
		
param_index :
		PARAM_INDEX
		STRING
		INTEGER
		{
			context->setParamIndex($2, $3);
		}
		;
		
param_bool :
		PARAM_BOOL
		STRING
		INTEGER
		{
			context->setParamBool($2, $3);
		}
		;
		
link_param :
		LINK_PARAM
		STRING
		STRING
		STRING
		{
			context->setLinkParam($2, $3, $4);
		}
		;

////////////////////////////////////////////////////////////////////////////////////

light :
		LIGHT
		STRING
		lightitems
		END LIGHT
		{
			context->createLight($2);
		}
		;
		
lightitems :
		|
		lightitems
		lightitem
		;
		
lightitem :
		add_light
		|
		add_emitter
		|
		origin
		|
		energy
		|
		area_samples
		;
		
add_light :
		ADD_LIGHT
		STRING
		{
			context->setLight($2);
		}
		;
		
add_emitter :
		ADD_EMITTER
		STRING
		{
			context->setEmitter($2);
		}
		;
		
origin :
		ORIGIN
		REAL
		REAL
		REAL
		{
			context->setOrigin($2, $3, $4);
		}
		;
		
energy :
		ENERGY
		REAL
		REAL
		REAL
		{
			context->setEnergy($2, $3, $4);
		}
		;
		
area_samples :
		AREA_SAMPLES
		INTEGER
		INTEGER
		INTEGER
		INTEGER
		INTEGER
		{
			context->setAreaSamples($2, $3, $4, $5, $6);
		}
		;

////////////////////////////////////////////////////////////////////////////////////

material :
		MATERIAL
		STRING
		materialitems
		END MATERIAL
		{
			context->createMaterial($2);
		}
		;
		
materialitems :
		|
		materialitems
		materialitem
		;
		
materialitem :
		add_surface
		|
		add_displace
		|
		add_shadow
		|
		add_volume
		|
		add_environment
		|
		add_photon
		;

add_surface :
		ADD_SURFACE
		STRING
		{
			context->setSurface($2);
		}
		;
		
add_displace :
		ADD_DISPLACE
		STRING
		{
			context->setDisplace($2);
		}
		;

add_shadow :
		ADD_SHADOW
		STRING
		{
			context->setShadow($2);
		}
		;
		
add_volume :
		ADD_VOLUME
		STRING
		{
			context->setVolume($2);
		}
		;
		
add_environment :
		ADD_ENVIRONMENT
		STRING
		{
			context->setEnvironment($2);
		}
		;
		
add_photon :
		ADD_PHOTON
		STRING
		{
			context->setPhoton($2);
		}
		;
		
////////////////////////////////////////////////////////////////////////////////////

texture :
		TEXTURE
		STRING
		textureitems
		END TEXTURE
		{
			context->createTexture($2);
		}
		;
		
textureitems :
		file_texture
		;

file_texture :
		FILE_TEXTURE
		STRING
		INTEGER
		{
			context->setFileTexture($2, $3);
		}
		;

////////////////////////////////////////////////////////////////////////////////////

object :
		OBJECT
		STRING
		STRING
		objectitems
		END OBJECT
		{
			context->createObject($2, $3);
		}
		;
		
objectitems :
		|
		objectitems
		objectitem
		;
		
objectitem :
		pos_list
		|
		motion_pos_list
		|
		nrm_list
		|
		uv_list
		|
		triangle_list
		;

pos_list :
		POS_LIST
		INTEGER
		positions
		{
			context->setPositionCount($2);
		}
		;
		
positions :
		|
		positions
		position
		;
		
position :
		REAL
		REAL
		REAL
		{
			context->appendPosition($1, $2, $3);
		}
		;
		
motion_pos_list :
		MOTION_POS_LIST
		INTEGER
		motion_positions
		{
			context->setMotionPositionCount($2);
		}
		;
		
motion_positions :
		|
		motion_positions
		motion_position
		;
		
motion_position :
		REAL
		REAL
		REAL
		{
			context->appendMotionPosition($1, $2, $3);
		}
		;
		
nrm_list :
		NRM_LIST
		INTEGER
		normals
		{
			context->setNormalCount($2);
		}
		;
		
normals :
		|
		normals
		normal
		;
		
normal :
		REAL
		REAL
		REAL
		{
			context->appendNormal($1, $2, $3);
		}
		;

uv_list :
		UV_LIST
		INTEGER
		uvs
		{
			context->setUVCount($2);
		}
		;
		
uvs :
		|
		uvs
		uv
		;
		
uv :
		REAL
		REAL
		{
			context->appendUV($1, $2);
		}
		;

triangle_list :
		TRIANGLE_LIST
		INTEGER
		triangles
		{
			context->setTriangleCount($2);
		}
		;

triangles :
		|
		triangles
		triangle
		;

triangle :
		INTEGER
		INTEGER
		INTEGER
		{
			context->appendTriangle($1, $2, $3);
		}
		;

////////////////////////////////////////////////////////////////////////////////////

instgroup :
		INSTGROUP
		STRING
		add_instances
		END INSTGROUP
		{
			context->createInstGroup($2);
		}
		;

add_instances :
		|
		add_instances
		add_instance
		;

add_instance :
		ADD_INSTANCE
		STRING
		{
			context->setInstance($2);
		}
		;
		
////////////////////////////////////////////////////////////////////////////////////

link :
		LINK
		STRING
		{
			context->createLink($2);
		}
		;
		
delete :
		DELETE_ELEMENT
		STRING
		{
			context->createDelete($2);
		}
		;
		
render :
		RENDER
		STRING
		STRING
		STRING
		{
			context->createRender($2, $3, $4);
		}
		;
%%


void yyerror(char *msg)
{
	fprintf(stderr, "error : %s @%d\n", msg, yylineno);
	exit(-1);
}

void ei_parse(const char *filename)
{
	yyin = fopen(filename, "r");
	if (yyin)
	{
		context.reset(new Context);
		context->echo(&std::cout);
		
		yyparse();
		
		context.reset();
	}
}

/*
int main(int argc, char* argv[])
{
	std::cout << "elvish render" << std::endl;
	++ argv, -- argc;
	if (argc > 0)
		yyin = fopen(argv[0], "r");
	else
		yyin = stdin;

	context.reset(new Context);
	context->echo(&std::cout);
	
	yyparse();
	
	context.reset();
	
	return 0;
}
*/
