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

#include <algorithm>
#include <iostream>
#include <iterator>

#include <eiAPI/ei_parser_context.hpp>

#include <eiAPI/ei.h>

Context::Context() : mFile(NULL)
{
	resetOptions();
	resetCamera();
	resetObject();
	resetInstance();
	resetShader();
	resetLight();
	resetMaterial();
	resetTexture();
	resetInstGroup();

	// Create a new context.
	ei_context(ei_create_context());

	// Link with display drivers and shaders.
	ei_link("eiIMG");
	ei_link("eiSHADER");
}

Context::~Context()
{
}

/////////////////////////////////////////////////////////

void Context::echo(std::ostream* file)
{
	mFile = file;
}

/////////////////////////////////////////////////////////

void Context::resetCamera()
{
	mCamera.outputList.clear();
	mCamera.outputVariableList.clear();
	mCamera.lensList.clear();
	mCamera.imagerList.clear();
	mCamera.aperture = 1.0f;
	mCamera.aspect = 1.0f;
	mCamera.focal = 1.0f;
	mCamera.resolution[0] = mCamera.resolution[1] = 300;
	mCamera.window[0] = eiMIN_INT;mCamera.window[1] = eiMAX_INT;mCamera.window[2] = eiMIN_INT;mCamera.window[3] = eiMAX_INT;
	mCamera.clip[0] = 0.0f;mCamera.clip[1] = eiMAX_SCALAR;
}

void Context::createCamera(char* name)
{
	mCamera.name = name;

	if (mFile)
	{
		*mFile
		<< "camera \"" << mCamera.name << "\"" << std::endl;
		for (std::map<Camera::Output, Camera::OutputVariableList>::const_iterator itr = mCamera.outputList.begin(); itr != mCamera.outputList.end(); ++itr)
		{
			*mFile
				<< "\toutput \"" << itr->first.name << "\" \"" << itr->first.fileFormat << "\" \"" << itr->first.dataType << "\"" << std::endl;

			for (Camera::OutputVariableList::const_iterator var_itr = itr->second.begin(); var_itr != itr->second.end(); ++var_itr)
			{
				*mFile
					<< "\t\toutput_variable \"" << var_itr->variable << "\" \"" << var_itr->dataType << "\"" << std::endl;
			}

			*mFile << "\tend output" << std::endl;
		}
		for (std::set<std::string>::const_iterator itr = mCamera.lensList.begin(); itr != mCamera.lensList.end(); ++ itr)
		{
			*mFile
				<< "\tadd_lens \"" << *itr << "\"" << std::endl;
		}
		for (std::set<std::string>::const_iterator itr = mCamera.imagerList.begin(); itr != mCamera.imagerList.end(); ++ itr)
		{
			*mFile
				<< "\tadd_imager \"" << *itr << "\"" << std::endl;
		}
		*mFile << std::fixed
			<< "\tfocal " << mCamera.focal << std::endl
			<< "\taperture " << mCamera.aperture << std::endl
			<< "\taspect " << mCamera.aspect << std::endl
			<< "\tresolution " << mCamera.resolution[0] << ' ' << mCamera.resolution[1] << std::endl
			<< "\twindow " << mCamera.window[0] << ' ' << mCamera.window[1] << ' ' << mCamera.window[2] << ' ' << mCamera.window[3] << std::endl
			<< "\tclip " << mCamera.clip[0] << ' ' << mCamera.clip[1] << std::endl
		<< "end camera\n" << std::endl;
	}

	ei_camera(mCamera.name.c_str());
		for (std::map<Camera::Output, Camera::OutputVariableList>::const_iterator itr = mCamera.outputList.begin(); itr != mCamera.outputList.end(); ++itr)
		{
			eiInt outputDataTypeTag = 0;
			if (itr->first.dataType == "rgb")
			{
				outputDataTypeTag = EI_IMG_DATA_RGB;
			}
			else if (itr->first.dataType == "rgba")
			{
				outputDataTypeTag = EI_IMG_DATA_RGBA;
			}
			else if (itr->first.dataType == "rgbaz")
			{
				outputDataTypeTag = EI_IMG_DATA_RGBAZ;
			}
			else
			{
				outputDataTypeTag = EI_IMG_DATA_NONE;
			}
			ei_output(itr->first.name.c_str(), itr->first.fileFormat.c_str(), outputDataTypeTag);

			for (Camera::OutputVariableList::const_iterator var_itr = itr->second.begin(); var_itr != itr->second.end(); ++var_itr)
			{
				eiInt outVarDataTypeTag = 0;
				if (var_itr->dataType == "int")
				{
					outVarDataTypeTag = EI_DATA_TYPE_INT;
				}
				else if (var_itr->dataType == "scalar")
				{
					outVarDataTypeTag = EI_DATA_TYPE_SCALAR;
				}
				else if (var_itr->dataType == "vector")
				{
					outVarDataTypeTag = EI_DATA_TYPE_VECTOR;
				}
				else
				{
					outVarDataTypeTag = EI_DATA_TYPE_NONE;
				}
				ei_output_variable(var_itr->variable.c_str(), outVarDataTypeTag);
			}

			ei_end_output();
		}
		for (std::set<std::string>::const_iterator itr = mCamera.lensList.begin(); itr != mCamera.lensList.end(); ++ itr)
		{
			ei_add_lens((*itr).c_str());
		}
		for (std::set<std::string>::const_iterator itr = mCamera.imagerList.begin(); itr != mCamera.imagerList.end(); ++ itr)
		{
			ei_add_imager((*itr).c_str());
		}
		ei_focal(mCamera.focal);
		ei_aperture(mCamera.aperture);
		ei_aspect(mCamera.aspect);
		ei_resolution(mCamera.resolution[0], mCamera.resolution[1]);
	ei_end_camera();

	resetCamera();
	
	free(name);
}

void Context::setOutput(char* name, char* fileFormat, char* dataType)
{
	Camera::Output output;

	output.name = name;
	output.fileFormat = fileFormat;
	output.dataType = dataType;

	mCamera.outputList.insert(std::make_pair(output, mCamera.outputVariableList));

	mCamera.outputVariableList.clear();

	free(name);
	free(fileFormat);
	free(dataType);
}

void Context::setOutputVariable(char* variable, char* dataType)
{
	Camera::OutputVariable outvar;

	outvar.variable = variable;
	outvar.dataType = dataType;

	mCamera.outputVariableList.insert(outvar);

	free(variable);
	free(dataType);
}

void Context::setFocal(float focal)
{
	mCamera.focal = focal;
}

void Context::setAperture(float aperture)
{
	mCamera.aperture = aperture;
}

void Context::setAspect(float aspect)
{
	mCamera.aspect = aspect;
}

void Context::setResolution(int width, int height)
{
	mCamera.resolution[0] = width;
	mCamera.resolution[1] = height;
}

void Context::setWindow(int xmin, int xmax, int ymin, int ymax)
{
	mCamera.window[0] = xmin;
	mCamera.window[1] = xmax;
	mCamera.window[2] = ymin;
	mCamera.window[3] = ymax;
}

void Context::setClip(float hither, float yon)
{
	mCamera.clip[0] = hither;
	mCamera.clip[0] = yon;
}

void Context::setLens(char* name)
{
	mCamera.lensList.insert(name);

	free(name);
}

void Context::setImager(char* name)
{
	mCamera.imagerList.insert(name);

	free(name);
}

/////////////////////////////////////////////////////////

void Context::resetInstance()
{
	mInstance.materials.clear();

	memset(mInstance.transform, 0, sizeof(float) * 16);
	mInstance.transform[0] = 1;
	mInstance.transform[5] = 1;
	mInstance.transform[10] = 1;
	mInstance.transform[15] = 1;

	memset(mInstance.motionTransform, 0, sizeof(float) * 16);
	mInstance.motionTransform[0] = 1;
	mInstance.motionTransform[5] = 1;
	mInstance.motionTransform[10] = 1;
	mInstance.motionTransform[15] = 1;
}

void Context::createInstance(char* name)
{
	mInstance.name = name;

	if (mFile)
	{
		*mFile
		<< "instance \"" << mInstance.name << "\"" << std::endl;

		for (std::set<std::string>::const_iterator itr = mInstance.materials.begin(); itr != mInstance.materials.end(); ++ itr)
		{
			*mFile
				<< "\tadd_material \"" << *itr << "\"" << std::endl;
		}

		*mFile
			<< "\telement \"" << mInstance.element << "\"" << std::endl;
		
		*mFile
			<< "\ttransform\n\t";
		for (int i = 0; i < 16; ++ i)
		{
			*mFile << std::fixed
				<< mInstance.transform[i] << ' ';
		}
		*mFile
			<< std::endl;

		*mFile
			<< "\tmotion_transform\n\t";
		for (int i = 0; i < 16; ++ i)
		{
			*mFile << std::fixed
				<< mInstance.motionTransform[i] << ' ';
		}
		*mFile
			<< std::endl;

		*mFile
		<< "end instance\n" << std::endl;
	}

	
	ei_instance(mInstance.name.c_str());
		for (std::set<std::string>::const_iterator itr = mInstance.materials.begin(); itr != mInstance.materials.end(); ++ itr)
		{
			ei_add_material(itr->c_str());
		}
		ei_element(mInstance.element.c_str());
		ei_transform(
			mInstance.transform[0], mInstance.transform[1], mInstance.transform[2], mInstance.transform[3],
			mInstance.transform[4], mInstance.transform[5], mInstance.transform[6], mInstance.transform[7],
			mInstance.transform[8], mInstance.transform[9], mInstance.transform[10], mInstance.transform[11],
			mInstance.transform[12], mInstance.transform[13], mInstance.transform[14], mInstance.transform[15]
		);
		ei_motion_transform(
			mInstance.motionTransform[0], mInstance.motionTransform[1], mInstance.motionTransform[2], mInstance.motionTransform[3],
			mInstance.motionTransform[4], mInstance.motionTransform[5], mInstance.motionTransform[6], mInstance.motionTransform[7],
			mInstance.motionTransform[8], mInstance.motionTransform[9], mInstance.motionTransform[10], mInstance.motionTransform[11],
			mInstance.motionTransform[12], mInstance.motionTransform[13], mInstance.motionTransform[14], mInstance.motionTransform[15]
		);
	ei_end_instance();
	
	resetInstance();

	free(name);
}

void Context::setMaterial(char* material)
{
	mInstance.materials.insert(material);

	free(material);
}

void Context::setElement(char* element)
{
	mInstance.element = element;
	
	free(element);
}

void Context::setTransform(float* transform)
{
	for (int i = 0; i < 16; ++ i)
	{
		mInstance.transform[i] = transform[i];
		mInstance.motionTransform[i] = transform[i];
	}
}

void Context::setMotionTransform(float* transform)
{
	for (int i = 0; i < 16; ++ i)
	{
		mInstance.motionTransform[i] = transform[i];
	}
}

/////////////////////////////////////////////////////////

void Context::resetObject()
{
	mObject.posList.clear();
	mObject.motionPosList.clear();
	mObject.nrmList.clear();
	mObject.uvList.clear();
	mObject.triangleList.clear();

	// Reserve space.
	static const size_t reserved = 1048576; // 1024 * 1024 = 1M
	mObject.posList.reserve(reserved * 3);
	mObject.motionPosList.reserve(reserved * 3);
	mObject.nrmList.reserve(reserved * 3);
	mObject.uvList.reserve(reserved * 3);
	mObject.triangleList.reserve(reserved * 3);
}

void Context::createObject(char* name, char* type)
{
	mObject.name = name;
	mObject.type = type;

	if (mFile)
	{
		*mFile
		<< "object \"" << mObject.name << "\" \"" << mObject.type << "\"" << std::endl;
	
		*mFile << std::fixed
			<< "\tpos_list " << mObject.posList.size() / 3 << std::endl << "\t";
		//std::copy(mObject.posList.begin(), mObject.posList.end(), std::ostream_iterator<float>(*mFile, " "));
		*mFile << std::endl;

		if (!mObject.motionPosList.empty())
		{
			*mFile << std::fixed
				<< "\tmotion_pos_list " << mObject.motionPosList.size() / 3 << std::endl << "\t";
			//std::copy(mObject.motionPosList.begin(), mObject.motionPosList.end(), std::ostream_iterator<float>(*mFile, " "));
			*mFile << std::endl;
		}

		if (!mObject.nrmList.empty())
		{
			*mFile << std::fixed
				<< "\tnrm_list " << mObject.nrmList.size() / 3 << std::endl << "\t";
			//std::copy(mObject.nrmList.begin(), mObject.nrmList.end(), std::ostream_iterator<float>(*mFile, " "));
			*mFile << std::endl;
		}
	
		*mFile
			<< "\ttriangle_list " << mObject.triangleList.size() << std::endl << "\t";
		//std::copy(mObject.triangleList.begin(), mObject.triangleList.end(), std::ostream_iterator<int>(*mFile, " "));
		*mFile
		<< std::endl << "end object\n" << std::endl;
	}
	
	ei_object(mObject.name.c_str(), mObject.type.c_str());
		ei_pos_list(ei_tab(EI_DATA_TYPE_VECTOR, mObject.posList.size() / 3));
			for (size_t i = 0; i < mObject.posList.size(); i += 3)
			{
				ei_tab_add_vector(mObject.posList[i], mObject.posList[i + 1], mObject.posList[i + 2]);
			}
		ei_end_tab();
		if (!mObject.motionPosList.empty())
		{
			ei_motion_pos_list(ei_tab(EI_DATA_TYPE_VECTOR, mObject.motionPosList.size() / 3));
				for (size_t i = 0; i < mObject.motionPosList.size(); i += 3)
				{
					ei_tab_add_vector(mObject.motionPosList[i], mObject.motionPosList[i + 1], mObject.motionPosList[i + 2]);
				}
			ei_end_tab();
		}
		if (!mObject.nrmList.empty())
		{
			eiTag tagVal = eiNULL_TAG;
			ei_declare("N", eiVARYING, EI_DATA_TYPE_TAG, &tagVal);
			tagVal = ei_tab(EI_DATA_TYPE_VECTOR, mObject.nrmList.size() / 3);
			ei_variable("N", &tagVal);
				for (size_t i = 0; i < mObject.nrmList.size(); i += 3)
				{
					ei_tab_add_vector(mObject.nrmList[i], mObject.nrmList[i + 1], mObject.nrmList[i + 2]);
				}
			ei_end_tab();
		}
		ei_triangle_list(ei_tab(EI_DATA_TYPE_INDEX, mObject.triangleList.size()));
			for (size_t i = 0; i < mObject.triangleList.size(); ++ i)
			{
				ei_tab_add_index(mObject.triangleList[i]);
			}
		ei_end_tab();
	ei_end_object();
	
	resetObject();
	
	free(name);
	free(type);
}

void Context::setPositionCount(int positionCount)
{
	if (positionCount != mObject.posList.size() / 3)
	{
		exit(EXIT_FAILURE);
	}
}

void Context::appendPosition(float x, float y, float z)
{
	mObject.posList.push_back(x);
	mObject.posList.push_back(y);
	mObject.posList.push_back(z);
}

void Context::setMotionPositionCount(int positionCount)
{
	if (positionCount != mObject.motionPosList.size() / 3)
	{
		exit(EXIT_FAILURE);
	}
}

void Context::appendMotionPosition(float x, float y, float z)
{
	mObject.motionPosList.push_back(x);
	mObject.motionPosList.push_back(y);
	mObject.motionPosList.push_back(z);
}

void Context::setNormalCount(int normalCount)
{
	if (normalCount != mObject.nrmList.size() / 3)
	{
		exit(EXIT_FAILURE);
	}
}

void Context::appendNormal(float x, float y, float z)
{
	mObject.nrmList.push_back(x);
	mObject.nrmList.push_back(y);
	mObject.nrmList.push_back(z);
}

void Context::setUVCount(int uvCount)
{
	if (uvCount != mObject.uvList.size() / 2)
	{
		exit(EXIT_FAILURE);
	}
}

void Context::appendUV(float u, float v)
{
	mObject.uvList.push_back(u);
	mObject.uvList.push_back(v);
}

void Context::setTriangleCount(int triangleCount)
{
	if (triangleCount != mObject.triangleList.size())
	{
		exit(EXIT_FAILURE);
	}
}

void Context::appendTriangle(int a, int b, int c)
{
	mObject.triangleList.push_back(a);
	mObject.triangleList.push_back(b);
	mObject.triangleList.push_back(c);
}

/////////////////////////////////////////////////////////

void Context::resetOptions()
{
	mOptions.contrast[0] = mOptions.contrast[1] = mOptions.contrast[2] = mOptions.contrast[3] = 0.05f;
	mOptions.bucketSize = 48;
	mOptions.filter = "gaussian";
	mOptions.filterWidth = 3.0f;
	mOptions.name = "default";
	mOptions.samples[0] = 0;mOptions.samples[1] = 2;
	mOptions.maxDisplace = 0.0f;
	mOptions.shutter[0] = 0.0f;mOptions.shutter[1] = 1.0f;
	mOptions.motion = 0;
	mOptions.motionSegments = 5;
	mOptions.traceDepth[0] = mOptions.traceDepth[1] = mOptions.traceDepth[2] = 6;
	mOptions.caustic = 0;
	mOptions.causticPhotons = 100000;
	mOptions.causticSamples = 100;
	mOptions.causticRadius = 0.0f;
	mOptions.causticScale[0] = mOptions.causticScale[1] = mOptions.causticScale[2] = 1.0f;
	mOptions.causticFilter = "cone";
	mOptions.causticFilterConst = 1.1f;
	mOptions.photonTraceDepth[0] = mOptions.photonTraceDepth[1] = mOptions.photonTraceDepth[2] = 5;
	mOptions.photonDecay = 2.0f;
	mOptions.globillum = 0;
	mOptions.globillumPhotons = 10000;
	mOptions.globillumSamples = 100;
	mOptions.globillumRadius = 0.0f;
	mOptions.globillumScale[0] = mOptions.globillumScale[1] = mOptions.globillumScale[2] = 1.0f;
	mOptions.finalgather = 0;
	mOptions.finalgatherRays = 500;
	mOptions.finalgatherSamples = 30;
	mOptions.finalgatherDensity = 1.0f;
	mOptions.finalgatherRadius = 0.0f;
	mOptions.finalgatherFalloff = 0;
	mOptions.finalgatherFalloffRange[0] = mOptions.finalgatherFalloffRange[1] = 0.0f;
	mOptions.finalgatherFilter = 4;
	mOptions.finalgatherTraceDepth[0] = mOptions.finalgatherTraceDepth[1] = mOptions.finalgatherTraceDepth[2] = mOptions.finalgatherTraceDepth[3] = 0;
	mOptions.finalgatherScale[0] = mOptions.finalgatherScale[1] = mOptions.finalgatherScale[2] = 1.0f;
	mOptions.exposure[0] = 1.0f;mOptions.exposure[1] = 1.0f;
	mOptions.quantize[0] = 255.0f;mOptions.quantize[1] = 0.0f;mOptions.quantize[2] = 255.0f;mOptions.quantize[3] = 0.5f;
	mOptions.face = "front";
}

void Context::createOptions(char* name)
{
	mOptions.name = name;

	if (mFile)
	{
		*mFile << std::fixed
		<< "options \"" << mOptions.name << "\"" << std::endl
			<< "\tsamples " << mOptions.samples[0] << ' ' << mOptions.samples[1] << std::endl
			<< "\tcontrast " << mOptions.contrast[0] << ' ' << mOptions.contrast[1] << ' ' << mOptions.contrast[2] << ' ' << mOptions.contrast[3] << std::endl
			<< "\tfilter \"" << mOptions.filter << "\" " << mOptions.filterWidth << std::endl
			<< "\tbucket_size " << mOptions.bucketSize << std::endl
			<< "\tmax_displace " << mOptions.maxDisplace << std::endl
			<< "\tshutter " << mOptions.shutter[0] << ' ' << mOptions.shutter[1] << std::endl
			<< "\tmotion " << mOptions.motion << std::endl
			<< "\tmotion_segments " << mOptions.motion << std::endl
			<< "\ttrace_depth " << mOptions.traceDepth[0] << ' ' << mOptions.traceDepth[1] << ' ' << mOptions.traceDepth[2] << std::endl
			<< "\tcaustic " << mOptions.caustic << std::endl
			<< "\tcaustic_photons " << mOptions.causticPhotons << std::endl
			<< "\tcaustic_accuracy " << mOptions.causticSamples << ' ' << mOptions.causticRadius << std::endl
			<< "\tcaustic_scale " << mOptions.causticScale[0] << ' ' << mOptions.causticScale[1] << ' ' << mOptions.causticScale[2] << std::endl
			<< "\tcaustic_filter " << mOptions.causticFilter << ' ' << mOptions.causticFilterConst << std::endl
			<< "\tphoton_trace_depth " << mOptions.photonTraceDepth[0] << ' ' << mOptions.photonTraceDepth[1] << ' ' << mOptions.photonTraceDepth[2] << std::endl
			<< "\tphoton_decay " << mOptions.photonDecay << std::endl
			<< "\tglobillum " << mOptions.globillum << std::endl
			<< "\tglobillum_photons " << mOptions.globillumPhotons << std::endl
			<< "\tglobillum_accuracy " << mOptions.globillumSamples << ' ' << mOptions.globillumRadius << std::endl
			<< "\tglobillum_scale " << mOptions.globillumScale[0] << ' ' << mOptions.globillumScale[1] << ' ' << mOptions.globillumScale[2] << std::endl
			<< "\tfinalgather " << mOptions.finalgather << std::endl
			<< "\tfinalgather_accuracy " << mOptions.finalgatherRays << ' ' << mOptions.finalgatherSamples << ' ' << mOptions.finalgatherDensity << ' ' << mOptions.finalgatherRadius << std::endl
			<< "\tfinalgather_falloff " << mOptions.finalgatherFalloff << std::endl
			<< "\tfinalgather_falloff_range " << mOptions.finalgatherFalloffRange[0] << ' ' << mOptions.finalgatherFalloffRange[1] << std::endl
			<< "\tfinalgather_filter " << mOptions.finalgatherFilter << std::endl
			<< "\tfinalgather_trace_depth " << mOptions.finalgatherTraceDepth[0] << ' ' << mOptions.finalgatherTraceDepth[1] << ' ' << mOptions.finalgatherTraceDepth[2] << ' ' << mOptions.finalgatherTraceDepth[3] << std::endl
			<< "\tfinalgather_scale " << mOptions.finalgatherScale[0] << ' ' << mOptions.finalgatherScale[1] << ' ' << mOptions.finalgatherScale[2] << std::endl
			<< "\texposure " << mOptions.exposure[0] << ' ' << mOptions.exposure[1] << std::endl
			<< "\tquantize " << mOptions.quantize[0] << ' ' << mOptions.quantize[1] << ' ' << mOptions.quantize[2] << ' ' << mOptions.quantize[3] << std::endl
			<< "\tface " << mOptions.face << std::endl
		<< "end options\n" << std::endl;
	}
	
	eiInt filterTag = 0;
	std::transform(mOptions.filter.begin(), mOptions.filter.end(), mOptions.filter.begin(), ::tolower);
	if (mOptions.filter == "box")
	{
		filterTag = EI_FILTER_BOX;
	}
	else if (mOptions.filter == "triangle")
	{
		filterTag = EI_FILTER_TRIANGLE;
	}
	else if (mOptions.filter == "catmullrom")
	{
		filterTag = EI_FILTER_CATMULLROM;
	}
	else if (mOptions.filter == "gaussian")
	{
		filterTag = EI_FILTER_GAUSSIAN;
	}
	else if (mOptions.filter == "sinc")
	{
		filterTag = EI_FILTER_SINC;
	}
	else if (mOptions.filter == "")
	{
		filterTag = EI_FILTER_NONE;
	}
	eiInt causticFilterTag = 0;
	std::transform(mOptions.causticFilter.begin(), mOptions.causticFilter.end(), mOptions.causticFilter.begin(), ::tolower);
	if (mOptions.causticFilter == "box")
	{
		causticFilterTag = EI_CAUSTIC_FILTER_BOX;
	}
	else if (mOptions.causticFilter == "cone")
	{
		causticFilterTag = EI_CAUSTIC_FILTER_CONE;
	}
	else if (mOptions.causticFilter == "gaussian")
	{
		causticFilterTag = EI_CAUSTIC_FILTER_GAUSSIAN;
	}
	else
	{
		causticFilterTag = EI_CAUSTIC_FILTER_NONE;
	}
	eiInt faceTag = 0;
	std::transform(mOptions.face.begin(), mOptions.face.end(), mOptions.face.begin(), ::tolower);
	if (mOptions.face == "front")
	{
		faceTag = EI_FACE_FRONT;
	}
	else if (mOptions.face == "back")
	{
		faceTag = EI_FACE_BACK;
	}
	else if (mOptions.face == "both")
	{
		faceTag = EI_FACE_BOTH;
	}
	else
	{
		faceTag = EI_FACE_NONE;
	}
	ei_options(mOptions.name.c_str());
		ei_samples(mOptions.samples[0], mOptions.samples[1]);
		ei_contrast(mOptions.contrast[0], mOptions.contrast[1], mOptions.contrast[2], mOptions.contrast[3]);
		ei_filter(filterTag, mOptions.filterWidth);
		ei_bucket_size(mOptions.bucketSize);
		ei_max_displace(mOptions.maxDisplace);
		ei_shutter(mOptions.shutter[0], mOptions.shutter[1]);
		ei_motion(mOptions.motion);
		ei_motion_segments(mOptions.motionSegments);
		ei_trace_depth(mOptions.traceDepth[0], mOptions.traceDepth[1], mOptions.traceDepth[2]);
		ei_caustic(mOptions.caustic);
		ei_caustic_photons(mOptions.causticPhotons);
		ei_caustic_accuracy(mOptions.causticSamples, mOptions.causticRadius);
		ei_caustic_scale(mOptions.causticScale[0], mOptions.causticScale[1], mOptions.causticScale[2]);
		ei_caustic_filter(causticFilterTag, mOptions.causticFilterConst);
		ei_photon_trace_depth(mOptions.photonTraceDepth[0], mOptions.photonTraceDepth[1], mOptions.photonTraceDepth[2]);
		ei_photon_decay(mOptions.photonDecay);
		ei_globillum(mOptions.globillum);
		ei_globillum_photons(mOptions.globillumPhotons);
		ei_globillum_accuracy(mOptions.globillumSamples, mOptions.globillumRadius);
		ei_globillum_scale(mOptions.globillumScale[0], mOptions.globillumScale[1], mOptions.globillumScale[2]);
		ei_finalgather(mOptions.finalgather);
		ei_finalgather_accuracy(mOptions.finalgatherRays, mOptions.finalgatherSamples, mOptions.finalgatherDensity, mOptions.finalgatherRadius);
		ei_finalgather_falloff(mOptions.finalgatherFalloff);
		ei_finalgather_falloff_range(mOptions.finalgatherFalloffRange[0], mOptions.finalgatherFalloffRange[1]);
		ei_finalgather_filter(mOptions.finalgatherFilter);
		ei_finalgather_trace_depth(mOptions.finalgatherTraceDepth[0], mOptions.finalgatherTraceDepth[1], mOptions.finalgatherTraceDepth[2], mOptions.finalgatherTraceDepth[3]);
		ei_finalgather_scale(mOptions.finalgatherScale[0], mOptions.finalgatherScale[1], mOptions.finalgatherScale[2]);
		ei_exposure(mOptions.exposure[0], mOptions.exposure[1]);
		ei_quantize(mOptions.quantize[0], mOptions.quantize[1], mOptions.quantize[2], mOptions.quantize[3]);
		ei_face(faceTag);
	ei_end_options();

	resetOptions();

	free(name);
}

void Context::setSamples(int minimal, int maximal)
{
	mOptions.samples[0] = minimal;
	mOptions.samples[1] = maximal;
}

void Context::setContrast(float r, float g, float b, float a)
{
	mOptions.contrast[0] = r;
	mOptions.contrast[1] = g;
	mOptions.contrast[2] = b;
	mOptions.contrast[3] = a;
}

void Context::setBucketSize(int size)
{
	mOptions.bucketSize = size;
}

void Context::setFilter(char* filter, float filterWidth)
{
	mOptions.filter = filter;
	mOptions.filterWidth = filterWidth;

	free(filter);
}

void Context::setMaxDisplace(float dist)
{
	mOptions.maxDisplace = dist;
}

void Context::setShutter(float open, float close)
{
	mOptions.shutter[0] = open;
	mOptions.shutter[1] = close;
}

void Context::setMotion(int motion)
{
	mOptions.motion = motion;
}

void Context::setMotionSegments(int num)
{
	mOptions.motionSegments = num;
}

void Context::setTraceDepth(int reflect, int refract, int sum)
{
	mOptions.traceDepth[0] = reflect;
	mOptions.traceDepth[1] = refract;
	mOptions.traceDepth[2] = sum;
}

void Context::setCaustic(int caustic)
{
	mOptions.caustic = caustic;
}

void Context::setCausticPhotons(int num)
{
	mOptions.causticPhotons = num;
}

void Context::setCausticAccuracy(int samples, float radius)
{
	mOptions.causticSamples = samples;
	mOptions.causticRadius = radius;
}

void Context::setCausticScale(float r, float g, float b)
{
	mOptions.causticScale[0] = r;
	mOptions.causticScale[1] = g;
	mOptions.causticScale[2] = b;
}

void Context::setCausticFilter(char* name, float c)
{
	mOptions.causticFilter = name;
	mOptions.causticFilterConst = c;

	free(name);
}

void Context::setPhotonTraceDepth(int reflect, int refract, int sum)
{
	mOptions.photonTraceDepth[0] = reflect;
	mOptions.photonTraceDepth[1] = refract;
	mOptions.photonTraceDepth[2] = sum;
}

void Context::setPhotonDecay(float decay)
{
	mOptions.photonDecay = decay;
}

void Context::setGlobillum(int globillum)
{
	mOptions.globillum = globillum;
}

void Context::setGlobillumPhotons(int num)
{
	mOptions.globillumPhotons = num;
}

void Context::setGlobillumAccuracy(int samples, float radius)
{
	mOptions.globillumSamples = samples;
	mOptions.globillumRadius = radius;
}

void Context::setGlobillumScale(float r, float g, float b)
{
	mOptions.globillumScale[0] = r;
	mOptions.globillumScale[1] = g;
	mOptions.globillumScale[2] = b;
}

void Context::setFinalgather(int finalgather)
{
	mOptions.finalgather = finalgather;
}

void Context::setFinalgatherAccuracy(int rays, int samples, float density, float radius)
{
	mOptions.finalgatherRays = rays;
	mOptions.finalgatherSamples = samples;
	mOptions.finalgatherDensity = density;
	mOptions.finalgatherRadius = radius;
}

void Context::setFinalgatherFalloff(int falloff)
{
	mOptions.finalgatherFalloff = falloff;
}

void Context::setFinalgatherFalloffRange(float start, float stop)
{
	mOptions.finalgatherFalloffRange[0] = start;
	mOptions.finalgatherFalloffRange[1] = stop;
}

void Context::setFinalgatherFilter(float size)
{
	mOptions.finalgatherFilter = size;
}

void Context::setFinalgatherTraceDepth(int reflect, int refract, int diffuse, int sum)
{
	mOptions.finalgatherTraceDepth[0] = reflect;
	mOptions.finalgatherTraceDepth[1] = refract;
	mOptions.finalgatherTraceDepth[2] = diffuse;
	mOptions.finalgatherTraceDepth[3] = sum;
}

void Context::setFinalgatherScale(float r, float g, float b)
{
	mOptions.finalgatherScale[0] = r;
	mOptions.finalgatherScale[1] = g;
	mOptions.finalgatherScale[2] = b;
}

void Context::setExposure(float gain, float gamma)
{
	mOptions.exposure[0] = gain;
	mOptions.exposure[1] = gamma;
}

void Context::setQuantize(float one, float min, float max, float dither_amp)
{
	mOptions.quantize[0] = one;
	mOptions.quantize[1] = min;
	mOptions.quantize[2] = max;
	mOptions.quantize[3] = dither_amp;
}

void Context::setFace(char* name)
{
	mOptions.face = name;

	free(name);
}

/////////////////////////////////////////////////////////

void Context::createLink(char* name)
{
	mLink.name = name;

	if (mFile)
	{
		*mFile
		<< "link \"" << mLink.name << "\"" << std::endl;
	}

	ei_link(mLink.name.c_str());

	free(name);
}

void Context::createDelete(char* name)
{
	mDelete.name = name;

	if (mFile)
	{
		*mFile
		<< "delete \"" << mDelete.name << "\"" << std::endl;
	}

	ei_delete(mDelete.name.c_str());

	free(name);
}

void Context::createRender(char* world, char* cameraInstance, char* optionsInstance)
{
	mRender.world = world;
	mRender.cameraInstance = cameraInstance;
	mRender.optionsInstance = optionsInstance;

	if (mFile)
	{
		*mFile
		<< "render \"" << mRender.world << "\" \"" << mRender.cameraInstance << "\" \"" << mRender.optionsInstance << "\"" << std::endl;
	}

	ei_render(mRender.world.c_str(), mRender.cameraInstance.c_str(), mRender.optionsInstance.c_str());

	free(world);
	free(cameraInstance);
	free(optionsInstance);
}

/////////////////////////////////////////////////////////

void Context::resetShader()
{
	mShader.paramInts.clear();
	mShader.paramScalars.clear();
	mShader.paramStrings.clear();
	mShader.paramVectors.clear();
	mShader.paramVector4s.clear();
	mShader.paramTags.clear();
	mShader.paramTextures.clear();
	mShader.paramIndices.clear();
	mShader.paramBools.clear();
	mShader.paramLinkages.clear();
}

void Context::createShader(char* name)
{
	mShader.name = name;

	if (mFile)
	{
		*mFile
		<< "shader \"" << mShader.name << "\"" << std::endl;
		for (std::map<std::string, int>::const_iterator itr = mShader.paramInts.begin(); itr != mShader.paramInts.end(); ++ itr)
		{
			*mFile
				<< "\tparam_int \"" << itr->first << "\" " << itr->second << std::endl;
		}
		for (std::map<std::string, std::string>::const_iterator itr = mShader.paramStrings.begin(); itr != mShader.paramStrings.end(); ++ itr)
		{
			*mFile
				<< "\tparam_string \"" << itr->first << "\" \"" << itr->second << "\"" << std::endl;
		}
		for (std::map<std::string, float>::const_iterator itr = mShader.paramScalars.begin(); itr != mShader.paramScalars.end(); ++ itr)
		{
			*mFile
				<< "\tparam_scalar \"" << itr->first << "\" " << itr->second << std::endl;
		}
		for (std::map<std::string, Shader::Vec3>::const_iterator itr = mShader.paramVectors.begin(); itr != mShader.paramVectors.end(); ++ itr)
		{
			*mFile
				<< "\tparam_vector \"" << itr->first << "\" " << itr->second << std::endl;
		}
		for (std::map<std::string, Shader::Vec4>::const_iterator itr = mShader.paramVector4s.begin(); itr != mShader.paramVector4s.end(); ++ itr)
		{
			*mFile
				<< "\tparam_vector4 \"" << itr->first << "\" " << itr->second << std::endl;
		}
		for (std::map<std::string, eiTag>::const_iterator itr = mShader.paramTags.begin(); itr != mShader.paramTags.end(); ++ itr)
		{
			*mFile
				<< "\tparam_tag \"" << itr->first << "\" " << itr->second << std::endl;
		}
		for (std::map<std::string, std::string>::const_iterator itr = mShader.paramTextures.begin(); itr != mShader.paramTextures.end(); ++ itr)
		{
			*mFile
				<< "\tparam_texture \"" << itr->first << "\" " << itr->second << std::endl;
		}
		for (std::map<std::string, eiIndex>::const_iterator itr = mShader.paramIndices.begin(); itr != mShader.paramIndices.end(); ++ itr)
		{
			*mFile
				<< "\tparam_index \"" << itr->first << "\" " << itr->second << std::endl;
		}
		for (std::map<std::string, eiBool>::const_iterator itr = mShader.paramBools.begin(); itr != mShader.paramBools.end(); ++ itr)
		{
			*mFile
				<< "\tparam_bool \"" << itr->first << "\" " << itr->second << std::endl;
		}
		for (std::set<Shader::ParamLinkage>::const_iterator itr = mShader.paramLinkages.begin(); itr != mShader.paramLinkages.end(); ++ itr)
		{
			*mFile
				<< "\tlink_param \"" << itr->paramName << "\" \"" << itr->srcShaderName << "\" \"" << itr->srcParamName << std::endl;
		}
	
		*mFile
		<< "end shader\n" << std::endl;
	}
	
	ei_shader(mShader.name.c_str());
	for (std::map<std::string, int>::const_iterator itr = mShader.paramInts.begin(); itr != mShader.paramInts.end(); ++ itr)
	{
		ei_shader_param_int(itr->first.c_str(), itr->second);
	}
	for (std::map<std::string, std::string>::const_iterator itr = mShader.paramStrings.begin(); itr != mShader.paramStrings.end(); ++ itr)
	{
		ei_shader_param_string(itr->first.c_str(), itr->second.c_str());
	}
	for (std::map<std::string, float>::const_iterator itr = mShader.paramScalars.begin(); itr != mShader.paramScalars.end(); ++ itr)
	{
		ei_shader_param_scalar(itr->first.c_str(), itr->second);
	}
	for (std::map<std::string, Shader::Vec3>::const_iterator itr = mShader.paramVectors.begin(); itr != mShader.paramVectors.end(); ++ itr)
	{
		ei_shader_param_vector(itr->first.c_str(), itr->second.x, itr->second.y, itr->second.z);
	}
	for (std::map<std::string, Shader::Vec4>::const_iterator itr = mShader.paramVector4s.begin(); itr != mShader.paramVector4s.end(); ++ itr)
	{
		ei_shader_param_vector4(itr->first.c_str(), itr->second.x, itr->second.y, itr->second.z, itr->second.w);
	}
	for (std::map<std::string, eiTag>::const_iterator itr = mShader.paramTags.begin(); itr != mShader.paramTags.end(); ++ itr)
	{
		ei_shader_param_tag(itr->first.c_str(), itr->second);
	}
	for (std::map<std::string, std::string>::const_iterator itr = mShader.paramTextures.begin(); itr != mShader.paramTextures.end(); ++ itr)
	{
		ei_shader_param_texture(itr->first.c_str(), itr->second.c_str());
	}
	for (std::map<std::string, eiIndex>::const_iterator itr = mShader.paramIndices.begin(); itr != mShader.paramIndices.end(); ++ itr)
	{
		ei_shader_param_index(itr->first.c_str(), itr->second);
	}
	for (std::map<std::string, eiBool>::const_iterator itr = mShader.paramBools.begin(); itr != mShader.paramBools.end(); ++ itr)
	{
		ei_shader_param_bool(itr->first.c_str(), itr->second);
	}
	for (std::set<Shader::ParamLinkage>::const_iterator itr = mShader.paramLinkages.begin(); itr != mShader.paramLinkages.end(); ++ itr)
	{
		ei_shader_link_param(itr->paramName.c_str(), itr->srcShaderName.c_str(), itr->srcParamName.c_str());
	}
	ei_end_shader();
	
	resetShader();

	free(name);
}

void Context::setParamInt(char* name, int x)
{
	mShader.paramInts.insert( std::make_pair(name, x) );

	free(name);
}

void Context::setParamScalar(char* name, float x)
{
	mShader.paramScalars.insert( std::make_pair(name, x) );

	free(name);
}

void Context::setParamString(char* name, char* content)
{
	mShader.paramStrings.insert( std::make_pair(name, content) );

	free(name);
	free(content);
}

void Context::setParamVector(char* name, float x, float y, float z)
{
	Shader::Vec3 v = {x, y, z};
	mShader.paramVectors.insert( std::make_pair(name, v) );
	
	free(name);
}

void Context::setParamVector4(char* name, float x, float y, float z, float w)
{
	Shader::Vec4 v = {x, y, z, w};
	mShader.paramVector4s.insert( std::make_pair(name, v) );
	
	free(name);
}

void Context::setParamTag(char* name, int x)
{
	mShader.paramTags.insert( std::make_pair(name, (eiTag)x) );

	free(name);
}

void Context::setParamTexture(char* name, char* texture)
{
	mShader.paramTextures.insert( std::make_pair(name, texture) );

	free(name);
	free(texture);
}

void Context::setParamIndex(char* name, int x)
{
	mShader.paramIndices.insert( std::make_pair(name, (eiIndex)x) );

	free(name);
}

void Context::setParamBool(char* name, int x)
{
	mShader.paramBools.insert( std::make_pair(name, (eiBool)x) );

	free(name);
}

void Context::setLinkParam(char* paramName, char* srcShaderName, char* srcParamName)
{
	Shader::ParamLinkage linkage;
	linkage.paramName = paramName;
	linkage.srcShaderName = srcShaderName;
	linkage.srcParamName = srcParamName;

	mShader.paramLinkages.insert(linkage);

	free(paramName);
	free(srcShaderName);
	free(srcParamName);
}

/////////////////////////////////////////////////////////

void Context::resetLight()
{
	mLight.lightList.clear();
	mLight.emitterList.clear();

	mLight.origin[0] = mLight.origin[1] = mLight.origin[2] = 0.0f;
	mLight.energy[0] = mLight.energy[1] = mLight.energy[2] = 10000.0f;
	mLight.areaSamples[0] = mLight.areaSamples[1] = 1; mLight.areaSamples[2] = eiMAX_INT; mLight.areaSamples[3] = mLight.areaSamples[4] = 1;
}

void Context::createLight(char* name)
{
	mLight.name = name;

	if (mFile)
	{
		*mFile
		<< "light \"" << mLight.name << "\"" << std::endl;
		for (std::set<std::string>::const_iterator itr = mLight.lightList.begin(); itr != mLight.lightList.end(); ++itr)
		{
			*mFile
				<< "\tadd_light \"" << *itr << "\"" << std::endl;
		}
		for (std::set<std::string>::const_iterator itr = mLight.emitterList.begin(); itr != mLight.emitterList.end(); ++itr)
		{
			*mFile
				<< "\tadd_emitter \"" << *itr << "\"" << std::endl;
		}
		*mFile
			<< "\torigin " << mLight.origin[0] << ' ' << mLight.origin[1] << ' ' << mLight.origin[2] << std::endl
			<< "\tenergy " << mLight.energy[0] << ' ' << mLight.energy[1] << ' ' << mLight.energy[2] << std::endl
			<< "\tarea_samples " << mLight.areaSamples[0] << ' ' << mLight.areaSamples[1] << ' ' << mLight.areaSamples[2] << ' ' << mLight.areaSamples[3] << ' ' << mLight.areaSamples[4] << std::endl
		<< "end light\n" << std::endl;
	}
	
	ei_light(mLight.name.c_str());
		for (std::set<std::string>::const_iterator itr = mLight.lightList.begin(); itr != mLight.lightList.end(); ++itr)
		{
			ei_add_light(itr->c_str());
		}
		for (std::set<std::string>::const_iterator itr = mLight.emitterList.begin(); itr != mLight.emitterList.end(); ++itr)
		{
			ei_add_emitter(itr->c_str());
		}
		ei_origin(mLight.origin[0], mLight.origin[1], mLight.origin[2]);
		ei_energy(mLight.energy[0], mLight.energy[1], mLight.energy[2]);
		ei_area_samples(mLight.areaSamples[0], mLight.areaSamples[1], mLight.areaSamples[2], mLight.areaSamples[3], mLight.areaSamples[4]);
	ei_end_light();
	
	resetLight();

	free(name);
}

void Context::setLight(char* light)
{
	mLight.lightList.insert(light);

	free(light);
}

void Context::setEmitter(char* emitter)
{
	mLight.emitterList.insert(emitter);

	free(emitter);
}

void Context::setOrigin(float x, float y, float z)
{
	mLight.origin[0] = x;
	mLight.origin[1] = y;
	mLight.origin[2] = z;
}

void Context::setEnergy(float r, float g, float b)
{
	mLight.energy[0] = r;
	mLight.energy[1] = g;
	mLight.energy[2] = b;
}

void Context::setAreaSamples(int uSamples, int vSamples, int lowLevel, int lowUSamples, int lowVSamples)
{
	mLight.areaSamples[0] = uSamples;
	mLight.areaSamples[1] = vSamples;
	mLight.areaSamples[2] = lowLevel;
	mLight.areaSamples[3] = lowUSamples;
	mLight.areaSamples[4] = lowVSamples;
}

/////////////////////////////////////////////////////////

void Context::resetMaterial()
{
	mMaterial.surfaceList.clear();
	mMaterial.displaceList.clear();
	mMaterial.shadowList.clear();
	mMaterial.volumeList.clear();
	mMaterial.envList.clear();
	mMaterial.photonList.clear();
}

void Context::createMaterial(char* name)
{
	mMaterial.name = name;

	if (mFile)
	{
		*mFile
		<< "material \"" << mMaterial.name << "\"" << std::endl;
		for (std::set<std::string>::const_iterator itr = mMaterial.surfaceList.begin(); itr != mMaterial.surfaceList.end(); ++itr)
		{
			*mFile
				<< "\tadd_surface \"" << *itr << "\"" << std::endl;
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.displaceList.begin(); itr != mMaterial.displaceList.end(); ++itr)
		{
			*mFile
				<< "\tadd_displace \"" << *itr << "\"" << std::endl;
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.shadowList.begin(); itr != mMaterial.shadowList.end(); ++itr)
		{
			*mFile
				<< "\tadd_shadow \"" << *itr << "\"" << std::endl;
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.volumeList.begin(); itr != mMaterial.volumeList.end(); ++itr)
		{
			*mFile
				<< "\tadd_volume \"" << *itr << "\"" << std::endl;
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.envList.begin(); itr != mMaterial.envList.end(); ++itr)
		{
			*mFile
				<< "\tadd_environment \"" << *itr << "\"" << std::endl;
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.photonList.begin(); itr != mMaterial.photonList.end(); ++itr)
		{
			*mFile
				<< "\tadd_photon \"" << *itr << "\"" << std::endl;
		}
		*mFile
		<< "end material\n" << std::endl;
	}
	
	ei_material(mMaterial.name.c_str());
		for (std::set<std::string>::const_iterator itr = mMaterial.surfaceList.begin(); itr != mMaterial.surfaceList.end(); ++itr)
		{
			ei_add_surface(itr->c_str());
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.displaceList.begin(); itr != mMaterial.displaceList.end(); ++itr)
		{
			ei_add_displace(itr->c_str());
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.shadowList.begin(); itr != mMaterial.shadowList.end(); ++itr)
		{
			ei_add_shadow(itr->c_str());
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.volumeList.begin(); itr != mMaterial.volumeList.end(); ++itr)
		{
			ei_add_volume(itr->c_str());
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.envList.begin(); itr != mMaterial.envList.end(); ++itr)
		{
			ei_add_environment(itr->c_str());
		}
		for (std::set<std::string>::const_iterator itr = mMaterial.photonList.begin(); itr != mMaterial.photonList.end(); ++itr)
		{
			ei_add_photon(itr->c_str());
		}
	ei_end_material();
	
	resetMaterial();

	free(name);
}

void Context::setSurface(char* surface)
{
	mMaterial.surfaceList.insert(surface);
	
	free(surface);
}

void Context::setDisplace(char* displace)
{
	mMaterial.displaceList.insert(displace);
	
	free(displace);
}

void Context::setShadow(char* shadow)
{
	mMaterial.shadowList.insert(shadow);

	free(shadow);
}

void Context::setVolume(char* volume)
{
	mMaterial.volumeList.insert(volume);
	
	free(volume);
}

void Context::setEnvironment(char* env)
{
	mMaterial.envList.insert(env);
	
	free(env);
}

void Context::setPhoton(char* photon)
{
	mMaterial.photonList.insert(photon);
	
	free(photon);
}

/////////////////////////////////////////////////////////

void Context::resetTexture()
{
	mTexture.fileTexture = "";
	mTexture.local = 0;
}

void Context::createTexture(char* name)
{
	mTexture.name = name;

	if (mFile)
	{
		*mFile
		<< "texture \"" << mTexture.name << "\"" << std::endl;
		*mFile
			<< "\tfile_texture \"" << mTexture.fileTexture << "\" " << mTexture.local << std::endl;
		*mFile
		<< "end texture\n" << std::endl;
	}
	
	ei_texture(mTexture.name.c_str());
		ei_file_texture(mTexture.fileTexture.c_str(), mTexture.local);
	ei_end_texture();

	resetTexture();

	free(name);
}

void Context::setFileTexture(char* filename, int local)
{
	mTexture.fileTexture = filename;
	mTexture.local = local;

	free(filename);
}

/////////////////////////////////////////////////////////

void Context::resetInstGroup()
{
	mInstGroup.instances.clear();
}

void Context::createInstGroup(char* name)
{
	mInstGroup.name = name;

	if (mFile)
	{
		*mFile
		<< "instgroup \"" << mInstGroup.name << "\"" << std::endl;
			for (std::set<std::string>::const_iterator itr = mInstGroup.instances.begin(); itr != mInstGroup.instances.end(); ++ itr)
			{
				*mFile
					<< "\tadd_instance \"" << *itr << "\"" << std::endl;
			}
		*mFile
		<< "end instgroup\n" << std::endl;
	}
	
	ei_instgroup(mInstGroup.name.c_str());
	for (std::set<std::string>::const_iterator itr = mInstGroup.instances.begin(); itr != mInstGroup.instances.end(); ++ itr)
	{
		ei_add_instance(itr->c_str());
	}
	ei_end_instgroup();
	
	resetInstGroup();
	
	free(name);
}

void Context::setInstance(char* instance)
{
	mInstGroup.instances.insert(instance);

	free(instance);
}
