#ifndef OPENXR_COMPONENT_H
#define OPENXR_COMPONENT_H

#include <CG_Data.h>

#include<unknwn.h>

#define XR_USE_PLATFORM_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define XR_USE_GRAPHICS_API_OPENGL
#define XR_EXTENSION_PROTOTYPES
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <array>

struct GLFWwindow;

class OpenXrComponent {
public:
	OpenXrComponent() = default;
	void init( GLFWwindow *window );
	void cleanup();

	void update();

	bool bind();
	void unbind();

	void blitToSwapchain();

	uint32_t fboWidth, fboHeight;
private:
	bool canRender();

private:
	bool m_initialised{ false };
	XrInstance m_instance{ XR_NULL_HANDLE };
	XrSession m_session{ XR_NULL_HANDLE };
	XrSwapchain m_swapchain;
	XrFrameState m_frameState;


	std::vector<XrSwapchainImageOpenGLKHR> m_swapchainImages;
	std::unique_ptr<GL_Engine::CG_Data::FBO> m_framebuffer;

	XrSessionState m_sessionState{ XR_SESSION_STATE_UNKNOWN };

	std::array<XrCompositionLayerProjectionView,2> m_projectionLayerViews;
	std::array<XrCompositionLayerProjection,1> m_projectionLayers;
	XrSpace m_space;

	static XrBool32 DebugLayerCallback( XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageTypes, const XrDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData );
};

#endif // OPENXR_COMPONENT_H