#pragma once

#include <maya/MPxFileTranslator.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>

#include<eiAPI\ei.h>

#include "OptionType.h"

class MDagPath;
class MFnDagNode;
class DagNodeWriter;

class MayaExporterForER : public MPxFileTranslator
{
public:
	MayaExporterForER();
	virtual ~MayaExporterForER();

	virtual MStatus         writer (const MFileObject& file,
									const MString& optionsString,
									MPxFileTranslator::FileAccessMode mode);
	virtual bool			haveWriteMethod () const;
	virtual bool            haveReadMethod () const;
	virtual bool            canBeOpened () const;
	MString                 defaultExtension() const;

	void					parseArglist( const MArgList& args );

	//render
	void                    render();

	//get options
	OpContrast				getContrast()			{ return opContrast;}
	OpSample				getSample()				{ return opSample;}
	OpFilter				getFilter()				{ return opFilter;}
	OpTraceDepth			getTraceDepth()			{ return opTraceDepth;}
	int						getGloabalIllumi()		{ return opGlobalIllumi;}
	int						getFinalGather()		{ return opFinalGather;}
	OpResolution			getResolution()			{ return opResolution;}
	double					getGamma()              { return opGamma;}


private: 
	virtual bool            isVisible(MFnDagNode& fnDag, MStatus& status);
	virtual MStatus         exportAll(ostream& os);
	virtual MStatus         processDagNode(const MDagPath dagPath, ostream& os);
	DagNodeWriter*			createDagNodeWriter(const MDagPath dagPath, MStatus& status);

	//more outputs
	void					outputRenderConfig(ostream& os);
	void                    outputOptions(ostream& os);
	void					outputLinks(ostream& os);
	void					outputGammaCorrection(ostream& os);

	//set options
	void					setContrast(double r,double g,double b,double a);
	void					setSample(int sMin,int sMax);
	void					setFilter(MString t,double s);
	void					setTraceDepth(int tRl,int tRR,int tSum);
	void					setGlobalIllumi(int g);
	void					setFinalGather(int f);
	void                    setResolution(int width,int height);

	//render embaded
	void                    render_createScene();
	void                    render_setOptions();
	void                    render_setConfigure();
	void                    render_setLinks();
	void                    render_setGammaCorrection();
	void                    render_override();
	

	MStringArray			instanceContainer;
	MString                 camaraInstance;
	MString					option;

	//options
	OpContrast				opContrast;
	OpSample				opSample;
	OpFilter				opFilter;
	OpTraceDepth			opTraceDepth;
	int						opGlobalIllumi;
	int						opFinalGather;
	OpResolution			opResolution;

	double					opGamma;

	//more self-defined shaders
	MStringArray			shaders;

	//current dir
	//char cur_dir[EI_MAX_FILE_NAME_LEN];

	
};