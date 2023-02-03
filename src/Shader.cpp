#include "Shader.h"
#include "File_IO.h"
#include <stdexcept>

namespace GL_Engine{

    std::list<GLuint> Shader::ShaderStack;

    Shader::Shader(){
    //    this->uboBlockIndices = std::map< std::string, 
    //                                      std::unique_ptr< UboStruct > >();
    }


    Shader::~Shader(){
        if (initialised) {
            glDeleteProgram(this->shaderID);
            initialised = false;
        }
    }

    void Shader::cleanup() {
        if( initialised ) {
            glDeleteProgram( this->shaderID );
            initialised = false;
        }
    }

    const GLuint Shader::getShaderID() const {
        return this->shaderID;
    }

    const GLuint Shader::compileShader(){
        this->shaderID = glCreateProgram();
        if ( !this->shaderID ) {
            throw std::runtime_error( "Error generating Shader ID!" );
            return 0;
        }

        for ( auto attrib : this->attributes ){
            glBindAttribLocation( this->shaderID, attrib->location,
                                  attrib->attributeName.c_str() );
        }

        // Attach each compiled stage to the Shader Program
        for ( auto stage : this->shaderStages ){
            this->compileShaderStage( stage );
            glAttachShader( this->shaderID, stage->id );
        }

        glLinkProgram( this->shaderID );
        GLchar errorBuffer[ 1024 ] = { 0 };
        GLint result;
        glGetProgramiv( this->shaderID, GL_LINK_STATUS, &result );
        if ( !result ) {
            glGetProgramInfoLog( this->shaderID, sizeof( errorBuffer ), NULL, 
                                 errorBuffer );
            throw std::runtime_error( "Error linking Shader\n" +
                                      std::string( errorBuffer ) );
            return 0;
        }

        for ( auto uniform : this->uniforms ){
            uniform->uniformObject->SetID( 
                    glGetUniformLocation( shaderID, uniform->name.c_str() ) );
            this->uniformMap[ uniform->name ] = uniform->uniformObject;
        }

        for ( auto & ubo : uboBlockIndices ) {
            ubo.second->blockIndex = glGetUniformBlockIndex( this->shaderID,
                                                            ubo.first.c_str() );
        }

        glUseProgram( this->shaderID );
        for( auto tex : textureLocations ){
            glUniform1i( glGetUniformLocation( this->shaderID, 
                                               tex.first.c_str() ),
                         tex.second );
        }

        for ( auto attrib : this->attributes ){
            GLuint location = glGetAttribLocation( this->shaderID, 
                                                   attrib->attributeName.c_str()
                                                 );
            if( location != attrib->location ){
                std::cout << "Warning! Intended location of attribute " <<
                             attrib->attributeName << " (" << attrib->location
                             << ") does not match actual location (" <<
                             location << ")." << std::endl;
            }
            delete attrib;
        }
        attributes.clear();
        

        for ( auto stage : this->shaderStages ){
            glDeleteShader( stage->id );
            delete stage; //Stages no longer needed, so clean them up
        }
        shaderStages.clear();

        unsigned int tempVao;
        glGenVertexArrays( 1, &tempVao );
        glBindVertexArray( tempVao );

        glValidateProgram( this->shaderID );
        glDeleteVertexArrays( 1, &tempVao );
        glGetProgramiv( this->shaderID, GL_VALIDATE_STATUS, &result );
        if ( !result ) {
            glGetProgramInfoLog( this->shaderID, sizeof( errorBuffer ),
                                 NULL, errorBuffer );
            throw std::runtime_error( "Error linking Shader!\n" +
                                     std::string( errorBuffer ) );
            return 0;
        }
        
        initialised = true;
        return this->shaderID ;
    }

    bool Shader::registerShaderStageFromFile( 
		const std::filesystem::path & _filePath, GLenum _stageType ){
        uint8_t fileLoadResult;
        std::string shaderText = File_IO::loadTextFile( _filePath,
                                                        & fileLoadResult );
        if ( fileLoadResult || shaderText.empty() ) {
            throw std::runtime_error( "Error loading shader file " +
                                      std::string( _filePath.string() ) );
        }

        this->registerShaderStage( std::move( shaderText ), _stageType );
        return true;
    }

    void Shader::registerShaderStage( const std::string &_shaderSource,
                                      GLenum _stageType ){
        ShaderStage *stage = new ShaderStage;
        stage->source = std::move( _shaderSource );
        stage->type = _stageType;
        this->shaderStages.push_back( stage );
        return;
    }

    void Shader::registerAttribute( const std::string & _attributeName,
                                    GLuint _location ){
        Attribute *attrib = new Attribute;
        attrib->attributeName = _attributeName;
        attrib->location = _location;
        this->attributes.push_back( attrib );
        return;
    }

    Shader::ShaderBindToken Shader::useShader() const {
        if ( uboBlockIndices.size() > 0 ) {
            for ( auto &ubo : uboBlockIndices ) {
                auto bPost = ubo.second->ubo->GetBindingPost();
                glUniformBlockBinding( this->shaderID, ubo.second->blockIndex,
                    bPost );
            }
        }
        return Shader::staticBind( shaderID );
    }

    std::shared_ptr< CG_Data::Uniform >
    Shader::registerUniform( const std::string & _uniformName ){
        UniformStruct *uniform = new UniformStruct;
        uniform->uniformObject = std::make_shared< CG_Data::Uniform >();
        uniform->name = _uniformName;
        this->uniforms.push_back( uniform );
        return uniform->uniformObject;
    }
    std::shared_ptr< CG_Data::Uniform >
    Shader::registerUniform( const std::string & _uniformName, 
                             std::function< void( const CG_Data::Uniform & ) > 
                                _callbackFunction ){
        UniformStruct *uniform = new UniformStruct;
        uniform->uniformObject = std::make_shared< CG_Data::Uniform >();
        uniform->name = _uniformName;
        uniform->uniformObject->SetUpdateCallback( _callbackFunction );
        this->uniforms.push_back( uniform );
        return uniform->uniformObject;
    }

    const GLuint Shader::compileShaderStage( ShaderStage * stage ){
        stage->id = glCreateShader( stage->type );

        if ( stage->id == 0 ) {
            throw std::runtime_error( "Error creating shader stage!" );
            return InvalidShaderId;
        }
        //Bind the source to the Stage, and compile
		const char * shaderCStr = stage->source.c_str();
        glShaderSource( stage->id, 1, 
                        ( const GLchar** ) & shaderCStr, nullptr );
        glCompileShader( stage->id );
        GLint result;
        // check for shader related errors using glGetShaderiv
        glGetShaderiv( stage->id, GL_COMPILE_STATUS, &result );
        if( !result ){
            GLchar errorBuffer[1024];
            glGetShaderInfoLog( stage->id, 1024, nullptr, errorBuffer );
            throw std::runtime_error( "Error compiling shader stage!\n" +
                                      std::string( errorBuffer ) );
            return InvalidShaderId;
        }
        return stage->id;
    }

    //Register a shader attribute, to be bound at _Location
    void Shader::registerTextureUnit( const std::string & _attributeName,
                                      GLuint _location ){
        this->textureLocations[ _attributeName ] = _location;
    }

    void Shader::registerUBO( const std::string &_uboName,
                              std::shared_ptr< const CG_Data::UBO > _ubo ){
        auto uboStruct = std::make_unique< UboStruct>();
        uboStruct->ubo = _ubo;
        this->uboBlockIndices[ _uboName ] = std::move( uboStruct );
    }

    void Shader::registerUboPlaceholder( const std::string &_uboName ) {
        //this->uboBlockIndices[ _uboName ] = std::make_unique<UboStruct>( UboStruct { .ubo = nullptr } );
    }

    std::shared_ptr< CG_Data::Uniform > 
    Shader::getUniform(uint8_t index) const {
        return this->uniforms[ index ]->uniformObject;
    }
    std::shared_ptr< CG_Data::Uniform >
    Shader::getUniform( const std::string & _uName ) {
        try{
            return this->uniformMap.at( _uName );
        } catch( std::out_of_range ){
            throw std::runtime_error( "Error: Uniform \"" + _uName + 
            "\" does not exist for this shader" );
        }
    }


    void Shader::updateUniforms() {
        for ( auto u : this->uniforms ) {
            u->uniformObject->Update();
        }
    }

    bool Shader::isInitialised() const { 
        return this->initialised; 
    }

    


}
