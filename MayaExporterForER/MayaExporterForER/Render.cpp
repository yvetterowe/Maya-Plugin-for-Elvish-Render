#include"Render.h"

void Render::parse(MString filename)
{
	ei_parse(filename.asChar());
}

void Render::overrideOptions(MayaExporterForER* exporter)
{
	ei_options("opt");

	OpContrast contrast = exporter->getContrast();
	ei_contrast(contrast.r,contrast.g,contrast.b,contrast.a);

	OpSample sample = exporter->getSample();
	ei_samples(sample.sMin,sample.sMax);

	OpFilter filter = exporter->getFilter();
	if(filter.fTypeId && filter.fTypeId!=-1)
		ei_filter(filter.fTypeId,filter.fSize);

	OpTraceDepth traceDepth = exporter->getTraceDepth();
	ei_trace_depth(traceDepth.reflect,traceDepth.refrect,traceDepth.sum);



	int globType = exporter->getGloabalIllumi();
	ei_globillum(globType);

	int finalGather = exporter->getFinalGather();
	ei_finalgather(finalGather);
	
	OpResolution resolution = exporter->getResolution();
	ei_resolution(resolution.width,resolution.height);

	ei_end_options();

}

void Render::createScene()
{
	char	cur_dir[ EI_MAX_FILE_NAME_LEN ];
	char	output_filename[ EI_MAX_FILE_NAME_LEN ];

	ei_get_current_directory(cur_dir);

	ei_context(ei_create_context());

	ei_verbose(EI_VERBOSE_ALL);
	ei_link("eiIMG");
	ei_link("eiSHADER");
}

void Render::setOptions()
{


}