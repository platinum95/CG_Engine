#ifndef GLFW_COMPONENT_H
#define GLFW_COMPONENT_H

#include "IComponent.h"
#include "IRenderable.h"

#include <GLFW/glfw3.h>

namespace GL_Engine {

class GlfwComponent : public IComponent {
public:
    GlfwComponent() = delete;

    GlfwComponent( uint16_t width, uint16_t height ) 
        : m_width( width )
        , m_height( height ) {}

    void initialise() override {
        cg_verify( glfwInit() );

        constexpr bool fullscreen = false;
        constexpr auto title = "Test";
        m_monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

        m_window = glfwCreateWindow( m_width, m_height, title, m_monitor, nullptr );
        cg_assertMsg( m_window, "GlfwComponent::initialise -- Failed to create GLFW Window" );

        glfwMakeContextCurrent( m_window );
        glfwSetWindowSize( m_window, m_width, m_height );
    }

    void update() override {
    };

    std::unique_ptr<IRenderable> generatePresentationNode() {
        return std::make_unique<GlfwPresentationRenderNode>( m_window );
    }

    GLFWwindow *getWindow() { return m_window; }

private:
    uint16_t m_width, m_height;
    GLFWmonitor *m_monitor;
    GLFWwindow *m_window;

    class GlfwPresentationRenderNode : public IRenderable {
    public:
        GlfwPresentationRenderNode( GLFWwindow *window )
            : m_window( window ){}

        void execute() {
            glfwSwapBuffers( m_window );
        }
    private:
        GLFWwindow *m_window;
    };
};


} // namespace GL_Engine
#endif GLFW_COMPONENT_H