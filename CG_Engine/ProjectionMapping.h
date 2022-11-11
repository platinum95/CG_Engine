#ifndef PROJECTION_MAPPING_H
#define PROJECTION_MAPPING_H

#include "Camera.h"
#include "Renderer.h"

namespace GL_Engine {

/* Class for Shadow Mapping */
class ProjectionMapping {
    public:
    ProjectionMapping( uint16_t _fbWidth, uint16_t _fbHeight, glm::vec3 _dir, const std::shared_ptr< CG_Data::UBO > ubo );
    
    std::shared_ptr< RenderPass > addRenderPass( uint16_t _modelMatrixIdx=-1 );
    std::shared_ptr< RenderPass > addRenderPass( Shader * _shader );
    void addRenderPass( std::shared_ptr< RenderPass > _renderPass );
    
    enum ProjectionMapType {
        ShadowMap, CausticMap
    };

    std::shared_ptr< CG_Data::Texture > getTexture( ProjectionMapType _type );
    std::shared_ptr< Renderer > getRenderer();

    void render( Renderer * renderer );
    Camera mappingCamera;

    private:
    std::shared_ptr< Renderer > renderer;
    std::unordered_map< ProjectionMapType, 
                        std::shared_ptr< CG_Data::Texture > > textureMaps;

    Shader defaultShader;

    static void defaultRenderFunction( RenderPass& _pass, void* _data );
    

    glm::vec3 direction;
    uint16_t fbWidth, fbHeight;

    glm::mat4 projectionMatrix;

    std::unique_ptr< CG_Data::FBO > fbo;

};

/* Caustic Mapping Class */
class CausticMapping {
public:
    CausticMapping( uint16_t _fbWidth, uint16_t _fbHeight, glm::vec3 _dir,
                    float _distance, std::string splatterPath );

    // Functions for receiver-object render passes
    std::shared_ptr< RenderPass >
    addReceiverPass( uint16_t _modelMatrixIdx=-1 );
    std::shared_ptr< RenderPass > addReceiverPass( Shader * shader );
    void addReceiverPass( std::shared_ptr< RenderPass > _renderPass );

    // Functions for caustic-object render passes
    std::shared_ptr< RenderPass > addCausticPass( uint16_t _modelMatrixIdx=-1 );
    std::shared_ptr< RenderPass > addCausticPass( Shader * shader );
    void addCausticPass( std::shared_ptr< RenderPass > _renderPass );

    
    std::shared_ptr< Renderer > getRenderer();

    void render( std::shared_ptr< Camera > _sceneCam );
    Camera mappingCamera;
    enum TextureType {
        ReceiverWorldspaceTexture, CausticSplatterTexture, CausticDepthTexture
    };

    std::shared_ptr< CG_Data::Texture > getTexture( TextureType _type );

private:
    std::shared_ptr< Renderer > receiverRenderer, causticRenderer;
    std::unordered_map< TextureType, 
                        std::shared_ptr< CG_Data::Texture > > textureMaps;

    Shader defaultShader, defaultCausticShader;

    static void defaultRenderFunction( RenderPass& _pass, void* _data );
    static void defaultCausticRenderFunction( RenderPass& _pass, void* _data );

    std::shared_ptr< CG_Data::Texture > splatterTex;

    void updateProjectionCamera( std::shared_ptr< Camera > _sceneCamera );
    

    glm::vec3 direction;
    uint16_t fbWidth, fbHeight;

    glm::mat4 projectionMatrix;

    // These may be merged later
    std::unique_ptr< CG_Data::FBO > receiverFbo, causticFbo;

    uint32_t surfaceArea;
    float distance;

};

}

#endif // PROJECTION_MAPPING_H