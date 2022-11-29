#ifndef OPENGL_COMPONENT_H
#define OPENGL_COMPONENT_H

#include "IComponent.h"
#include "IRenderable.h"
namespace GL_Engine {

class OpenGlComponent : public IComponent {
public:

    void initialise() override {
        cg_assertMsg( gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ), "OpenGlComponent::initialise -- Failed to load OpenGL" );
    }

    void update() override {
    };
};


class FramebufferCopyRenderNode : public IRenderable {
public:
    void execute() override {
        glBlitFramebuffer( 0, 0, 1920, 1080, 0, 0, 1920, 1080, GL_COLOR_BUFFER_BIT, GL_LINEAR );
    }
};

} // namespace GL_Engine
#endif OPENGL_COMPONENT_H