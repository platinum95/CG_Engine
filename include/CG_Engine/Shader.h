#pragma once

#include <vector>
#include "Common.h"
#include "CG_Data.h"
#include <map>
#include <filesystem>

namespace GL_Engine{
    class Shader
    {
    public:
        static constexpr GLuint InvalidShaderId = std::numeric_limits<GLint>::max();

        Shader();
        ~Shader();
        void cleanup();
        
        const GLuint getShaderID() const;

        //Compile a shader stage from a source text file. Returns Shader ID
        const GLuint compileShader();

        //Register a shader file to the pipeline
        bool registerShaderStageFromFile( 
			const std::filesystem::path & _filePath, GLenum _stageType );

        //Register a shader source to the pipeline
        void registerShaderStage( std::string &&_shaderSource, GLenum _stageType );

        //Register a shader attribute, to be bound at _Location
        void registerAttribute( const std::string & _attributeName,
                                GLuint _location);

        //Register a shader attribute, to be bound at _Location
        void registerTextureUnit( const std::string & _attributeName,
                                  GLuint _location);

        //Activate the program
        void useShader() const;

        //Register a Uniform.
        //Returns pointer to UBO (Object finalised after call to compile)
        std::shared_ptr< CG_Data::Uniform > 
        registerUniform( const std::string & _uniformName );

        std::shared_ptr< CG_Data::Uniform >
        registerUniform( const std::string & _uniformName, 
                         std::function< void( const CG_Data::Uniform & ) > 
                            _callbackFunction );

        void registerUBO( const std::string &_uboName,
                          std::shared_ptr< const CG_Data::UBO > _ubo );
        std::shared_ptr< CG_Data::Uniform >
        getUniform( uint8_t index ) const;

        std::shared_ptr< CG_Data::Uniform > 
        getUniform( const std::string & _uName );


        void updateUniforms();

        bool isInitialised() const;

    private:
        struct ShaderStage {
            std::string source;
            GLenum type;
            GLuint id;
        };
        struct Attribute {
            std::string attributeName;
            GLuint location;
        };
        struct UniformStruct{
            std::string name;
            std::shared_ptr< CG_Data::Uniform > uniformObject;
        };
        struct UboStruct {
            std::shared_ptr< const CG_Data::UBO > ubo;
            GLuint blockIndex;
        };
        std::vector< ShaderStage* > shaderStages;
        std::vector< Attribute* > attributes;
        std::vector< UniformStruct* > uniforms;
        std::map< std::string, std::shared_ptr< CG_Data::Uniform > > uniformMap;
        std::map< std::string, std::unique_ptr< UboStruct > > uboBlockIndices;
        std::map< std::string, GLuint > textureLocations;

        GLuint shaderID = InvalidShaderId;
        bool initialised{ false };
        const GLuint compileShaderStage( ShaderStage *stage );
    };

}
