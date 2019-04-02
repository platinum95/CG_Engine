#include "GUI.h"

namespace GL_Engine{


GUI::GUI( std::shared_ptr< RenderPass > _rPass,
        std::shared_ptr< CG_Data::Texture > _texture,
        glm::vec2 _topLeft, glm::vec2 _bottomRight,
        std::filesystem::path fragmentPath ){

    uint8_t vPosAttribInd = 0;
    uint8_t tPosAttribInd = 1;
    this->active = true;

    if( _rPass->shader == nullptr ){
        std::string defaultVertexShader = "\
        #version 330\n \
        in vec2 vPos;\n \
        in vec2 tPos;\n \
        out vec2 tPosPass;\n \
        void main(){\n \
            tPosPass = tPos;\n \
            gl_Position = vec4( vPos, 0.0, 1.0 );\n \
        }\n \
        ";
    
        std::string defaultFragmentShader = "\
        #version 330\n \
        in vec2 tPosPass;\n \
        out vec4 fragColour;\n \
        uniform sampler2D guiTexture;\n \
        void main(){\n \
            fragColour = texture( guiTexture, tPosPass );\n \
        }\n \
        ";

        guiShader.registerShaderStage( defaultVertexShader, GL_VERTEX_SHADER );
        if( fragmentPath == "" ){
            guiShader.registerShaderStage( defaultFragmentShader, 
                                        GL_FRAGMENT_SHADER );
        }
        else{
            guiShader.registerShaderStageFromFile( fragmentPath, 
                                                   GL_FRAGMENT_SHADER );
        }
        guiShader.registerAttribute( "vPos", vPosAttribInd );
        guiShader.registerAttribute( "tPos", tPosAttribInd );
        guiShader.registerTextureUnit( "guiTexture", 0 );
        guiShader.compileShader();

        _rPass->shader = &guiShader;
    }

    float vPos[ 8 ] = {
        _topLeft.x,     _topLeft.y,
        _bottomRight.x, _topLeft.y,
        _bottomRight.x, _bottomRight.y,
        _topLeft.x,     _bottomRight.y
    };
    float tPos[ 8 ] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };
    uint32_t vInd[ 6 ] = {
        0, 3, 1,
        1, 3, 2
    };

    this->guiVao = std::make_shared< CG_Data::VAO >();
    this->guiVao->BindVAO();
    auto vPosVbo = std::make_unique< CG_Data::VBO >( &vPos, 8 * sizeof( float ),
                                                    GL_STATIC_DRAW );
    auto tPosVbo = std::make_unique< CG_Data::VBO >( &tPos, 8 * sizeof( float ),
                                                    GL_STATIC_DRAW );
    auto vIndVbo = std::make_unique< CG_Data::VBO >( &vInd, 
                                                     6 * sizeof( uint32_t ),
                                                     GL_STATIC_DRAW,
                                                     GL_ELEMENT_ARRAY_BUFFER );

    vPosVbo->BindVBO();
    glVertexAttribPointer( vPosAttribInd, 2, GL_FLOAT, GL_FALSE, 0, nullptr );
    glEnableVertexAttribArray( vPosAttribInd );
    
    tPosVbo->BindVBO();
    glVertexAttribPointer( tPosAttribInd, 2, GL_FLOAT, GL_FALSE, 0, nullptr );
    glEnableVertexAttribArray( tPosAttribInd );

    
    this->guiVao->AddVBO( std::move( vPosVbo ) );
    this->guiVao->AddVBO( std::move( tPosVbo ) );
    this->guiVao->AddVBO( std::move( vIndVbo ) );

    _rPass->BatchVao = this->guiVao;
    _rPass->renderFunction = DefaultRenderer;
    _rPass->Data = &this->active;
    _rPass->Textures.push_back( std::move( _texture ) );
    this->renderPass = std::move( _rPass );

}

void GUI::setActive( bool _active ){
    this->active = _active;
}


void GUI::DefaultRenderer( RenderPass& _rPass, void* _data ) {
    bool active = *( bool * )_data;
    if( active ){
        _rPass.shader->useShader();
        _rPass.BatchVao->BindVAO();
        for (auto tex : _rPass.Textures) {
            tex->Bind();
        }
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
    }
}





}