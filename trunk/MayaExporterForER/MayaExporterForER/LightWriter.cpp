#include "LightWriter.h"
#include "stringprintf.h"

LightWriter::LightWriter(MDagPath dagPath,MStatus status) : DagNodeWriter(dagPath,status)
{
}

LightWriter::~LightWriter()
{
}

MStatus LightWriter::ExtractInfo()
{
	return MStatus::kSuccess;
}

/*MStatus LightWriter::WriteToFile( ostream& os )
{
	return MStatus::kSuccess;
}*/

MStatus LightWriter::render()
{
	return MStatus::kSuccess;
}

void LightWriter::render_emitter()
{
	ei_shader("emitter_shader");
		ei_shader_param_string("desc","sphere_emitter");
	ei_end_shader();
}
