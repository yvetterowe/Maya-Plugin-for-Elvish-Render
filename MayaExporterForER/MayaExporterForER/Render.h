#include<eiAPI\ei.h>
#include"MayaExporterForER.h"

class Render
{
public:
	void parse(MString filename);
	void overrideOptions(MayaExporterForER* exporter);

};