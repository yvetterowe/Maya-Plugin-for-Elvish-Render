%option bison-bridge bison-locations
%option case-insensitive never-interactive
%option nounistd
%option noyywrap
%option yylineno

%{
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ei_parser.yy.hpp"

%}

%%
\n								{++ yylineno;}
[ \r\t]							{}

[0-9]+							{yylval->integer = atoi(yytext); return INTEGER;}
[\+\-0-9]+"."[0-9]*				{yylval->real    = atof(yytext); return REAL;}
\"[^\"]*\"						{char* temp = strdup(yytext); int len = strlen(temp); yylval->string = (char*)malloc(len + 1); memset(yylval->string, '\0', len + 1); strncpy(yylval->string, temp + 1, len - 2); return STRING;}
#[^\n]*							{}

options							return OPTIONS;
samples							return SAMPLES;
contrast						return CONTRAST;
bucket_size						return BUCKET_SIZE;
filter							return FILTER;
max_displace                    return MAX_DISPLACE;
shutter                         return SHUTTER;
motion                          return MOTION;
motion_segments					return MOTION_SEGMENTS;
trace_depth                     return TRACE_DEPTH;
caustic							return CAUSTIC;
caustic_photons					return CAUSTIC_PHOTONS;
caustic_accuracy				return CAUSTIC_ACCURACY;
caustic_scale					return CAUSTIC_SCALE;
caustic_filter					return CAUSTIC_FILTER;
photon_trace_depth				return PHOTON_TRACE_DEPTH;
photon_decay					return PHOTON_DECAY;
globillum						return GLOBILLUM;
globillum_photons				return GLOBILLUM_PHOTONS;
globillum_accuracy				return GLOBILLUM_ACCURACY;
globillum_scale					return GLOBILLUM_SCALE;
finalgather						return FINALGATHER;
finalgather_accuracy			return FINALGATHER_ACCURACY;
finalgather_falloff				return FINALGATHER_FALLOFF;
finalgather_falloff_range		return FINALGATHER_FALLOFF_RANGE;
finalgather_filter				return FINALGATHER_FILTER;
finalgather_trace_depth			return FINALGATHER_TRACE_DEPTH;
finalgather_scale				return FINALGATHER_SCALE;
exposure						return EXPOSURE;
quantize						return QUANTIZE;
face							return FACE;


camera							return CAMERA;
output							return OUTPUT;
output_variable					return OUTPUT_VARIABLE;
focal							return FOCAL;
aperture						return APERTURE;
aspect							return ASPECT;
resolution						return RESOLUTION;
window							return WINDOW;
clip							return CLIP;
add_lens						return ADD_LENS;
add_imager						return ADD_IMAGER;


instance						return INSTANCE;
add_material					return ADD_MATERIAL;
element							return ELEMENT;
transform						return TRANSFORM;
motion_transform				return MOTION_TRANSFORM;


shader							return SHADER;
param_string					return PARAM_STRING;
param_int						return PARAM_INT;
param_scalar					return PARAM_SCALAR;
param_vector					return PARAM_VECTOR;
param_vector4					return PARAM_VECTOR4;
param_tag						return PARAM_TAG;
param_texture					return PARAM_TEXTURE;
param_index						return PARAM_INDEX;
param_bool						return PARAM_BOOL;
link_param						return LINK_PARAM;


light							return LIGHT;
add_light						return ADD_LIGHT;
add_emitter						return ADD_EMITTER;
origin							return ORIGIN;
energy							return ENERGY;
area_samples					return AREA_SAMPLES;


material						return MATERIAL;
add_surface						return ADD_SURFACE;
add_displace					return ADD_DISPLACE;
add_shadow						return ADD_SHADOW;
add_volume						return ADD_VOLUME;
add_environment					return ADD_ENVIRONMENT;
add_photon						return ADD_PHOTON;


texture							return TEXTURE;
file_texture					return FILE_TEXTURE;


object							return OBJECT;
pos_list						return POS_LIST;
motion_pos_list					return MOTION_POS_LIST;
nrm_list						return NRM_LIST;
uv_list							return UV_LIST;
triangle_list					return TRIANGLE_LIST;

instgroup						return INSTGROUP;
add_instance					return ADD_INSTANCE;

link							return LINK;
delete							return DELETE_ELEMENT;
render							return RENDER;

end								return END;

%%

/*
int main(int argc, char* argv[])
{
	int tok;

	++ argv, -- argc;
	if (argc > 0)
		yyin = fopen(argv[0], "r");
	else
		yyin = stdin;

	while(tok = yylex())
	{
	}
	printf("%d lines\n", lines);
}
*/
