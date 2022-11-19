#ifndef OPENXR_COMPONENT_H
#define OPENXR_COMPONENT_H

#include "InternalObj.h"

struct GLFWwindow;

namespace GL_Engine {
class Renderer;
class Camera;

namespace CG_Data { class FBO; }
}

class OpenXrComponent {
public:
    OpenXrComponent() = default;
    void init( GLFWwindow *window );
    void cleanup();

    void update();

    bool bind();
    void unbind();

    void blitToSwapchain();

    void render( std::array<GL_Engine::CG_Data::FBO *, 2> fbos, GL_Engine::Renderer *renderer );

    uint32_t fboWidth, fboHeight;
    GL_Engine::Camera *camera;
private:
    bool canRender();

private:
    class XrInternal;
    InternalObj<XrInternal, 1024> m_internal;
};

#endif // OPENXR_COMPONENT_H