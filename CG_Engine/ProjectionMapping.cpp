
#include <ProjectionMapping.h>
#include <ModelLoader.h>

namespace GL_Engine{

    std::string defaultVertexShaderStr = "\
    #version 330\n \
    layout (std140) uniform CameraProjectionData{ \n \
        mat4 viewMatrix;\n \
        mat4 projectionMatrix;\n \
        mat4 pvMatrix;\n \
        vec4 cameraPosition;\n \
        vec4 cameraOrientation; \n \
        vec4 clippingPlane; \n \
    };\n \
    in vec3 vPosition;\n \
    uniform mat4 modelMatrix;\n \
    void main(){\n \
        gl_Position = pvMatrix * modelMatrix * vec4( vPosition, 1.0 );\n \
    }\n \
    ";
    std::string defaultFragmentShaderStr = "\
    #version 330\n \
    vec4 fOut;\n \
    void main(){\n \
        fOut = vec4( 1.2, 10.3, 10.4, 1.0 );\n \
    }\n \
    ";

    ProjectionMapping::ProjectionMapping( uint16_t _fbWidth, uint16_t _fbHeight,
                                          glm::vec3 _dir, const std::shared_ptr< CG_Data::UBO > ubo ){

        this->direction = _dir;
        this->fbWidth = _fbWidth;
        this->fbHeight = _fbHeight;

        this->projectionMatrix = glm::ortho( -50.0f, 50.0f, -50.0f, 50.0f, 
                                             1.0f, 700.5f );
        this->mappingCamera.setProjectionMatrix( this->projectionMatrix );
        this->mappingCamera.setCameraPosition( glm::vec3( 0, 2.0*12.2, 2.0*12.2 ) );
        this->mappingCamera.yawBy( 180.0f );
        this->mappingCamera.pitchBy( -45.0f );
        this->mappingCamera.initialise();

        this->fbo = std::make_unique< CG_Data::FBO >( fbWidth, fbHeight );


        auto depthBuffer = this->fbo->addAttachment(
                                CG_Data::FBO::AttachmentType::DepthTexture,
                                this->fbWidth, this->fbHeight );
        
        auto colourBuffer = this->fbo->addAttachment(
                                CG_Data::FBO::AttachmentType::ColourTexture,
                                this->fbWidth, this->fbHeight );
        
        // GLuint shadowTextureId;
        // glGenTextures( 1, &shadowTextureId );
        // glBindTexture( GL_TEXTURE_2D, shadowTextureId );
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, this->fbWidth, 
        //               this->fbHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
        //               nullptr );
        // auto shadowTexture = std::make_shared< CG_Data::Texture >( 
        //     shadowTextureId, GL_TEXTURE0, GL_TEXTURE_2D );
        // glBindFramebuffer( GL_FRAMEBUFFER, this->fbo->getID() );
        // //this->fbo->bind(0);
        // glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTexture->GetID(), 0 );
        // glDrawBuffer( GL_NONE );
        // glReadBuffer( GL_NONE );
        // if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE ){
        //     std::cerr << "FB not complete" << std::endl;
        // }
        // glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        // auto colourBuffer = this->fbo->addAttachment( 
        //                         CG_Data::FBO::AttachmentType::TextureAttachment,
        //                         this->fbWidth, this->fbHeight );

        auto colourTex = 
            std::static_pointer_cast< CG_Data::FBO::TexturebufferObject >(
                colourBuffer )->GetTexture();
        colourTex->SetUnit( GL_TEXTURE0 );

        auto shadowTexture = 
            std::static_pointer_cast< CG_Data::FBO::TexturebufferObject >(
                depthBuffer )->GetTexture();

        this->textureMaps[ ShadowMap ] = std::move( shadowTexture );
       // this->textureMaps[ ShadowMap ] = std::move( colourTex );

        this->renderer = std::make_shared< Renderer >();


        auto camUbo = mappingCamera.getCameraUbo();
        renderer->AddUBO( camUbo.get() );

        auto matrixUniformUpdateLambda = []( const CG_Data::Uniform & u ){
            glUniformMatrix4fv( u.GetID(), 1, GL_FALSE, 
                                static_cast< const GLfloat * >( u.GetData() ) );
        };


        defaultShader.registerShaderStage( defaultVertexShaderStr,
                                           GL_VERTEX_SHADER );
        defaultShader.registerShaderStage( defaultFragmentShaderStr,
                                           GL_FRAGMENT_SHADER );
        defaultShader.registerAttribute( "vPosition", 0 );
        defaultShader.registerUBO( "CameraProjectionData", camUbo );
        defaultShader.registerUniform( "modelMatrix" );
        defaultShader.compileShader();
        defaultShader.getUniform( "modelMatrix" )->SetUpdateCallback( 
                        matrixUniformUpdateLambda );
    }
    
    std::shared_ptr< RenderPass > 
    ProjectionMapping::addRenderPass( uint16_t _modelMatrixIdx ){
        auto rPass = this->renderer->AddRenderPass( &this->defaultShader );
        if( _modelMatrixIdx >= 0 ){
            rPass->AddDataLink( defaultShader.getUniform( "modelMatrix" ).get(), 
                                _modelMatrixIdx );
        }
        rPass->renderFunction = defaultRenderFunction;
        return std::move( rPass );

    }

    std::shared_ptr< RenderPass > 
    ProjectionMapping::addRenderPass( Shader * _shader ){
        auto rp = this->renderer->AddRenderPass( _shader );
        rp->renderFunction = defaultRenderFunction;
        return std::move( rp );
    }

    void ProjectionMapping::addRenderPass( std::shared_ptr< RenderPass > 
                                           _renderPass ){
        this->renderer->AddRenderPass( _renderPass );
    }
    

    std::shared_ptr< CG_Data::Texture > 
    ProjectionMapping::getTexture( ProjectionMapType _type ){
        // Should throw an exception if the map doesn't exist
        return this->textureMaps.at( _type );
    }

    std::shared_ptr< Renderer > ProjectionMapping::getRenderer(){
        return this->renderer;
    }

    void ProjectionMapping::render( Renderer * _renderer ){
        this->fbo->bind( 0 );
        //glViewport( 0, 0, this->fbWidth, this->fbHeight );
        //glBindFramebuffer( GL_FRAMEBUFFER, this->fbo->getID() );

        glClearColor( 0.1f, 0.2f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        //_renderer->Render();
        glCullFace( GL_FRONT );
        this->renderer->Render();
        glCullFace( GL_BACK );
        this->fbo->unbind();
    }


    void 
    ProjectionMapping::defaultRenderFunction( RenderPass& _pass, void* _data ){
        _pass.shader->useShader();
        _pass.BatchVao->BindVAO();
        for (auto tex : _pass.Textures) {
            tex->Bind();
        }
        for (auto&& batch : _pass.batchUnits) {
            if (batch->active && batch->entity->isActive()) {
                for (auto l : _pass.dataLink) {
                    l.uniform->SetData(batch->entity->GetData(l.eDataIndex));
                    l.uniform->Update();
                }
                batch->entity->UpdateUniforms();
                batch->entity->update();
                _pass.DrawFunction();
            }
        }
	
    }

    









    /* --- Caustic Mapping class --- */
    std::string defaultReceiverVertexShaderStr = "\
    #version 330\n \
    layout (std140) uniform CameraProjectionData{ \n \
        mat4 viewMatrix;\n \
        mat4 projectionMatrix;\n \
        mat4 pvMatrix;\n \
        vec4 cameraPosition;\n \
        vec4 cameraOrientation; \n \
        vec4 clippingPlane; \n \
    };\n \
    in vec3 vPosition;\n \
    out vec4 vWorldPos; \n \
    uniform mat4 modelMatrix;\n \
    void main(){\n \
        vWorldPos = modelMatrix * vec4( vPosition, 1.0 );\n \ 
        gl_Position = pvMatrix * modelMatrix * vec4( vPosition, 1.0 );\n \
    }\n \
    ";
    std::string defaultReceiverFragmentShaderStr = "\
    #version 330\n \
    in vec4 vWorldPos; \n \
    out vec4 fOut;\n \
    void main(){\n \
        fOut = vWorldPos;\n \
    }\n \
    ";

    std::string defaultCausticVertexShaderStr = std::string(
        #include "./res/DefaultCausticV.glsl"
    );
    
    std::string defaultCausticFragmentShaderStr = std::string(
        #include "./res/DefaultCausticF.glsl"
    );

    CausticMapping::CausticMapping( uint16_t _fbWidth, uint16_t _fbHeight, 
                                    glm::vec3 _dir, std::string _splatterPath ){

        this->direction = _dir;
        this->fbWidth = _fbWidth;
        this->fbHeight = _fbHeight;

        this->splatterTex = 
            ModelLoader::loadTexture( _splatterPath, GL_TEXTURE1 );

        this->surfaceArea = 1;

        this->projectionMatrix = glm::ortho( -50.0f, 50.0f, -50.0f, 50.0f, 
                                             1.0f, 700.5f );
        this->mappingCamera.setProjectionMatrix( this->projectionMatrix );
        this->mappingCamera.setCameraPosition( glm::vec3( 0, 2.0*12.2, 2.0*12.2 ) );
        this->mappingCamera.yawBy( 180.0f );
        this->mappingCamera.pitchBy( -45.0f );
        this->mappingCamera.initialise();

        this->receiverFbo = std::make_unique< CG_Data::FBO >( fbWidth, 
                                                              fbHeight );


        auto depthBuffer = this->receiverFbo->addAttachment(
                                CG_Data::FBO::AttachmentType::DepthTexture,
                                this->fbWidth, this->fbHeight );
        
        auto colourBuffer = this->receiverFbo->addAttachment(
                        CG_Data::FBO::AttachmentType::ColourTexture,
                        this->fbWidth, this->fbHeight );
        
        auto colourTex = 
            std::static_pointer_cast< CG_Data::FBO::TexturebufferObject >(
                colourBuffer )->GetTexture();
        colourTex->SetUnit( GL_TEXTURE0 );

        auto depthTexture = 
            std::static_pointer_cast< CG_Data::FBO::TexturebufferObject >(
                depthBuffer )->GetTexture();
        depthTexture->SetUnit( GL_TEXTURE0 );


        this->textureMaps[ ReceiverWorldspaceTexture ] =
            std::move( depthTexture );



        this->causticFbo = std::make_unique< CG_Data::FBO >( fbWidth, 
                                                             fbHeight );


        auto depthCausticBuffer = this->causticFbo->addAttachment(
                                CG_Data::FBO::AttachmentType::DepthTexture,
                                this->fbWidth, this->fbHeight );
        
        auto colourCausticBuffer = this->causticFbo->addAttachment(
                                CG_Data::FBO::AttachmentType::ColourTexture,
                                this->fbWidth, this->fbHeight );
        
        auto colourCausticTex = 
            std::static_pointer_cast< CG_Data::FBO::TexturebufferObject >(
                colourCausticBuffer )->GetTexture();
        colourCausticTex->SetUnit( GL_TEXTURE4 );

        auto shadowCausticTexture = 
            std::static_pointer_cast< CG_Data::FBO::TexturebufferObject >(
                depthCausticBuffer )->GetTexture();


        this->textureMaps[ CausticSplatterTexture ] =
            std::move( colourCausticTex );

        this->receiverRenderer = std::make_shared< Renderer >();
        this->causticRenderer = std::make_shared< Renderer >();


        auto camUbo = mappingCamera.getCameraUbo();
        receiverRenderer->AddUBO( camUbo.get() );
        causticRenderer->AddUBO( camUbo.get() );

        auto matrixUniformUpdateLambda = []( const CG_Data::Uniform & u ){
            glUniformMatrix4fv( u.GetID(), 1, GL_FALSE, 
                                static_cast< const GLfloat * >( u.GetData() ) );
        };

        auto uint32UniformUpdateLambda = []( const CG_Data::Uniform & u ){
            GLuint val = *static_cast< const GLuint * >( u.GetData() );
            glUniform1ui( u.GetID(), val );
        };


        defaultShader.registerShaderStage( defaultReceiverVertexShaderStr,
                                           GL_VERTEX_SHADER );
        defaultShader.registerShaderStage( defaultReceiverFragmentShaderStr,
                                           GL_FRAGMENT_SHADER );
        defaultShader.registerAttribute( "vPosition", 0 );
        defaultShader.registerUBO( "CameraProjectionData", camUbo );
        defaultShader.registerUniform( "modelMatrix" );
        defaultShader.compileShader();
        defaultShader.getUniform( "modelMatrix" )->SetUpdateCallback( 
                        matrixUniformUpdateLambda );

        defaultCausticShader.registerShaderStage( defaultCausticVertexShaderStr,
                                           GL_VERTEX_SHADER );
        defaultCausticShader.registerShaderStage( defaultCausticFragmentShaderStr,
                                           GL_FRAGMENT_SHADER );
        defaultCausticShader.registerAttribute( "vPosition", 0 );
        defaultCausticShader.registerAttribute( "vNormal", 2 );
        defaultCausticShader.registerUBO( "CameraProjectionData", camUbo );
        defaultCausticShader.registerUniform( "modelMatrix" );
        defaultCausticShader.registerUniform( "surfaceArea" );
        defaultCausticShader.registerTextureUnit( "receiverTex", 0 );
        defaultCausticShader.registerTextureUnit( "splatterTex", 1 );
        defaultCausticShader.compileShader();
        defaultCausticShader.getUniform( "modelMatrix" )->SetUpdateCallback( 
            matrixUniformUpdateLambda );
        defaultCausticShader.getUniform( "surfaceArea" )->SetUpdateCallback(
            uint32UniformUpdateLambda
        );
    }
    
    std::shared_ptr< RenderPass > 
    CausticMapping::addReceiverPass( uint16_t _modelMatrixIdx ){
        auto rPass = 
            this->receiverRenderer->AddRenderPass( &this->defaultShader );
        if( _modelMatrixIdx >= 0 ){
            rPass->AddDataLink( defaultShader.getUniform( "modelMatrix" ).get(), 
                                _modelMatrixIdx );
        }
        
        rPass->renderFunction = defaultRenderFunction;
        return std::move( rPass );

    }

    std::shared_ptr< RenderPass > 
    CausticMapping::addReceiverPass( Shader * _shader ){
        auto rp = this->receiverRenderer->AddRenderPass( _shader );
        rp->renderFunction = defaultRenderFunction;
        return std::move( rp );
    }

    void CausticMapping::addReceiverPass( std::shared_ptr< RenderPass > 
                                           _renderPass ){
        this->receiverRenderer->AddRenderPass( _renderPass );
    }

    std::shared_ptr< RenderPass > 
    CausticMapping::addCausticPass( uint16_t _modelMatrixIdx ){
        auto rPass = 
            this->causticRenderer->AddRenderPass( &this->defaultCausticShader );
        if( _modelMatrixIdx >= 0 ){
            rPass->AddDataLink( defaultCausticShader.getUniform( "modelMatrix" ).get(), 
                                _modelMatrixIdx );
        }
        rPass->AddUniform( defaultCausticShader.getUniform( "surfaceArea" ).get(),
                           ( void * ) &this->surfaceArea );
        rPass->renderFunction = defaultCausticRenderFunction;
        rPass->Textures.push_back( this->textureMaps.at( ReceiverWorldspaceTexture ) );
        rPass->Textures.push_back( this->splatterTex );
        return std::move( rPass );

    }

    std::shared_ptr< RenderPass > 
    CausticMapping::addCausticPass( Shader * _shader ){
        auto rp = this->causticRenderer->AddRenderPass( _shader );
        rp->renderFunction = defaultCausticRenderFunction;
        rp->Textures.push_back( this->textureMaps.at( ReceiverWorldspaceTexture ) );
        rp->Textures.push_back( this->splatterTex );
        return std::move( rp );
    }

    void CausticMapping::addCausticPass( std::shared_ptr< RenderPass > 
                                           _renderPass ){
        this->causticRenderer->AddRenderPass( _renderPass );
    }

    std::shared_ptr< CG_Data::Texture > 
    CausticMapping::getTexture( TextureType _type ){
        // Should throw an exception if the map doesn't exist
        return this->textureMaps.at( _type );
    }

    std::shared_ptr< Renderer > CausticMapping::getRenderer(){
        return this->receiverRenderer;
    }

    void CausticMapping::render( Renderer * _renderer ){
        this->receiverFbo->bind( 0 );

        glClearColor( 0.1f, 0.9f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        //_renderer->Render()vPosLightspace;
        glCullFace( GL_FRONT );
        this->receiverRenderer->Render();

        GLuint qId;
        glGenQueries( 1, &qId );
        glBeginQuery( GL_SAMPLES_PASSED, qId );
        this->causticFbo->bind( 0 );
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
        //_renderer->Render();
        glDepthMask( GL_FALSE );
        glCullFace( GL_BACK );
        glEnable(GL_BLEND);
        glBlendFunc( GL_ONE, GL_ONE );  
        this->causticRenderer->Render();
        glDisable( GL_BLEND );
        glEndQuery( GL_SAMPLES_PASSED );
        GLint samplesPassed;
        glGetQueryObjectiv( qId, GL_QUERY_RESULT, &samplesPassed );
        this->surfaceArea = ( uint32_t ) samplesPassed;
        glDeleteQueries( 1, &qId );

        glCullFace( GL_BACK );
        glDepthMask( GL_TRUE );
        this->causticFbo->unbind();
    }


    void 
    CausticMapping::defaultRenderFunction( RenderPass& _pass, void* _data ){
        _pass.shader->useShader();
        _pass.BatchVao->BindVAO();
        for (auto tex : _pass.Textures) {
            tex->Bind();
        }
        for (auto&& batch : _pass.batchUnits) {
            if (batch->active && batch->entity->isActive()) {
                for (auto l : _pass.dataLink) {
                    l.uniform->SetData(batch->entity->GetData(l.eDataIndex));
                    l.uniform->Update();
                }
                batch->entity->UpdateUniforms();
                batch->entity->update();
                _pass.DrawFunction();
            }
        }
	
    }

    void 
    CausticMapping::defaultCausticRenderFunction( RenderPass& _pass,
                                                  void* _data ){
        _pass.shader->useShader();
        for( auto uniDataPair : _pass.uniforms ){
            uniDataPair.first->SetData( uniDataPair.second );
            uniDataPair.first->Update();
        }
        _pass.BatchVao->BindVAO();
        for (auto tex : _pass.Textures) {
            tex->Bind();
        }
        for (auto&& batch : _pass.batchUnits) {
            if (batch->active && batch->entity->isActive()) {
                for (auto l : _pass.dataLink) {
                    l.uniform->SetData(batch->entity->GetData(l.eDataIndex));
                    l.uniform->Update();
                }
                batch->entity->UpdateUniforms();
                batch->entity->update();
                _pass.DrawFunction();
            }
        }
	
    }



}