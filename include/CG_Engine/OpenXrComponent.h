#ifndef OPENXR_COMPONENT_H
#define OPENXR_COMPONENT_H

#include "InternalObj.h"
#include "IComponent.h"
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

class OpenXrComponent : public IComponent {
public:
    OpenXrComponent();
    void cleanup();

    void update() override;
    static OpenXrComponent* get() { return &s_component; }

    bool bind();
    void unbind();

    void blitToSwapchain();

    void render( std::shared_ptr<IRenderable> renderable );

    void tempSetCamera( std::shared_ptr<Camera> camera );

private:
    void initialise() override;

    bool canRender();

private:
    class XrInternal;
    InternalObj<XrInternal, 512> m_internal;
    uint32_t fboWidth, fboHeight;
    static OpenXrComponent s_component;
};

} // namespace GL_Engine
#endif // OPENXR_COMPONENT_H