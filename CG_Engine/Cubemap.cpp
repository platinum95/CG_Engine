#include "Cubemap.h"

namespace GL_Engine {
    const float Cubemap::vertices[24] = {
        -1, 1, -1,		//0 T1
        -1, -1, -1,		//1 T1
        1, -1, -1,		//2 T2
        1, 1, -1,			//3 T1


        -1, 1, 1,			//4 T1
        -1, -1, 1,		//5 T1
        1, -1, 1,			//6 T2
        1, 1,  1			//7 T1

    };

    //GL_QUAD Clockwise
    const unsigned int Cubemap::indices[36] = {
        0, 1, 3, 1, 2, 3,	//F1 Front
        2, 3, 7, 2, 7, 6,	//F2 Right
        4, 5, 7, 5, 7, 6,	//F3 Back
        0, 1, 4, 1, 4, 5,	//F4 Left
        1, 2, 6, 1, 6, 5,	//F5 Bottom
        0, 7, 4, 0, 7, 3	//F6 Top
    };


    Cubemap::Cubemap(const std::vector<std::string> &_TextureFiles, Shader *_CubemapShader, Renderer *_Renderer) {
        CubemapShader = _CubemapShader;
        GenerateCubemap(_TextureFiles);
        SetupArrayObjects();
        SetupRenderPass(_Renderer);
        CubeRenderPass->shader = this->CubemapShader;
        CreateShader(_CubemapShader);
    }
    Cubemap::~Cubemap() {
        
    }

    std::shared_ptr<CG_Data::Texture> Cubemap::getTextureMap(){
        return this->MapTexture;
    }

    void Cubemap::GenerateCubemap(const std::vector<std::string> &_TextureFiles) {
        MapTexture = std::make_shared<CG_Data::Texture>(GL_TEXTURE0, GL_TEXTURE_CUBE_MAP);
        MapTexture->Bind();

        GLenum type = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        for (auto str : _TextureFiles) {
            int width, height, nChannels;
            void *data = File_IO::LoadImageFile(str, width, height, nChannels, false);
            glTexImage2D(type++, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            File_IO::FreeImageData(data);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    const std::shared_ptr< RenderPass > Cubemap::GetRenderPass() const { 
        return this->CubeRenderPass;
    }

    void Cubemap::CreateShader(Shader *_Shader) {
        
    }
    void Cubemap::SetupRenderPass(Renderer *_Renderer) {
        this->CubeRenderPass = _Renderer->AddRenderPass(this->CubemapShader, std::function<void(RenderPass &, void*)>(Cubemap::CubemapRenderer), nullptr);
        CubeRenderPass->SetDrawFunction([]() {glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); });
        CubeRenderPass->BatchVao = CubemapVAO;
        CubeRenderPass->Textures.push_back(MapTexture);
    }
    void Cubemap::SetupArrayObjects() {
        CubemapVAO = std::make_shared<CG_Data::VAO>();
        CubemapVAO->BindVAO();
        std::unique_ptr<CG_Data::VBO> indexVBO = std::make_unique<CG_Data::VBO>((void*)&indices[0], 36 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
        CubemapVAO->AddVBO(std::move(indexVBO));
        std::unique_ptr<CG_Data::VBO> meshVBO = std::make_unique<CG_Data::VBO>((void*)&vertices[0], 24 * sizeof(float), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
        CubemapVAO->AddVBO(std::move(meshVBO));
    }

    void Cubemap::CubemapRenderer(RenderPass& _Pass, void* _Data) {
        _Pass.shader->UseShader();
        _Pass.BatchVao->BindVAO();
        _Pass.Textures[0]->Bind();
        _Pass.DrawFunction();
    }




    EnvironmentMap::EnvironmentMap( const glm::vec3 & _centre, uint16_t _fbSize ){
        this->envCamera.initialise();
        this->envCamera.setCameraPosition( _centre );
        this->envCamera.setProjectionMatrix( 0.01f, 1000.0f, 90.0, 1.0 );
        this->fbSize = _fbSize;

        this->dynamicFbo = std::make_unique< CG_Data::FBO >( this->fbSize, this->fbSize );
        auto depthBuffer = dynamicFbo->addAttachment(
                             CG_Data::FBO::AttachmentType::DepthAttachment,
                             this->fbSize, this->fbSize );

        auto parameters = []() {
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				};
 
        this->staticColourTex = std::make_shared<CG_Data::Texture>(GL_TEXTURE0, GL_TEXTURE_CUBE_MAP, this->fbSize, this->fbSize, parameters );
        this->staticColourTex->Bind();
        for ( GLuint i = 0; i < 6; i++ ) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, this->fbSize,
                this->fbSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
        }

        this->dynamicColourTex = std::make_shared< CG_Data::Texture >( GL_TEXTURE0,  GL_TEXTURE_CUBE_MAP, this->fbSize, this->fbSize, parameters );
        this->dynamicColourTex->Bind();
        for ( GLuint i = 0; i < 6; i++ ) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, this->fbSize,
                this->fbSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
        }

        this->staticDepthTex = std::make_shared< CG_Data::Texture >( GL_TEXTURE0, GL_TEXTURE_CUBE_MAP, this->fbSize, this->fbSize, parameters );
        this->staticDepthTex->Bind();
        for ( GLuint i = 0; i < 6; i++ ) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, this->fbSize,
                this->fbSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr );
        }

        this->dynamicDepthTex = std::make_shared< CG_Data::Texture >( GL_TEXTURE0, GL_TEXTURE_CUBE_MAP, this->fbSize, this->fbSize, parameters );
        this->dynamicDepthTex->Bind();
        for ( GLuint i = 0; i < 6; i++ ) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, this->fbSize,
                this->fbSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr );
        }

        
    }

    EnvironmentMap::~EnvironmentMap(){
        this->cleanup();
    }

    void EnvironmentMap::cleanup(){
        if( dynamicFbo ){
            dynamicFbo->cleanup();
            dynamicFbo.reset();
            this->dynamicColourTex->Cleanup();
            this->staticColourTex->Cleanup();
            this->staticDepthTex->Cleanup();
            this->dynamicDepthTex->Cleanup();
        }
    }

    void EnvironmentMap::setStaticRenderer( std::shared_ptr< Renderer > _staticRenderer ){
        this->staticRenderer = std::move( _staticRenderer );
        this->staticRenderer->AddUBO( this->envCamera.getCameraUbo().get() );
    }

    void EnvironmentMap::setDynamicRenderer( std::shared_ptr< Renderer > _dynamicRenderer ){
        this->dynamicRenderer = std::move( _dynamicRenderer );
    }

    std::shared_ptr< CG_Data::Texture > EnvironmentMap::getStaticTexture(){
        return this->staticColourTex;
    }

	std::shared_ptr< CG_Data::Texture > EnvironmentMap::getDynamicTexture(){
        return this->dynamicColourTex;
    }

    const std::shared_ptr< CG_Data::UBO > EnvironmentMap::getCameraUbo() const{
        return this->envCamera.getCameraUbo();
    }

    const Camera & EnvironmentMap::getCamera() const {
        return this->envCamera;
    }

    void EnvironmentMap::renderStaticMap(){
        auto fbo = CG_Data::FBO( this->fbSize, this->fbSize );
        auto depthBuffer = fbo.addAttachment(
                             CG_Data::FBO::AttachmentType::DepthAttachment,
                             this->fbSize, this->fbSize );
        fbo.bind( 0 );

        // Render in all directions
        for( uint8_t i = 0; i < 6; i++ ){
            this->envCamera.environDirect( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i );
            this->envCamera.update();
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                                    this->staticColourTex->GetID(), 0 );

            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                                    this->staticDepthTex->GetID(), 0 );

            
            glClearColor( 0.5f, 0.2f, 0.1f, 1.0f );
            glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
            this->staticRenderer->Render();
        }

        fbo.unbind();
        fbo.cleanup();
    }

    void EnvironmentMap::renderDynamicMap(){

        this->dynamicFbo->bind( 0 );

        // Render in all directions
        for( uint8_t i = 0; i < 6; i++ ){
            this->envCamera.environDirect( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i );
            this->envCamera.update();
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                                    this->dynamicColourTex->GetID(), 0 );

            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                                    this->dynamicDepthTex->GetID(), 0 );

            glClearColor( 0.5f, 0.2f, 0.1f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            this->dynamicRenderer->Render();
        }
        this->dynamicFbo->unbind();

    }

}