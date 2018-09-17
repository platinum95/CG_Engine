#include "Shader.h"
#include "File_IO.h"
#include <stdexcept>

namespace GL_Engine{

	Shader::Shader(){
		this->UBO_BlockIndices = std::map<std::string, UBO_Struct>();
	}


	Shader::~Shader(){
		if (initialised) {
			glDeleteProgram(this->ShaderID);
			initialised = false;
		}
	}

	void Shader::Cleanup() {
		if (initialised) {
			glDeleteProgram(this->ShaderID);
			initialised = false;
		}
	}

	const GLuint Shader::GetShaderID() const {
		return ShaderID;
	}

	const uint8_t Shader::CompileShader(){
		this->ShaderID = glCreateProgram();
		if (!ShaderID) {
			throw std::runtime_error("Error generating Shader ID!");
			return 0;
		}

		for (auto Attrib : this->Attributes) {
			glBindAttribLocation(ShaderID, Attrib->Location, Attrib->AttributeName);
			delete Attrib;
		}
		Attributes.clear();

		// Attach each compiled stage to the Shader Program
		for (auto Stage : this->shaderStages){
			CompileShaderStage(Stage);
			glAttachShader(ShaderID, Stage->ID);
		}

		glLinkProgram(ShaderID);
		GLchar ErrorBuffer[1024] = { 0 };
		GLint Result;
		glGetProgramiv(ShaderID, GL_LINK_STATUS, &Result);
		if (!Result) {
			glGetProgramInfoLog(ShaderID, sizeof(ErrorBuffer), NULL, ErrorBuffer);
			throw std::runtime_error("Error linking Shader\n" + std::string(ErrorBuffer));
			return 0;
		}
                unsigned int tempVao;
                glGenVertexArrays( 1, &tempVao );
                glBindVertexArray( tempVao );
		glValidateProgram(ShaderID);
		glDeleteVertexArrays( 1, &tempVao );
		glGetProgramiv(ShaderID, GL_VALIDATE_STATUS, &Result);
		if (!Result) {
			glGetProgramInfoLog(ShaderID, sizeof(ErrorBuffer), NULL, ErrorBuffer);
			throw std::runtime_error("Error linking Shader!\n" + std::string(ErrorBuffer));
			return 0;
		}

		for (auto uni : Uniforms) {
			uni->UniformObject->SetID(glGetUniformLocation(ShaderID, uni->Name));
			UniformMap[uni->Name] = uni->UniformObject;
			//delete uni;
		}
		for (auto &ubo : UBO_BlockIndices) {
			ubo.second.BlockIndex = glGetUniformBlockIndex(this->ShaderID, ubo.first.c_str());
		}
		glUseProgram(this->ShaderID);
		for(auto tex : TextureLocations){
			glUniform1i(glGetUniformLocation(this->ShaderID, tex.first.c_str()), tex.second);
		}
		//UBOs.clear();
		

		for (auto Stage : this->shaderStages){
			glDeleteShader(Stage->ID);	//Stages no longer needed, so clean them up
			delete Stage;
		}
		shaderStages.clear();
		initialised = true;
		return ShaderID;
	}

	bool Shader::RegisterShaderStageFromFile(const char * _FilePath, GLenum _StageType){
		uint8_t FileLoadResult;
		const char* ShaderText = File_IO::LoadTextFile(_FilePath, &FileLoadResult);
		if (FileLoadResult || ShaderText == nullptr) {
			throw std::runtime_error("Error loading shader file " + std::string(_FilePath));
		}

		RegisterShaderStage(ShaderText, _StageType);
		return true;
	}
	void Shader::RegisterShaderStage(const char* _ShaderSource, GLenum _StageType) {
		ShaderStage *stage = new ShaderStage;
		stage->Source = _ShaderSource;
		stage->Type = _StageType;
		this->shaderStages.push_back(stage);
		return;
	}

	void Shader::RegisterAttribute(const char * _AttributeName, GLuint _Location){
		Attribute *attrib = new Attribute;
		attrib->AttributeName = _AttributeName;
		attrib->Location = _Location;
		Attributes.push_back(attrib);
		return;
	}

	void Shader::UseShader() const {
		if (UBO_BlockIndices.size() > 0) {
			for (auto ubo : UBO_BlockIndices) {
				glUniformBlockBinding(this->ShaderID, ubo.second.BlockIndex, ubo.second.ubo->GetBindingPost());
			}
		}
		glUseProgram(this->ShaderID);
	}

	CG_Data::Uniform * Shader::RegisterUniform(const char * _UniformName){
		UniformStruct *uniform = new UniformStruct;
		uniform->UniformObject = new CG_Data::Uniform();
		uniform->Name = _UniformName;
		this->Uniforms.push_back(uniform);
		return uniform->UniformObject;
	}
	CG_Data::Uniform * Shader::RegisterUniform(const char * _UniformName, std::function<void(const CG_Data::Uniform&)> _CallbackFunction) {
		UniformStruct *uniform = new UniformStruct;
		uniform->UniformObject = new CG_Data::Uniform();
		uniform->Name = _UniformName;
		uniform->UniformObject->SetUpdateCallback(_CallbackFunction);
		this->Uniforms.push_back(uniform);
		return uniform->UniformObject;
	}

	const GLuint Shader::CompileShaderStage(ShaderStage * stage){
		stage->ID = glCreateShader(stage->Type);

		if (stage->ID == 0) {
			throw std::runtime_error("Error creating shader stage!");
			return -1;
		}
		//Bind the source to the Stage, and compile
		glShaderSource(stage->ID, 1, (const GLchar**)&stage->Source, NULL);
		glCompileShader(stage->ID);
		GLint Result;
		// check for shader related errors using glGetShaderiv
		glGetShaderiv(stage->ID, GL_COMPILE_STATUS, &Result);
		if (!Result) {
			GLchar ErrorBuffer[1024];
			glGetShaderInfoLog(stage->ID, 1024, NULL, ErrorBuffer);
			throw std::runtime_error("Error compiling shader!\n" + std::string(ErrorBuffer));
			return -1;
		}
		return stage->ID;
	}

	//Register a shader attribute, to be bound at _Location
	void Shader::RegisterTextureUnit(std::string _AttributeName, GLuint _Location) {
		this->TextureLocations[_AttributeName] = _Location;
	}

	void Shader::RegisterUBO(const std::string &_UBO_Name, CG_Data::UBO *_ubo) {
		UBO_Struct ubo_struct;
		ubo_struct.ubo = _ubo;
		UBO_BlockIndices[_UBO_Name] = ubo_struct;
	}

	CG_Data::Uniform* Shader::GetUniform(uint8_t index) const {
		return Uniforms[index]->UniformObject;
	}
	CG_Data::Uniform* Shader::GetUniform(std::string _uName) {
		return UniformMap[_uName];
	}


	void Shader::UpdateUniforms() {
		for (auto u : this->Uniforms) {
			u->UniformObject->Update();
		}
	}

	bool Shader::Initialised() const { 
		return this->initialised; 
	}

	


}
