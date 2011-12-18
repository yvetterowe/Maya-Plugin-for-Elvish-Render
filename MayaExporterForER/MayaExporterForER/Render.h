#pragma once

#include<eiAPI\ei.h>
#include"MayaExporterForER.h"

class Render
{
public:
	void parse(MString filename);
	//void overrideOptions(MayaExporterForER* exporter);
	void createScene();
	void setOptions();
};