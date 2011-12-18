#include "DagNodeWriter.h"
#include "stringprintf.h"

#include <maya/MFnDagNode.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnTransform.h>

#include<eiAPI\ei.h>

DagNodeWriter::DagNodeWriter(MDagPath dagPath, MStatus status)
{
	fpath = new MDagPath(dagPath);
	MFnDagNode node(dagPath);

	MFnTransform transForm = MFnTransform(node.parent(0));
	fTransmatOrigin = transForm.transformation();
	fTransMat = transForm.transformation().asMatrix();
	fTranslation = transForm.getTranslation(MSpace::kObject);
}

DagNodeWriter::~DagNodeWriter()
{
	if(fpath!=NULL) delete fpath;
}

void DagNodeWriter::outputTabs( ostream& os, int tabCount )
{
	for (int i = 0; i < tabCount; i++) {
		os << "\t";
	}
}

void DagNodeWriter::outputInstance( ostream& os, MString instName )
{
	os<<"instance "<<"\""<<instName.asChar()<<"\"\n";
	outputTabs(os,1); os<<"element "<<"\""<<fname.asChar()<<"\"\n";
	outputTabs(os,1); outputTransform(os);
	os<<"end instance"<<"\n";
	os<<"\n";
}

void DagNodeWriter::render_instance(MString instName)
{
	ei_instance(instName.asChar());
	   ei_element(fname.asChar());
	   render_transform();
	ei_end_instance();
}

MString DagNodeWriter::GetInstName()
{
	return fInstName;
}

void DagNodeWriter::outputTransform( ostream& os )
{
	os<<"transform ";
	for(int i = 0;i<4;++i)
	{
		for(int j = 0;j<4;++j)
		{
			os<<StringPrintf("%.6lf ",fTransMat(i,j));
		}
	}
	os<<"\n";
}

void DagNodeWriter::render_transform()
{
	ei_transform(fTransMat(0,0),fTransMat(0,1),
		         fTransMat(0,2),fTransMat(0,3),
				 fTransMat(1,0),fTransMat(1,1),
		         fTransMat(1,2),fTransMat(1,3),
				 fTransMat(2,0),fTransMat(2,1),
		         fTransMat(2,2),fTransMat(2,3),
				 fTransMat(3,0),fTransMat(3,1),
		         fTransMat(3,2),fTransMat(3,3));

}

void DagNodeWriter::openPhoton()
{
	isPhotonOpen = true;
}
