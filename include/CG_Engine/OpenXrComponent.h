#ifndef OPENXR_COMPONENT_H
#define OPENXR_COMPONENT_H

#include "InternalObj.h"
#include "IRenderable.h"

#include <memory>

struct GLFWwindow;

namespace GL_Engine {
class IRendererable;
class Renderer;
class Camera;

namespace CG_Data { class FBO; }
}

namespace GL_Engine {

class OpenXrComponent {
public:
    OpenXrComponent();
    void init( GLFWwindow *window );
    void cleanup();

    void update();

    bool bind();
    void unbind();

    void blitToSwapchain();

    void render( std::shared_ptr<IRenderable> renderable );

    void tempSetCamera( std::shared_ptr<Camera> camera );

private:
    bool canRender();

private:
    class XrInternal;
    InternalObj<XrInternal, 1024*8> m_internal;
    uint32_t fboWidth, fboHeight;
};

} // namespace GL_Engine
#endif // OPENXR_COMPONENT_H