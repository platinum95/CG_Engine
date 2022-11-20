#include "Water.h"
#include "ModelLoader.h"

namespace GL_Engine{

std::vector< Water * > Water::waterObjects;

Water::Water( uint16_t _fbWidth, uint16_t _fbHeight, 
              std::filesystem::path _dudvPath,
              std::vector< std::shared_ptr< Renderer > > _renderers,
              std::shared_ptr< Camera > _sceneCamera,
              std::shared_ptr< CG_Data::VAO > _waterData ){

    this->sceneCamera = _sceneCamera;
    this->renderers = _renderers;
    this->waterVao = _waterData;
    auto cameraUbo = sceneCamera->getCameraUbo();

    /* Create Shader */
    std::string defaultWaterVertexShaderStr = std::string(
        #include "./res/WaterShaderV.glsl"
    );
    std::string defaultWaterFragmentShaderStr = std::string(
        #include "./res/WaterShaderF.glsl"
    );
    waterShader = std::make_unique< Shader >();
    waterShader->registerShaderStage(
        std::move( defaultWaterVertexShaderStr ), GL_VERTEX_SHADER );
	waterShader->registerShaderStage(
        std::move( defaultWaterFragmentShaderStr ), GL_FRAGMENT_SHADER );
	waterShader->registerAttribute( "vPosition", 0 );
    waterShader->registerAttribute( "tCoord", 1 );
	waterShader->registerTextureUnit( "reflectionTexture", 0 );
	waterShader->registerTextureUnit( "refractionTexture", 1 );
	waterShader->registerTextureUnit( "dudvMap", 2 );
	waterShader->registerUBO( std::string( "cameraProjectionData" ),
                              cameraUbo );
	waterShader->registerUniform( "time" );
    waterShader->registerUniform( "modelMatrix" );
	waterShader->compileShader();

    auto matrixUniformUpdateLambda = []( const CG_Data::Uniform & u ){
        glUniformMatrix4fv( u.GetID(), 1, GL_FALSE, 
                            static_cast< const GLfloat * >( u.GetData() ) );
    };
    auto floatUniformUpdateLambda = []( const CG_Data::Uniform & u ){
        glUniform1f( u.GetID(),
                     *static_cast< const GLfloat * >( u.GetData() ) );
    };

    auto waterModelUniform = waterShader->getUniform( "modelMatrix" );
    waterModelUniform->SetUpdateCallback( matrixUniformUpdateLambda );

    auto waterTimeUniform = waterShader->getUniform( "time" );
    waterTimeUniform->SetUpdateCallback( floatUniformUpdateLambda );



    /* Create FBO */
    this->waterFbo = std::make_unique< CG_Data::FBO >( 
        _fbWidth, _fbHeight );
	auto reflectColAttach = waterFbo->addAttachment( 
        CG_Data::FBO::AttachmentType::ColourTexture, _fbWidth, _fbHeight );
        
    auto refractColAttach = waterFbo->addAttachment( 
        CG_Data::FBO::AttachmentType::ColourTexture, _fbWidth, _fbHeight );
    
	waterFbo->addAttachment( 
        CG_Data::FBO::AttachmentType::DepthRenderbuffer, _fbWidth, _fbHeight );

	this->reflTex = std::static_pointer_cast< 
        CG_Data::FBO::TexturebufferObject >( reflectColAttach )->GetTexture();
	this->refrTex = std::static_pointer_cast< 
        CG_Data::FBO::TexturebufferObject >( refractColAttach )->GetTexture();

	reflTex->SetUnit( GL_TEXTURE0 );
	refrTex->SetUnit( GL_TEXTURE1 );

    /* Create DUDV Texture */
    this->dudvTexture = ModelLoader::loadTexture( _dudvPath, GL_TEXTURE2 );

    /* Set up render pass */
    this->waterRenderPass = std::make_shared< RenderPass >();
	waterRenderPass->renderFunction = defaultWaterRenderer;
	waterRenderPass->Data = this;
	waterRenderPass->shader = this->waterShader.get();
    auto numVertices = waterVao->getIndexCount();

	waterRenderPass->SetDrawFunction( [ numVertices ]() { 
        glDrawElements( GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, nullptr ); 
    } );
	waterRenderPass->Textures.push_back( reflTex );
	waterRenderPass->Textures.push_back( refrTex );
	waterRenderPass->Textures.push_back( dudvTexture );
	waterRenderPass->BatchVao = this->waterVao;
	//waterRenderPass->AddDataLink( WaterTimeUniform, waterTimeIndex );
	waterRenderPass->AddBatchUnit( this );

    auto waterModelMatrixIdx = this->AddData( 
    ( void * ) glm::value_ptr( this->TransformMatrix ) );
    waterRenderPass->AddDataLink( waterModelUniform.get(),
                                  waterModelMatrixIdx );

    auto waterTimeMatrixIdx = this->AddData( ( void * ) &this->time );
    waterRenderPass->AddDataLink( waterTimeUniform.get(),
                                  waterTimeMatrixIdx );

    waterObjects.push_back( this );

    waterStopwatch.Initialise();
    time = 0.0f;
}

std::shared_ptr< RenderPass > Water::getRenderPass(){
    return this->waterRenderPass;
}

void Water::cleanup(){
    waterShader->cleanup();
    waterFbo->cleanup();
    dudvTexture->Cleanup();
    waterRenderPass->Cleanup();
    sceneCamera.reset();
    waterVao->Cleanup();
    renderers.clear();
}

Water::~Water(){
    waterObjects.erase( std::remove( waterObjects.begin(),
                                     waterObjects.end(), this ),
                        waterObjects.end() );
}



void Water::defaultWaterRenderer( RenderPass & _rPass, void * _data ){
    auto that = ( Water * ) _data;
    if( !that->isActive() )
        return;
    auto camera = that->sceneCamera;
    auto camUbo = const_cast< CameraUboData * >( camera->getCameraUboData() );
    std::vector< bool > waterState;
    waterState.reserve( waterObjects.size() );
    for( auto waterObj : waterObjects ){
        waterState.push_back( waterObj->isActive() );
        waterObj->Deactivate();
    }

    auto frameTimeMicros = that->waterStopwatch.MeasureTime().count();
    that->time += static_cast<float>( frameTimeMicros ) / 1.0e6f;


    that->Deactivate();
    
    glEnable ( GL_CLIP_DISTANCE0 );
    camUbo->clippingPlane[ 0 ] = 0;
    camUbo->clippingPlane[ 1 ] = camera->getCameraPosition().y < 0 ? -1.0f : 1.0f;
    camUbo->clippingPlane[ 2 ] = 0;
    camUbo->clippingPlane[ 3 ] = 0;
    
    // Reflect pass
    camera->reflectCamera();
    camera->update();
    
    auto bindToken = that->waterFbo->bind( 0 );
    for( auto & renderer : that->renderers ){
        renderer->Render();
    }
    camera->reflectCamera();

    // Refract pass
    camUbo->clippingPlane[ 1 ] *= -1;
    camera->update();
    bindToken = that->waterFbo->bind( 1 );
    for( auto &renderer : that->renderers ){
        renderer->Render();
    }

    for( size_t i = 0; i < waterObjects.size(); i++ ){
        if( waterState[ i ] )
            waterObjects[ i ]->Activate();
    }

    std::move( bindToken ).release();
    camUbo->clippingPlane[ 1 ] = 1000;
    glDisable( GL_CLIP_DISTANCE0 );
    that->Activate();

    that->waterShader->useShader();
	that->waterVao->BindVAO();
    that->reflTex->Bind();
    that->refrTex->Bind();
    that->dudvTexture->Bind();
    for( auto dLink : _rPass.dataLink ){
        dLink.uniform->SetData( that->GetData( dLink.eDataIndex ) );
        dLink.uniform->Update();
    }
    that->UpdateUniforms();
    that->update();
    _rPass.DrawFunction();
    
}


}