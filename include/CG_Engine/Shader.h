#ifndef SHADER_H
#define SHADER_H

#include "CG_Data.h"
#include "Common.h"
#include "Entity.h"

#include <filesystem>
#include <map>
#include <vector>

namespace GL_Engine {

class Shader
{
private:
    static std::list<GLuint> ShaderStack;

    static void staticUnbind( GLuint id ) {
        const auto entry = std::find( ShaderStack.crbegin(), ShaderStack.crend(), id );
        cg_assertMsg( entry != ShaderStack.crend(), "Attempting to unbind untracked framebuffer!" );

        if ( entry == ShaderStack.crbegin() ) {
            ShaderStack.pop_back();
            const auto newShaderIdIt = ShaderStack.crbegin();
            const auto newShaderId = newShaderIdIt == ShaderStack.crend() ? 0 : *newShaderIdIt;
            glUseProgram( newShaderId );
        }
        else {
            ShaderStack.erase( std::next( entry ).base() );
        }
    }

    using ShaderBindToken = ScopedToken<GLuint, Shader::staticUnbind>;

    static ShaderBindToken staticBind( GLuint id ) {
        glUseProgram( id );
        ShaderStack.push_back( id );
        return ShaderBindToken( std::move( id ) );
    }

public:
    static constexpr GLuint InvalidShaderId = std::numeric_limits<GLint>::max();

    Shader();
    ~Shader();
    void cleanup();

    const GLuint getShaderID() const;

    //Compile a shader stage from a source text file. Returns Shader ID
    const GLuint compileShader();

    //Register a shader file to the pipeline
    bool registerShaderStageFromFile( const std::filesystem::path &_filePath, GLenum _stageType );

    //Register a shader source to the pipeline
    void registerShaderStage( const std::string &_shaderSource, GLenum _stageType );

    //Register a shader attribute, to be bound at _Location
    void registerAttribute( const std::string &_attributeName, GLuint _location );

    //Register a shader attribute, to be bound at _Location
    void registerTextureUnit( const std::string &_attributeName, GLuint _location );

    //Activate the program
    ShaderBindToken useShader() const;

    //Register a Uniform.
    //Returns pointer to UBO (Object finalised after call to compile)
    std::shared_ptr<CG_Data::Uniform> registerUniform( const std::string &_uniformName );

    std::shared_ptr<CG_Data::Uniform> registerUniform( const std::string &_uniformName, std::function<void(const CG_Data::Uniform&)> _callbackFunction );

    void registerUBO( const std::string &_uboName, std::shared_ptr<const CG_Data::UBO> _ubo );

    // TODO - some kind of type-id for the UBO
    void registerUboPlaceholder( const std::string &_uboName );

    std::shared_ptr<CG_Data::Uniform> getUniform( uint8_t index ) const;

    std::shared_ptr<CG_Data::Uniform> getUniform( const std::string &_uName );

    GLuint getUboBlockIndex( const std::string &uboName ) const {
        return glGetUniformBlockIndex( shaderID, uboName.c_str() );
        //return uboBlockIndices.at( uboName )->blockIndex;
    }

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
    struct UniformStruct {
        std::string name;
        std::shared_ptr< CG_Data::Uniform > uniformObject;
    };
    struct UboStruct {
        std::shared_ptr< const CG_Data::UBO > ubo;
        GLuint blockIndex;
    };
    std::vector<ShaderStage*> shaderStages;
    std::vector<Attribute*> attributes;
    std::vector<UniformStruct*> uniforms;
    std::map<std::string, std::shared_ptr<CG_Data::Uniform>> uniformMap;
    std::map<std::string, std::unique_ptr<UboStruct>> uboBlockIndices;
    std::map<std::string, GLuint> textureLocations;

    GLuint shaderID = InvalidShaderId;
    bool initialised{ false };
    const GLuint compileShaderStage( ShaderStage *stage );
};

// TODO - more generic
class TempSimpleShaderRenderNode : public IRenderable {
public:
    TempSimpleShaderRenderNode( std::shared_ptr<Shader> shader, std::shared_ptr<GL_Engine::ModelAttribute> model )
        : m_shader( std::move( shader ) )
        , m_model( std::move( model ) )
    {
        const auto &material = m_model->GetMaterial();
        cg_assert( material.HasComponent( MaterialComponent::Phong ) );
    }

    void execute() override {
        UsingScopedToken( m_shader->useShader() ) {

            auto matBufferBindPost = m_model->GetMaterialBuffer()->GetBindingPost();
            auto blockIndex = m_shader->getUboBlockIndex( "MaterialData" );

            auto modelMatUniform = m_shader->getUniform( "modelMatrix" );
            modelMatUniform->SetData( (void *)&m_model->GetMeshSceneTransformation() );
            modelMatUniform->Update();
            glUniformBlockBinding( m_shader->getShaderID(), blockIndex, matBufferBindPost );

            m_model->execute();
        }
    }

private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<GL_Engine::ModelAttribute> m_model;
};

// TODO - more generic

class TempTexturedShaderRenderNode : public IRenderable {
public:
    TempTexturedShaderRenderNode( std::shared_ptr<Shader> shader, std::shared_ptr<GL_Engine::ModelAttribute> model )
        : m_shader( std::move( shader ) )
        , m_model( std::move( model ) )
    {
        const Material &material = m_model->GetMaterial();
        cg_assert( material.HasComponent( MaterialComponent::Phong ) );
        cg_assert( material.HasComponent( MaterialComponent::DiffuseTexture ) );
    }

    void execute() override {
        UsingScopedToken( m_shader->useShader() ) {
            auto material = m_model->GetMaterial();
            auto tex = material.GetComponent<MaterialComponent::DiffuseTexture, CG_Data::Texture>();
            tex->Bind(); 
            auto matBufferBindPost = m_model->GetMaterialBuffer()->GetBindingPost();
            auto blockIndex = m_shader->getUboBlockIndex( "MaterialData" );

            auto modelMatUniform = m_shader->getUniform( "modelMatrix" );
            modelMatUniform->SetData( (void *)&m_model->GetMeshSceneTransformation() );
            modelMatUniform->Update();
            glUniformBlockBinding( m_shader->getShaderID(), blockIndex, matBufferBindPost );

            m_model->execute();
        }
    }

private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<GL_Engine::ModelAttribute> m_model;
};

template<typename T>
concept HasTransformationMatrix =
    requires ( T t ) {
        { t.GetTransformationMatrix() } -> std::same_as<glm::mat4 &>;
};

template<typename T>
//    requires HasTransformationMatrix<T>
class TempTexturedShaderRenderNode2 : public IRenderable {
public:
    TempTexturedShaderRenderNode2( std::shared_ptr<Shader> shader, std::shared_ptr<T> entity )
        : m_shader( std::move( shader ) )
        , m_entity( std::move( entity ) )
    {
        const Material &material = m_entity->GetMaterial();
        cg_assert( material.HasComponent( MaterialComponent::Phong ) );
        cg_assert( material.HasComponent( MaterialComponent::DiffuseTexture ) );
    }

    void execute() override {
        UsingScopedToken( m_shader->useShader() ) {
            auto material = m_entity->GetMaterial();
            auto tex = material.GetComponent<MaterialComponent::DiffuseTexture, CG_Data::Texture>();
            tex->Bind();
            auto matBufferBindPost = m_entity->GetMaterialBuffer()->GetBindingPost();
            auto blockIndex = m_shader->getUboBlockIndex( "MaterialData" );

            auto modelMatUniform = m_shader->getUniform( "modelMatrix" );
            modelMatUniform->SetData( (void *)&m_entity->GetTransformationMatrix() );
            modelMatUniform->Update();
            glUniformBlockBinding( m_shader->getShaderID(), blockIndex, matBufferBindPost );

            m_entity->execute();
        }
    }

private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<T> m_entity;
};

template<typename T>
//    requires HasTransformationMatrix<T>
class TempSimpleShaderRenderNode2 : public IRenderable {
public:
    TempSimpleShaderRenderNode2( std::shared_ptr<Shader> shader, std::shared_ptr<T> entity )
        : m_shader( std::move( shader ) )
        , m_entity( std::move( entity ) )
    {
        const auto &material = m_entity->GetMaterial();
        cg_assert( material.HasComponent( MaterialComponent::Phong ) );
    }

    void execute() override {
        UsingScopedToken( m_shader->useShader() ) {

            auto matBufferBindPost = m_entity->GetMaterialBuffer()->GetBindingPost();
            auto blockIndex = m_shader->getUboBlockIndex( "MaterialData" );

            auto modelMatUniform = m_shader->getUniform( "modelMatrix" );
            modelMatUniform->SetData( (void *)&m_entity->GetTransformationMatrix() );
            modelMatUniform->Update();
            glUniformBlockBinding( m_shader->getShaderID(), blockIndex, matBufferBindPost );

            m_entity->execute();
        }
    }

private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<T> m_entity;
};



} // namespace GL_Engine
#endif // SHADER_H