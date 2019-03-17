#ifndef GUI_H
#define GUI_H

#include "Renderer.h"
#include "CG_Data.h"
#include "Shader.h"

namespace GL_Engine{

class GUI {
    public:
    GUI( std::shared_ptr< RenderPass > _rPass,
         std::shared_ptr< CG_Data::Texture > _texture,
         glm::vec2 _topLeft, glm::vec2 _bottomRight,
         std::filesystem::path fragmentPath="" );

    void setActive( bool _active );


    private:
    Shader guiShader;

    std::shared_ptr< CG_Data::VAO > guiVao;
    std::shared_ptr< RenderPass > renderPass;
    bool active;

    static void DefaultRenderer( RenderPass& _rPass, void* _data );

};


}
#endif