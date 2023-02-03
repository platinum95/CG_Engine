#include "ProjectionMapping.h"

#include "ModelLoader.h"
#include "Renderer.h"

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


        defaultShader.registerShaderStage( std::string( defaultVertexShaderStr ),
                                           GL_VERTEX_SHADER );
        defaultShader.registerShaderStage( std::string( defaultFragmentShaderStr ),
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
        return rPass;

    }

    std::shared_ptr< RenderPass > 
    ProjectionMapping::addRenderPass( Shader * _shader ){
        auto rp = this->renderer->AddRenderPass( _shader );
        rp->renderFunction = defaultRenderFunction;
        return rp;
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
        auto bindToken = this->fbo->bind( 0 );
        //glViewport( 0, 0, this->fbWidth, this->fbHeight );
        //glBindFramebuffer( GL_FRAMEBUFFER, this->fbo->getID() );

        glClearColor( 0.1f, 0.2f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        //_renderer->Render();
        glCullFace( GL_FRONT );
        this->renderer->Render();
        glCullFace( GL_BACK );
    }


    void 
    ProjectionMapping::defaultRenderFunction( RenderPass& _pass, void* _data ){
        UsingScopedToken( _pass.shader->useShader() ) {
            _pass.BatchVao->BindVAO();
            for ( auto tex : _pass.Textures ) {
                tex->Bind();
            }
            for ( auto &&batch : _pass.batchUnits ) {
                if ( batch->active && batch->entity->isActive() ) {
                    for ( auto l : _pass.dataLink ) {
                        l.uniform->SetData( batch->entity->GetData( l.eDataIndex ) );
                        l.uniform->Update();
                    }
                    batch->entity->UpdateUniforms();
                    batch->entity->update();
                    _pass.DrawFunction();
                }
            }
        }
    }

    /* --- Caustic Mapping class --- */

    // Default receiver shaders, just get depth value
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

    // Inline-include for default caustic shaders
    std::string defaultCausticVertexShaderStr = std::string(
        #include "./res/DefaultCausticV.glsl"
    );
    
    std::string defaultCausticFragmentShaderStr = std::string(
        #include "./res/DefaultCausticF.glsl"
    );

    // Create the caustic mapper
    CausticMapping::CausticMapping( uint16_t _fbWidth, uint16_t _fbHeight, 
                                    glm::vec3 _dir, float _distance,
                                    std::filesystem::path _splatterPath ){

        this->direction = _dir;
        this->fbWidth = _fbWidth;
        this->fbHeight = _fbHeight;
        this->distance = _distance;

        // Load the point-texture (usually Gaussian falloff)
        this->splatterTex = 
            ModelLoader::loadTexture( _splatterPath, GL_TEXTURE1 );

        this->surfaceArea = 1;
        // Default projection matrix
        this->projectionMatrix = glm::ortho( -50.0f, 50.0f, -50.0f, 50.0f, 
                                             1.0f, 700.5f );
        this->mappingCamera.setProjectionMatrix( this->projectionMatrix );
        this->mappingCamera.setCameraPosition( glm::vec3( 0, 2.0*12.2, 2.0*12.2 ) );
        // Just hardcode the direction for now.
        this->mappingCamera.yawBy( 180.0f );
        this->mappingCamera.pitchBy( -45.0f );
        this->mappingCamera.initialise();

        // Create the receiver FBO
        this->receiverFbo = std::make_unique< CG_Data::FBO >( fbWidth, 
                                                              fbHeight );

        // Get the texturebuffers
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


        // Create the caustic FBO
        this->causticFbo = std::make_unique< CG_Data::FBO >( fbWidth, 
                                                             fbHeight );

        // Get the texturebuffers
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

        auto causticDepthTexture = 
            std::static_pointer_cast< CG_Data::FBO::TexturebufferObject >(
                depthCausticBuffer )->GetTexture();


        this->textureMaps[ CausticSplatterTexture ] =
            std::move( colourCausticTex );

        this->textureMaps[ CausticDepthTexture ] =
            std::move( causticDepthTexture );

        // Create the two renderers
        this->receiverRenderer = std::make_shared< Renderer >();
        this->causticRenderer = std::make_shared< Renderer >();

        // Set up the shaders and such
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

        // Default shaders
        defaultShader.registerShaderStage( std::string( defaultReceiverVertexShaderStr ),
                                           GL_VERTEX_SHADER );
        defaultShader.registerShaderStage( std::string( defaultReceiverFragmentShaderStr ),
                                           GL_FRAGMENT_SHADER );
        defaultShader.registerAttribute( "vPosition", 0 );
        defaultShader.registerUBO( "CameraProjectionData", camUbo );
        defaultShader.registerUniform( "modelMatrix" );
        defaultShader.compileShader();
        defaultShader.getUniform( "modelMatrix" )->SetUpdateCallback( 
                        matrixUniformUpdateLambda );

        defaultCausticShader.registerShaderStage( std::string( defaultCausticVertexShaderStr ),
                                           GL_VERTEX_SHADER );
        defaultCausticShader.registerShaderStage( std::string( defaultCausticFragmentShaderStr ),
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
    
    // Add a receiver render pass, using the default shader
    std::shared_ptr< RenderPass > 
    CausticMapping::addReceiverPass( uint16_t _modelMatrixIdx ){
        auto rPass = 
            this->receiverRenderer->AddRenderPass( &this->defaultShader );
        if( _modelMatrixIdx >= 0 ){
            rPass->AddDataLink( defaultShader.getUniform( "modelMatrix" ).get(), 
                                _modelMatrixIdx );
        }
        
        rPass->renderFunction = defaultRenderFunction;
        return rPass;

    }

    // Add a receiver render pass, overriding the shader
    std::shared_ptr< RenderPass > 
    CausticMapping::addReceiverPass( Shader * _shader ){
        auto rp = this->receiverRenderer->AddRenderPass( _shader );
        rp->renderFunction = defaultRenderFunction;
        return rp;
    }

    void CausticMapping::addReceiverPass( std::shared_ptr< RenderPass > 
                                           _renderPass ){
        this->receiverRenderer->AddRenderPass( _renderPass );
    }

    // Add a caustic render pass, using the default shader
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
        return rPass;

    }

    // Add a caustic render pass, overriding the shader
    std::shared_ptr< RenderPass > 
    CausticMapping::addCausticPass( Shader * _shader ){
        auto rp = this->causticRenderer->AddRenderPass( _shader );
        rp->renderFunction = defaultCausticRenderFunction;
        rp->Textures.push_back( this->textureMaps.at( ReceiverWorldspaceTexture ) );
        rp->Textures.push_back( this->splatterTex );
        return rp;
    }

    // Add an existing RP object to the caustic renderer
    void CausticMapping::addCausticPass( std::shared_ptr< RenderPass > 
                                           _renderPass ){
        this->causticRenderer->AddRenderPass( _renderPass );
    }

    // Get the specified texture (from FBO texturebuffer)
    std::shared_ptr< CG_Data::Texture > 
    CausticMapping::getTexture( TextureType _type ){
        // Should throw an exception if the map doesn't exist
        return this->textureMaps.at( _type );
    }

    std::shared_ptr< Renderer > CausticMapping::getRenderer(){
        return this->receiverRenderer;
    }

    // Main Caustic mapping render function
    void CausticMapping::render( std::shared_ptr< Camera > _sceneCam ){
        updateProjectionCamera( _sceneCam );

        // Render receiver pass
        auto bindToken = this->receiverFbo->bind( 0 );
  
        glClearColor( 0.1f, 0.9f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glCullFace( GL_BACK );
        this->receiverRenderer->Render();

        GLuint qId;
        glGenQueries( 1, &qId );
        glBeginQuery( GL_SAMPLES_PASSED, qId );

        // Render caustic pass
        bindToken = this->causticFbo->bind( 0 );
        glCullFace( GL_BACK );
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
        // Disable depth discarding so the points can overlap while still
        // keeping depth info
        glDepthFunc( GL_ALWAYS );

        glEnable(GL_BLEND);
        glBlendFunc( GL_ONE, GL_ONE );  
        glEnable( GL_PROGRAM_POINT_SIZE );
        this->causticRenderer->Render();

        glDisable( GL_BLEND );
        glEndQuery( GL_SAMPLES_PASSED );

        GLint samplesPassed;
        glGetQueryObjectiv( qId, GL_QUERY_RESULT, &samplesPassed );
        this->surfaceArea = ( uint32_t ) samplesPassed;
        glDeleteQueries( 1, &qId );

        glCullFace( GL_BACK );
        glDepthFunc( GL_LESS );
    }


    void 
    CausticMapping::defaultRenderFunction( RenderPass& _pass, void* _data ){
        UsingScopedToken( _pass.shader->useShader() ) {
            _pass.BatchVao->BindVAO();
            for ( auto tex : _pass.Textures ) {
                tex->Bind();
            }
            for ( auto &&batch : _pass.batchUnits ) {
                if ( batch->active && batch->entity->isActive() ) {
                    for ( auto l : _pass.dataLink ) {
                        l.uniform->SetData( batch->entity->GetData( l.eDataIndex ) );
                        l.uniform->Update();
                    }
                    batch->entity->UpdateUniforms();
                    batch->entity->update();
                    _pass.DrawFunction();
                }
            }
        }
    }

    void 
    CausticMapping::defaultCausticRenderFunction( RenderPass& _pass,
                                                  void* _data ){
        UsingScopedToken( _pass.shader->useShader() ) {
            for ( auto uniDataPair : _pass.uniforms ) {
                uniDataPair.first->SetData( uniDataPair.second );
                uniDataPair.first->Update();
            }
            _pass.BatchVao->BindVAO();
            for ( auto tex : _pass.Textures ) {
                tex->Bind();
            }
            for ( auto &&batch : _pass.batchUnits ) {
                if ( batch->active && batch->entity->isActive() ) {
                    for ( auto l : _pass.dataLink ) {
                        l.uniform->SetData( batch->entity->GetData( l.eDataIndex ) );
                        l.uniform->Update();
                    }
                    batch->entity->UpdateUniforms();
                    batch->entity->update();
                    _pass.DrawFunction();
                }
            }
        }
    }

    // Update the lights view to be contained within the scene camera's view
    // frustum. Based on code from ThinMatrix (YouTube)
    void CausticMapping::updateProjectionCamera( 
            std::shared_ptr< Camera > _sceneCamera ){
                
        // Start by getting the various direction vectors of the scene camera
        auto sceneCamForward = glm::normalize( _sceneCamera->getForwardVector() );
        auto sceneCamUp = glm::normalize( _sceneCamera->getUpVector() );
        auto sceneCamRight = glm::normalize( _sceneCamera->getRightVector() );
        auto sceneCamDown = glm::normalize( -sceneCamUp );
        auto sceneCamLeft = glm::normalize( -sceneCamRight );
        auto sceneFarWidth = ( float ) ( this->distance * 
            tan( glm::radians( _sceneCamera->getFov() / 2.0 ) ) );
        auto sceneNearWidth = ( float ) ( _sceneCamera->getNearPlane() *
            tan( glm::radians( _sceneCamera->getFov() / 2.0 ) ) );
        auto sceneFarHeight = sceneFarWidth / _sceneCamera->getAspectRatio();
        auto sceneNearHeight = sceneNearWidth / _sceneCamera->getAspectRatio();

        // Vectors to various horizontal frustum edges
        auto toFar = sceneCamForward * this->distance;
        auto toNear = sceneCamForward * _sceneCamera->getNearPlane();
        auto centreNear = toNear + _sceneCamera->getCameraPosition();
        auto centreFar = toFar + _sceneCamera->getCameraPosition();

        // Centre positions of horizontal frustum edges
        auto farTop = centreFar + ( sceneCamUp * sceneFarHeight );
        auto farBottom = centreFar + ( sceneCamDown * sceneFarHeight );
        auto nearTop = centreNear + ( sceneCamUp * sceneNearHeight );
        auto nearBottom = centreNear + ( sceneCamDown * sceneNearHeight );

        // Treat camera as being at 0,0,0, move projection matrix accordingly
        this->mappingCamera.setCameraPosition( glm::vec3( 0.0, 0.0, 0.0 ) );
        mappingCamera.update();
        auto lightViewMatrix = this->mappingCamera.getViewMatrix();
        
        // Get the frustum corner position from centre point, direction, and edge
        // width
        auto getLightSpaceFrustrumCorner = 
            [ &lightViewMatrix ]
            ( glm::vec3 _start, glm::vec3 dir, float width ) -> glm::vec3
            {
                auto point =
                    glm::vec4( _start + ( dir * width ), 1.0f );
                point = lightViewMatrix * point;
                return glm::vec3( point );
            };
        // Get the points of all 8 frustum corners
        glm::vec3 points[ 8 ];
        points[ 0 ] = getLightSpaceFrustrumCorner( farTop, sceneCamRight,
                                                        sceneFarWidth );
        points[ 1 ] = getLightSpaceFrustrumCorner( farTop, sceneCamLeft,
                                                        sceneFarWidth );
        points[ 2 ] = getLightSpaceFrustrumCorner( farBottom,
                                                        sceneCamRight,
                                                        sceneFarWidth );
        points[ 3 ] = getLightSpaceFrustrumCorner( farBottom, sceneCamLeft,
                                                        sceneFarWidth );
        points[ 4 ] = getLightSpaceFrustrumCorner( nearTop, sceneCamRight,
                                                        sceneNearWidth );
        points[ 5 ] = getLightSpaceFrustrumCorner( nearTop, sceneCamLeft,
                                                        sceneNearWidth );
        points[ 6 ] = getLightSpaceFrustrumCorner( nearBottom,
                                                        sceneCamRight,
                                                        sceneNearWidth );
        points[ 7 ] = getLightSpaceFrustrumCorner( nearBottom,
                                                sceneCamLeft,
                                                sceneNearWidth );
        // Find the box size thats constrained by these points
        float minX = points[ 0 ].x;
        float maxX = points[ 0 ].x;
        float minY = points[ 0 ].y;
        float maxY = points[ 0 ].y;
        float minZ = points[ 0 ].z;
        float maxZ = points[ 0 ].z;
        for( int i = 1; i < 8; i++ ){
            auto point = points[ i ];

            if( point.x > maxX ) maxX = point.x;
            else if( point.x < minX ) minX = point.x;

            if( point.y > maxY ) maxY = point.y;
            else if( point.y < minY ) minY = point.y;

            if( point.z > maxZ ) maxZ = point.z;
            else if( point.z < minZ ) minZ = point.z;
        }
        // Add some offset to avoid overclipping
        auto offset = 10.0f;
        maxZ += offset;
        minZ -= offset;
        maxY += offset;
        minY -= offset;
        maxX += offset;
        minX -= offset;

        // Finally, create the projection matrix
        auto width = maxX - minX;
        auto height = maxY - minY;
        auto length = maxZ - minZ;
        
        this->projectionMatrix = glm::mat4( 1.0f );
        projectionMatrix[ 0 ][ 0 ] = 2.0f / width;
        projectionMatrix[ 1 ][ 1 ] = 2.0f / height;
        projectionMatrix[ 2 ][ 2 ] = -2.0f / length;
        projectionMatrix[ 3 ][ 3 ] = 1.0f;
        this->projectionMatrix = glm::ortho( minX, maxX, minY, maxY, minZ, maxZ );
        this->mappingCamera.setProjectionMatrix( projectionMatrix );
        
        auto boxCentre = glm::vec4( ( minX + maxX ) / 2.0f,
                                    ( minY + maxY ) / 2.0f,
                                    ( minZ + maxZ ) / 2.0f,
                                    1.0f );

        this->direction = glm::normalize( this->direction );
        this->mappingCamera.update( true );



    }



}