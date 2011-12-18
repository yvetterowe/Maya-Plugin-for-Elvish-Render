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

#ifndef EI_PARSER_CONTEXT_HPP
#define EI_PARSER_CONTEXT_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

#include <eiAPI/ei.h>

class Context
{
public:
	Context();
	~Context();

	void echo(std::ostream* file);

	void resetCamera();
	void createCamera(char* name);
	void setOutput(char* name, char* fileFormat, char* dataType);
	void setOutputVariable(char* variable, char* dataType);
	void setFocal(float focal);
	void setAperture(float aperture);
	void setAspect(float aspect);
	void setResolution(int width, int height);
	void setWindow(int xmin, int xmax, int ymin, int ymax);
	void setClip(float hither, float yon);
	void setLens(char* name);
	void setImager(char* name);

	void resetInstance();
	void createInstance(char* name);
	void setMaterial(char* material);
	void setElement(char* element);
	void setTransform(float* transform);
	void setMotionTransform(float* transform);
	
	void resetObject();
	void createObject(char* name, char* type);
	void setPositionCount(int positionCount);
	void appendPosition(float x, float y, float z);
	void setMotionPositionCount(int positionCount);
	void appendMotionPosition(float x, float y, float z);
	void setNormalCount(int normalCount);
	void appendNormal(float x, float y, float z);
	void setUVCount(int uvCount);
	void appendUV(float u, float v);
	void setTriangleCount(int triangleCount);
	void appendTriangle(int a, int b, int c);

	void resetOptions();
	void createOptions(char* name);
	void setSamples(int minimal, int maximal);
	void setContrast(float r, float g, float b, float a);
	void setBucketSize(int size);
	void setFilter(char* filter, float filterWidth);
	void setMaxDisplace(float dist);
	void setShutter(float open, float close);
	void setMotion(int motion);
	void setMotionSegments(int num);
	void setTraceDepth(int reflect, int refract, int sum);
	void setCaustic(int caustic);
	void setCausticPhotons(int num);
	void setCausticAccuracy(int samples, float radius);
	void setCausticScale(float r, float g, float b);
	void setCausticFilter(char* name, float c);
	void setPhotonTraceDepth(int reflect, int refract, int sum);
	void setPhotonDecay(float decay);
	void setGlobillum(int globillum);
	void setGlobillumPhotons(int num);
	void setGlobillumAccuracy(int samples, float radius);
	void setGlobillumScale(float r, float g, float b);
	void setFinalgather(int finalgather);
	void setFinalgatherAccuracy(int rays, int samples, float density, float radius);
	void setFinalgatherFalloff(int falloff);
	void setFinalgatherFalloffRange(float start, float stop);
	void setFinalgatherFilter(float size);
	void setFinalgatherTraceDepth(int reflect, int refract, int diffuse, int sum);
	void setFinalgatherScale(float r, float g, float b);
	void setExposure(float gain, float gamma);
	void setQuantize(float one, float min, float max, float dither_amp);
	void setFace(char* name);
	
	void resetShader();
	void createShader(char* name);
	void setParamInt(char* name, int x);
	void setParamScalar(char* name, float x);
	void setParamString(char* name, char* content);
	void setParamVector(char* name, float x, float y, float z);
	void setParamVector4(char* name, float x, float y, float z, float w);
	void setParamTag(char* name, int x);
	void setParamTexture(char* name, char* texture);
	void setParamIndex(char* name, int x);
	void setParamBool(char* name, int x);
	void setLinkParam(char* paramName, char* srcShaderName, char* srcParamName);
	
	void resetLight();
	void createLight(char* name);
	void setLight(char* light);
	void setEmitter(char* emitter);
	void setOrigin(float x, float y, float z);
	void setEnergy(float r, float g, float b);
	void setAreaSamples(int uSamples, int vSamples, int lowLevel, int lowUSamples, int lowVSamples);

	void resetMaterial();
	void createMaterial(char* name);
	void setSurface(char* surface);
	void setDisplace(char* displace);
	void setShadow(char* shadow);
	void setVolume(char* volume);
	void setEnvironment(char* env);
	void setPhoton(char* photon);

	void resetTexture();
	void createTexture(char* name);
	void setFileTexture(char* filename, int local);
	
	void resetInstGroup();
	void createInstGroup(char* name);
	void setInstance(char* instance);
	
	void createLink(char* name);
	void createDelete(char* name);
	void createRender(char* world, char* cameraInstance, char* optionsInstance);

private:
	struct Camera
	{
		struct Output
		{
			std::string name;
			std::string fileFormat;
			std::string dataType;

			bool operator < (const Output& rhs) const
			{
				return (name < rhs.name) || (fileFormat < rhs.fileFormat) || (dataType < rhs.dataType);
			}
		};

		struct OutputVariable
		{
			std::string variable;
			std::string dataType;

			bool operator < (const OutputVariable& rhs) const
			{
				return (variable < rhs.variable) || (dataType < rhs.dataType);
			}
		};

		typedef std::set<OutputVariable>	OutputVariableList;

		std::string name;
		std::map<Output, OutputVariableList>	outputList;
		OutputVariableList		outputVariableList;
		std::set<std::string>	lensList;
		std::set<std::string>	imagerList;
		float       aperture;
		float       aspect;
		float       focal;
		int         resolution[2];
		int			window[4];
		float		clip[2];
	};
	
	struct Instance
	{
		std::string           element;
		std::set<std::string> materials;
		std::string           name;
		float                 transform[16];
		float				  motionTransform[16];
	};
	
	struct InstGroup
	{
		std::string           name;
		std::set<std::string> instances;
	};
	
	struct Light
	{
		std::string name;
		std::set<std::string> lightList;
		std::set<std::string> emitterList;
		float       origin[3];
		float		energy[3];
		int			areaSamples[5];
	};
	
	struct Material
	{
		std::string name;
		std::set<std::string> surfaceList;
		std::set<std::string> displaceList;
		std::set<std::string> shadowList;
		std::set<std::string> volumeList;
		std::set<std::string> envList;
		std::set<std::string> photonList;
	};

	struct Texture
	{
		std::string name;
		std::string fileTexture;
		int			local;
	};

	struct Options
	{
		std::string name;
		int         samples[2];
		float       contrast[4];
		std::string filter;
		int			bucketSize;
		float       filterWidth;
		float       maxDisplace;
		float       shutter[2];
		int         motion;
		int			motionSegments;
		int         traceDepth[3];
		int			caustic;
		int			causticPhotons;
		int			causticSamples;
		float		causticRadius;
		float		causticScale[3];
		std::string	causticFilter;
		float		causticFilterConst;
		int			photonTraceDepth[3];
		float		photonDecay;
		int			globillum;
		int			globillumPhotons;
		int			globillumSamples;
		float		globillumRadius;
		float		globillumScale[3];
		int			finalgather;
		int			finalgatherRays;
		int			finalgatherSamples;
		float		finalgatherDensity;
		float		finalgatherRadius;
		int			finalgatherFalloff;
		float		finalgatherFalloffRange[2];
		float		finalgatherFilter;
		int			finalgatherTraceDepth[4];
		float		finalgatherScale[3];
		float		exposure[2];
		float		quantize[4];
		std::string	face;
	};

	struct Link
	{
		std::string name;
	};

	struct Delete
	{
		std::string name;
	};
	
	struct Render
	{
		std::string world;
		std::string cameraInstance;
		std::string optionsInstance;
	};
	
	struct Shader
	{
		struct Vec3
		{
			float x, y, z;
			friend std::ostream& operator<<(std::ostream& os, const Vec3& triple)
			{
				os << triple.x << ' ' << triple.y << ' ' << triple.z;
				return os;
			}
		};
		struct Vec4
		{
			float x, y, z, w;
			friend std::ostream& operator<<(std::ostream& os, const Vec4& q)
			{
				os << q.x << ' ' << q.y << ' ' << q.z << ' ' << q.w;
				return os;
			}
		};
		struct ParamLinkage
		{
			std::string paramName;
			std::string srcShaderName;
			std::string srcParamName;
			friend std::ostream& operator<<(std::ostream& os, const ParamLinkage& l)
			{
				os << l.paramName << ' ' << l.srcShaderName << ' ' << l.srcParamName;
				return os;
			}
			bool operator < (const ParamLinkage& rhs) const
			{
				return (paramName < rhs.paramName) || (srcShaderName < rhs.srcShaderName) || (srcParamName < rhs.srcParamName);
			}
		};
		std::string                        name;
		std::map<std::string, int>		   paramInts;
		std::map<std::string, float>       paramScalars;
		std::map<std::string, std::string> paramStrings;
		std::map<std::string, Vec3>        paramVectors;
		std::map<std::string, Vec4>		   paramVector4s;
		std::map<std::string, eiTag>	   paramTags;
		std::map<std::string, std::string> paramTextures;
		std::map<std::string, eiIndex>	   paramIndices;
		std::map<std::string, eiBool>	   paramBools;
		std::set<ParamLinkage>			   paramLinkages;
	};
	
	struct Object
	{
		std::string        name;
		std::string        type;
		std::vector<float> posList;
		std::vector<float> motionPosList;
		std::vector<float> nrmList;
		std::vector<float> uvList;
		std::vector<int>   triangleList;
	};
	
	Camera    mCamera;
	Light     mLight;
	Instance  mInstance;
	InstGroup mInstGroup;
	Material  mMaterial;
	Texture	  mTexture;
	Object    mObject;
	Options   mOptions;
	Link	  mLink;
	Delete	  mDelete;
	Render    mRender;
	Shader    mShader;

	std::ostream* mFile;
};



#endif
