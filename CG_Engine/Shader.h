#pragma once

#include <vector>
#include "Common.h"
#include "CG_Data.h"
#include <map>
namespace GL_Engine{
	class Shader
	{
	public:

		Shader();
		~Shader();
		void Cleanup();
		
		const GLuint GetShaderID() const;

		//Compile a shader stage from a source text file. Returns Shader ID
		const uint8_t CompileShader();

		//Register a shader file to the pipeline
		bool RegisterShaderStageFromFile(const char *_FilePath, GLenum _StageType);

		//Register a shader source to the pipeline
		void RegisterShaderStage(const char* _ShaderSource, GLenum _StageType);

		//Register a shader attribute, to be bound at _Location
		void RegisterAttribute(const char* _AttributeName, GLuint _Location);

		//Register a shader attribute, to be bound at _Location
		void RegisterTextureUnit(std::string _AttributeName, GLuint _Location);

		//Activate the program
		void UseShader() const;

		//Register a Uniform.
		//Returns pointer to UBO (Object finalised after call to compile)
		CG_Data::Uniform* RegisterUniform(const char* _UniformName);

		CG_Data::Uniform* RegisterUniform(const char* _UniformName, std::function<void(const CG_Data::Uniform&)> _CallbackFunction);

		void RegisterUBO(std::string &_UBO_Name, CG_Data::UBO *_ubo);
		CG_Data::Uniform* GetUniform(uint8_t index) const;
		CG_Data::Uniform* GetUniform(std::string _uName);


		void UpdateUniforms();

		bool Initialised() const;

	private:
		struct ShaderStage {
			const char* Source;
			GLenum Type;
			GLuint ID;
		};
		struct Attribute {
			const char* AttributeName;
			GLuint Location;
		};
		struct UniformStruct{
			const char *Name;
			CG_Data::Uniform *UniformObject;
		};
		struct UBO_Struct {
			CG_Data::UBO *ubo;
			GLuint BlockIndex;
		};
		std::vector<ShaderStage*> shaderStages;
		std::vector<Attribute*> Attributes;
		std::vector<UniformStruct*> Uniforms;
		std::map<std::string, CG_Data::Uniform*> UniformMap;
		std::map<std::string, UBO_Struct> UBO_BlockIndices;
		std::map<std::string, GLuint> TextureLocations;

		GLuint ShaderID;
		bool initialised{ false };
		const GLuint CompileShaderStage(ShaderStage *stage);
	};

}