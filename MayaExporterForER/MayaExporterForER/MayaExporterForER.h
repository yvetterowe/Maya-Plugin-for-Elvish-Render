#pragma once

#include <maya/MPxFileTranslator.h>
#include <maya/MStringArray.h>

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

private: 
	virtual bool            isVisible(MFnDagNode& fnDag, MStatus& status);
	virtual MStatus         exportAll(ostream& os);
	virtual MStatus         exportSelection(ostream& os);
	virtual MStatus         processDagNode(const MDagPath dagPath, ostream& os);
	DagNodeWriter*			createDagNodeWriter(const MDagPath dagPath, MStatus& status);

	void					outputRenderConfig(ostream& os);

	MStringArray			instanceContainer;
};