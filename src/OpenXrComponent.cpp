#include "OpenXrComponent.h"

#include "Camera.h"
#include "CG_Instance.h"
#include "GlfwComponent.h"
#include "Log.h"
#include "Renderer.h"
#include "xr_linear.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>
#include <vector>
#include <string_view>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <unknwn.h>

#define XR_USE_PLATFORM_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define XR_USE_GRAPHICS_API_OPENGL
#define XR_EXTENSION_PROTOTYPES
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

namespace {

XrResult VerifyResult2( const XrResult &&result, std::string_view callName, const std::initializer_list<XrResult> validResults = {} ) {
    if ( result != XR_SUCCESS && std::ranges::find( validResults, result ) == validResults.end() ) {
        cg_assertMsg( false, "OpenXrComponent::VerifyResult -- XR call {} had unexpected result {}", callName, static_cast<int>( result ) );
    }
    return result;
}

#define VerifyResult( call, ... ) VerifyResult2( call, #call, __VA_ARGS__ )

template<typename T>
std::vector<T> XrEnumerateItemsFunc( XrResult( *func )( uint32_t, uint32_t *, T * ), T &&defaultValue = T() ) {
    uint32_t numItems{ 0 };
    VerifyResult( func( 0, &numItems, nullptr ) );
    std::vector<T> items( numItems, defaultValue );
    VerifyResult( func( numItems, &numItems, items.data() ) );
    return items;
}

template<typename ... Args, typename T>
std::vector<T> XrEnumerateItemsFunc( XrResult( *func )( Args..., uint32_t, uint32_t *, T * ), Args... args, T &&defaultValue = T() ) {
    uint32_t numItems{ 0 };
    VerifyResult( func( args..., 0, &numItems, nullptr ) );
    std::vector<T> items( numItems, defaultValue );
    VerifyResult( func( args..., numItems, &numItems, items.data() ) );
    return items;
}

XrBool32 DebugLayerCallback( XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageTypes, const XrDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData ) {
    std::cout << callbackData->message << std::endl;
    return XR_FALSE;
}
}
namespace GL_Engine {
class OpenXrComponent::XrInternal {
public:
    XrInternal() {
        Log::Debug( "XrInternal constructing" );
    }
    ~XrInternal() {

    }

    void test() {};

    //private:
    bool m_initialised{ false };
    XrInstance m_instance{ XR_NULL_HANDLE };
    XrSession m_session{ XR_NULL_HANDLE };
    XrSwapchain m_swapchain;
    XrFrameState m_frameState;

    std::vector<XrSwapchainImageOpenGLKHR> m_swapchainImages;
    std::unique_ptr<GL_Engine::CG_Data::FBO> m_framebuffer;

    XrSessionState m_sessionState{ XR_SESSION_STATE_UNKNOWN };

    std::array<XrCompositionLayerProjectionView, 2> m_projectionLayerViews;
    std::array<XrCompositionLayerProjection, 1> m_projectionLayers;
    XrSpace m_space;

    GL_Engine::CG_Data::FBO::FramebufferBindToken<GL_DRAW_FRAMEBUFFER> bindToken;

    std::array<std::shared_ptr<CG_Data::FBO>, 2> fbos;

    std::shared_ptr<Camera> m_camera;

    static XrBool32 DebugLayerCallback( XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageTypes, const XrDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData );
};

OpenXrComponent OpenXrComponent::s_component;

OpenXrComponent::OpenXrComponent() : m_internal() {

}

void OpenXrComponent::tempSetCamera( std::shared_ptr<Camera> camera ) {
    m_internal->m_camera = std::move( camera );
}

void OpenXrComponent::initialise() {

    auto properties = XrEnumerateItemsFunc( xrEnumerateApiLayerProperties );

    auto availableExtensions = XrEnumerateItemsFunc<const char *>( xrEnumerateInstanceExtensionProperties, nullptr, { .type = XR_TYPE_EXTENSION_PROPERTIES } );

    std::vector<const char *> sessionExtensions;
    sessionExtensions.reserve( 2 );
    auto checkAndAddExtension = [&sessionExtensions, &availableExtensions] ( std::string_view extensionName ) {
        const bool extensionAvailable = std::ranges::any_of(
            availableExtensions,
            [extensionName] ( const auto &a ) {
                return a.extensionName == extensionName;
            } );

        cg_assertMsg( extensionAvailable, "Failed to find required extension {}", extensionName );

        sessionExtensions.push_back( extensionName.data() );
    };

    checkAndAddExtension( XR_EXT_DEBUG_UTILS_EXTENSION_NAME );
    checkAndAddExtension( XR_KHR_OPENGL_ENABLE_EXTENSION_NAME );
    checkAndAddExtension( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME );

    XrInstanceCreateInfo createInfo;
    createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    createInfo.next = nullptr;
    createInfo.createFlags = 0;
    strncpy( createInfo.applicationInfo.applicationName, "CG_Engine", sizeof( createInfo.applicationInfo.applicationName ) );
    strncpy( createInfo.applicationInfo.engineName, "CG_Engine", sizeof( createInfo.applicationInfo.applicationName ) );
    createInfo.applicationInfo.applicationVersion = 0;
    createInfo.applicationInfo.engineVersion = 0;
    createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

    createInfo.enabledApiLayerCount = 0;
    createInfo.enabledApiLayerNames = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>( sessionExtensions.size() );
    createInfo.enabledExtensionNames = sessionExtensions.data();

    XrDebugUtilsMessengerCreateInfoEXT dumci;

    typedef XrBool32( XRAPI_PTR *PFN_xrDebugUtilsMessengerCallbackEXT )(
        XrDebugUtilsMessageSeverityFlagsEXT              messageSeverity,
        XrDebugUtilsMessageTypeFlagsEXT                  messageTypes,
        const XrDebugUtilsMessengerCallbackDataEXT *callbackData,
        void *userData );

    dumci.next = nullptr;
    dumci.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dumci.messageSeverities =
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dumci.messageTypes =
        XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
    dumci.userData = this;
    dumci.userCallback = &DebugLayerCallback;

    createInfo.next = &dumci;

    VerifyResult( xrCreateInstance( &createInfo, &m_internal->m_instance ) );

    XrSystemId systemId;
    XrSystemGetInfo getInfo;
    getInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    getInfo.next = nullptr;
    getInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    // Will fail if no device found (i.e. not connected)
    if ( VerifyResult( xrGetSystem( m_internal->m_instance, &getInfo, &systemId ), { XR_ERROR_FORM_FACTOR_UNAVAILABLE } ) ) {
        // Failed to find a suitable device
        return;
    }

    XrInstanceProperties instanceProperties{ .type = XR_TYPE_INSTANCE_PROPERTIES };
    VerifyResult( xrGetInstanceProperties( m_internal->m_instance, &instanceProperties ) );

    XrSystemProperties systemProperties{ .type = XR_TYPE_SYSTEM_PROPERTIES };
    VerifyResult( xrGetSystemProperties( m_internal->m_instance, systemId, &systemProperties ) );

    auto blendModes = XrEnumerateItemsFunc<XrInstance, XrSystemId, XrViewConfigurationType>( xrEnumerateEnvironmentBlendModes, m_internal->m_instance, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO );

    auto viewConfigurations = XrEnumerateItemsFunc<XrInstance, XrSystemId>( xrEnumerateViewConfigurations, m_internal->m_instance, systemId );

    XrViewConfigurationProperties viewConfigProps{ .type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES };
    VerifyResult( xrGetViewConfigurationProperties( m_internal->m_instance, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, &viewConfigProps ) );

    auto viewConfigViews = XrEnumerateItemsFunc<XrInstance, XrSystemId, XrViewConfigurationType>(
        xrEnumerateViewConfigurationViews,
        m_internal->m_instance,
        systemId,
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        { .type = XR_TYPE_VIEW_CONFIGURATION_VIEW }
    );


    //xrCreateActionSet();
    //xrCreateAction();
    //xrSuggestInteractionProfileBindings();

    PFN_xrGetOpenGLGraphicsRequirementsKHR pxrGetOpenGLGraphicsRequirementsKHR;
    xrGetInstanceProcAddr( m_internal->m_instance, "xrGetOpenGLGraphicsRequirementsKHR",
        (PFN_xrVoidFunction *)&pxrGetOpenGLGraphicsRequirementsKHR );

    XrGraphicsRequirementsOpenGLKHR graphicsRequirements{ .type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
    VerifyResult( pxrGetOpenGLGraphicsRequirementsKHR( m_internal->m_instance, systemId, &graphicsRequirements ) );

    GlfwComponent *test1 = nullptr;
    IComponent *test2 = test1;
    auto glWindow = getInstance()->getComponent<GlfwComponent>()->getWindow();
    cg_assert( glWindow );

    XrGraphicsBindingOpenGLWin32KHR gBinding;
    gBinding.hDC = GetDC( glfwGetWin32Window( glWindow ) );
    gBinding.hGLRC = glfwGetWGLContext( glWindow );
    gBinding.next = nullptr;
    gBinding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR;

    XrSessionCreateInfo createSessionInfo;
    createSessionInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    createSessionInfo.systemId = systemId;
    createSessionInfo.createFlags = 0;
    createSessionInfo.next = &gBinding;

    VerifyResult( xrCreateSession( m_internal->m_instance, &createSessionInfo, &m_internal->m_session ) );

    auto swapchainFormats = XrEnumerateItemsFunc<XrSession>( xrEnumerateSwapchainFormats, m_internal->m_session );

    fboWidth = viewConfigViews[0].recommendedImageRectWidth;
    fboHeight = viewConfigViews[0].recommendedImageRectHeight;

    XrSwapchainCreateInfo swapchainCreateInfo{ .type = XR_TYPE_SWAPCHAIN_CREATE_INFO };
    swapchainCreateInfo.next = nullptr;
    swapchainCreateInfo.createFlags = 0;
    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
    swapchainCreateInfo.format = GL_RGBA8;
    swapchainCreateInfo.faceCount = 1;
    swapchainCreateInfo.mipCount = 1;
    swapchainCreateInfo.sampleCount = 1;
    swapchainCreateInfo.arraySize = 1;
    swapchainCreateInfo.width = viewConfigViews[0].recommendedImageRectWidth * 2;
    swapchainCreateInfo.height = viewConfigViews[0].recommendedImageRectHeight;
    VerifyResult( xrCreateSwapchain( m_internal->m_session, &swapchainCreateInfo, &m_internal->m_swapchain ) );

    uint32_t numSwapchainImages{ 0 };
    VerifyResult( xrEnumerateSwapchainImages( m_internal->m_swapchain, 0, &numSwapchainImages, nullptr ) );
    std::vector<XrSwapchainImageBaseHeader *> swapchainImagesBase( numSwapchainImages );
    m_internal->m_swapchainImages.resize( numSwapchainImages );
    for ( uint8_t i = 0; i < numSwapchainImages; ++i ) {
        m_internal->m_swapchainImages[i].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
        m_internal->m_swapchainImages[i].next = nullptr;
        swapchainImagesBase[i] = reinterpret_cast<XrSwapchainImageBaseHeader *>( &m_internal->m_swapchainImages[i] );
    }
    VerifyResult( xrEnumerateSwapchainImages( m_internal->m_swapchain, numSwapchainImages, &numSwapchainImages, swapchainImagesBase[0] ) );

    m_internal->m_framebuffer = std::make_unique<GL_Engine::CG_Data::FBO>( viewConfigViews[0].recommendedImageRectWidth * 2, viewConfigViews[0].recommendedImageRectHeight );

    m_internal->fbos[0] = std::make_shared<CG_Data::FBO>( viewConfigViews[0].recommendedImageRectWidth, viewConfigViews[0].recommendedImageRectHeight );
    auto renderColAttach = m_internal->fbos[0]->addAttachment( CG_Data::FBO::AttachmentType::ColourTexture, viewConfigViews[0].recommendedImageRectWidth, viewConfigViews[0].recommendedImageRectHeight );
    m_internal->fbos[0]->addAttachment( CG_Data::FBO::AttachmentType::DepthTexture, viewConfigViews[0].recommendedImageRectWidth, viewConfigViews[0].recommendedImageRectHeight );
    std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>( renderColAttach )->GetTexture()->SetUnit( GL_TEXTURE0 );

    m_internal->fbos[1] = std::make_shared<CG_Data::FBO>( viewConfigViews[0].recommendedImageRectWidth, viewConfigViews[0].recommendedImageRectHeight );
    auto renderColAttach2 = m_internal->fbos[1]->addAttachment( CG_Data::FBO::AttachmentType::ColourTexture, viewConfigViews[0].recommendedImageRectWidth, viewConfigViews[0].recommendedImageRectHeight );
    m_internal->fbos[1]->addAttachment( CG_Data::FBO::AttachmentType::DepthTexture, viewConfigViews[0].recommendedImageRectWidth, viewConfigViews[0].recommendedImageRectHeight );
    std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>( renderColAttach2 )->GetTexture()->SetUnit( GL_TEXTURE0 );

    XrReferenceSpaceCreateInfo refSpaceCreateInfo{
        .type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
        .next = nullptr,
        .referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL,
        .poseInReferenceSpace = {
            .orientation = {
                .x = 0,
                .y = 0,
                .z = 0,
                .w = 1
            },
            .position = {
                .x = 0,
                .y = 0,
                .z = 0
            }
        }
    };

    VerifyResult( xrCreateReferenceSpace( m_internal->m_session, &refSpaceCreateInfo, &m_internal->m_space ) );

    for ( uint8_t i = 0; i < 2; ++i ) {
        m_internal->m_projectionLayerViews[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        m_internal->m_projectionLayerViews[i].next = nullptr;
        m_internal->m_projectionLayerViews[i].subImage.swapchain = m_internal->m_swapchain;
        m_internal->m_projectionLayerViews[i].subImage.imageArrayIndex = 0;
        m_internal->m_projectionLayerViews[i].subImage.imageRect.offset.x = i == 0 ? 0 : viewConfigViews[0].recommendedImageRectWidth;
        m_internal->m_projectionLayerViews[i].subImage.imageRect.offset.y = 0;
        m_internal->m_projectionLayerViews[i].subImage.imageRect.extent.height = viewConfigViews[0].recommendedImageRectHeight;
        m_internal->m_projectionLayerViews[i].subImage.imageRect.extent.width = viewConfigViews[0].recommendedImageRectWidth;
    }

    m_internal->m_projectionLayers[0].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    m_internal->m_projectionLayers[0].next = nullptr;
    m_internal->m_projectionLayers[0].viewCount = 2;
    m_internal->m_projectionLayers[0].views = m_internal->m_projectionLayerViews.data();
    m_internal->m_projectionLayers[0].layerFlags = 0;
    m_internal->m_projectionLayers[0].space = m_internal->m_space;

    m_internal->m_initialised = true;
}

void OpenXrComponent::cleanup() {
    if ( m_internal->m_instance != XR_NULL_HANDLE ) {
        xrDestroyInstance( m_internal->m_instance );
    }
}

void OpenXrComponent::update() {
    uint8_t buffer[XR_MAX_EVENT_DATA_SIZE];
    XrEventDataBuffer *eventData = static_cast<XrEventDataBuffer *>( (void *)buffer );

    while ( true ) {
        eventData->type = XR_TYPE_EVENT_DATA_BUFFER;
        eventData->next = nullptr;
        auto result = xrPollEvent( m_internal->m_instance, eventData );
        if ( result == XR_EVENT_UNAVAILABLE ) {
            return;
        }
        VerifyResult( std::move( result ) );

        switch ( eventData->type ) {
        case XR_TYPE_EVENT_DATA_EVENTS_LOST:
        {
            auto eventData = static_cast<XrEventDataEventsLost *>( (void *)buffer );
            break;
        }
        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
        {
            auto eventData = static_cast<XrEventDataInstanceLossPending *>( (void *)buffer );
            break;
        }
        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
        {
            auto eventData = static_cast<XrEventDataInteractionProfileChanged *>( (void *)buffer );
            break;
        }
        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
        {
            auto eventData = static_cast<XrEventDataReferenceSpaceChangePending *>( (void *)buffer );
            break;
        }
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
        {
            auto eventData = static_cast<XrEventDataSessionStateChanged *>( (void *)buffer );
            m_internal->m_sessionState = eventData->state;
            if ( m_internal->m_sessionState == XR_SESSION_STATE_READY ) {
                XrSessionBeginInfo beginInfo{ .type = XR_TYPE_SESSION_BEGIN_INFO };
                beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                beginInfo.next = nullptr;
                VerifyResult( xrBeginSession( m_internal->m_session, &beginInfo ) );
                break;
            }
        }
        }
    }
}

bool OpenXrComponent::canRender() {
    switch ( m_internal->m_sessionState ) {
    case XR_SESSION_STATE_READY:
    case XR_SESSION_STATE_FOCUSED:
    case XR_SESSION_STATE_VISIBLE:
    case XR_SESSION_STATE_SYNCHRONIZED:
        return true;

    case XR_SESSION_STATE_STOPPING:
    case XR_SESSION_STATE_LOSS_PENDING:
    case XR_SESSION_STATE_EXITING:
    case XR_SESSION_STATE_MAX_ENUM:
    case XR_SESSION_STATE_UNKNOWN:
    case XR_SESSION_STATE_IDLE:
    default:
        return false;
    }
}

bool OpenXrComponent::bind() {
    if ( !canRender() ) {
        return false;
    }

    XrFrameWaitInfo frameWaitInfo{ .type = XR_TYPE_FRAME_WAIT_INFO };
    frameWaitInfo.next = nullptr;
    m_internal->m_frameState.next = nullptr;
    m_internal->m_frameState.type = XR_TYPE_FRAME_STATE;
    auto waitFrameResult = xrWaitFrame( m_internal->m_session, &frameWaitInfo, &m_internal->m_frameState );
    if ( waitFrameResult != XR_EVENT_UNAVAILABLE && waitFrameResult != XR_SUCCESS ) {
        std::exit( -1 );
    }

    XrFrameBeginInfo frameBeginInfo{ .type = XR_TYPE_FRAME_BEGIN_INFO };
    frameBeginInfo.next = nullptr;
    VerifyResult( xrBeginFrame( m_internal->m_session, &frameBeginInfo ) );

    if ( !m_internal->m_frameState.shouldRender ) {
        XrFrameEndInfo frameEndInfo{ .type = XR_TYPE_FRAME_END_INFO };
        frameEndInfo.next = nullptr;
        frameEndInfo.displayTime = m_internal->m_frameState.predictedDisplayTime;
        frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        frameEndInfo.layerCount = 0;
        frameEndInfo.layers = nullptr;

        VerifyResult( xrEndFrame( m_internal->m_session, &frameEndInfo ) );
        return false;
    }

    XrViewLocateInfo viewLocateInfo{ .type = XR_TYPE_VIEW_LOCATE_INFO };
    viewLocateInfo.next = nullptr;
    viewLocateInfo.displayTime = m_internal->m_frameState.predictedDisplayTime;
    viewLocateInfo.space = m_internal->m_space;
    viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    XrViewState viewState{ .type = XR_TYPE_VIEW_STATE };

    const auto views = XrEnumerateItemsFunc<XrSession, const XrViewLocateInfo *, XrViewState *>( xrLocateViews, m_internal->m_session, &viewLocateInfo, &viewState, { .type = XR_TYPE_VIEW } );
    for ( uint8_t i = 0; i < 2; ++i ) {
        m_internal->m_projectionLayerViews[i].fov = views[i].fov;
        m_internal->m_projectionLayerViews[i].pose = views[i].pose;
    }

    XrSwapchainImageAcquireInfo acquireInfo{ .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
    acquireInfo.next = nullptr;

    uint32_t index{ 0 };
    VerifyResult( xrAcquireSwapchainImage( m_internal->m_swapchain, &acquireInfo, &index ) );
    auto image = m_internal->m_swapchainImages[index];

    XrSwapchainImageWaitInfo swapchainWaitInfo = { .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
    swapchainWaitInfo.next = nullptr;
    swapchainWaitInfo.timeout = XR_INFINITE_DURATION;
    VerifyResult( xrWaitSwapchainImage( m_internal->m_swapchain, &swapchainWaitInfo ) );

    m_internal->bindToken = CG_Data::FBO::staticBind<GL_DRAW_FRAMEBUFFER>( { m_internal->m_framebuffer->getID(), glm::vec2( fboWidth, fboHeight ) } );

    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, static_cast<GLuint>( image.image ), 0 );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );
    cg_verify( glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );

    return true;
}

void OpenXrComponent::unbind() {
    XrSwapchainImageReleaseInfo releaseInfo{ .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    releaseInfo.next = nullptr;

    xrReleaseSwapchainImage( m_internal->m_swapchain, &releaseInfo );

    XrFrameEndInfo frameEndInfo{ .type = XR_TYPE_FRAME_END_INFO };
    frameEndInfo.next = nullptr;
    frameEndInfo.displayTime = m_internal->m_frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    frameEndInfo.layerCount = 1;// static_cast<uint32_t>( layersPointers.size() );
    auto projectionLayersData = m_internal->m_projectionLayers.data();
    frameEndInfo.layers = static_cast<XrCompositionLayerBaseHeader **>( (void *)&projectionLayersData );

    VerifyResult( xrEndFrame( m_internal->m_session, &frameEndInfo ) );

    std::move( m_internal->bindToken ).release();
}

void OpenXrComponent::blitToSwapchain() {
    if ( m_internal->m_initialised && bind() ) {
        glBlitFramebuffer( 0, 0, fboWidth, fboHeight, 0, 0, fboWidth, fboHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        glBlitFramebuffer( 0, 0, fboWidth, fboHeight, fboWidth - 400, 0, fboWidth - 400, fboHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        unbind();
    }
}

void OpenXrComponent::render( std::shared_ptr<IRenderable> renderable ) {
    //if ( m_internal->m_initialised && bind() ) {
    //        auto generateCameraRenderNode = [this, renderable] ( uint8_t eyeId ) {
    //            auto renderNode = std::make_shared<CameraRenderNode>();
    //            auto pose = m_internal->m_projectionLayerViews[eyeId].pose;
    //            auto glmQuat = glm::quat( pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z );
    //            XrMatrix4x4f proj;
    //            XrMatrix4x4f_CreateProjectionFov( &proj, GRAPHICS_OPENGL, m_internal->m_projectionLayerViews[eyeId].fov, 0.05f, 100.0f );
    //            glm::mat4 *newProj = reinterpret_cast<glm::mat4 *>( &proj );

    //            renderNode->camera = m_internal->m_camera;
    //            renderNode->camera->translateCamera2( glm::vec3( pose.position.x, pose.position.y, pose.position.z ) );
    //            renderNode->camera->setCameraOrientation( glmQuat );
    //            renderNode->camera->setProjectionMatrix( *newProj );
    //            renderNode->camera->update();

    //            renderNode->target = renderable;

    //            auto fboNode = std::make_shared<CG_Data::FboRenderNodeBase>( m_internal->fbos[eyeId] );
    //           // fboNode->writeTarget = std::move( renderNode );
    //            fboNode->writeTarget = std::make_shared<DebugRenderNode>( ( [this, eyeId] { glClearColor( 0.0f, 0.0f, 1.0f, 1.0f ); glClear( GL_COLOR_BUFFER_BIT ); } ) );
    //            fboNode->readTarget = std::make_shared<DebugRenderNode>( ( [this, eyeId] { glBlitFramebuffer( 0, 0, fboWidth, fboHeight, fboWidth * eyeId, 0, fboWidth * eyeId, fboHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR ); } ) );

    //            return fboNode;
    //        };

    //        const auto prePos = m_internal->m_camera->getCameraPosition();
    //        auto eye1 = generateCameraRenderNode( 0 );
    //        auto eye2 = generateCameraRenderNode( 1 );
    //        CG_Data::FboRenderNodeBase::GenerateDrawRenderNode( eye1 )->execute();
    //        m_internal->m_camera->setCameraPosition( prePos );
    //        CG_Data::FboRenderNodeBase::GenerateDrawRenderNode( eye2 )->execute();
    //        m_internal->m_camera->setCameraPosition( prePos );

    //    //1
    //    // glViewport( 0, 0, fboWidth * 2, fboHeight );
    //    CG_Data::FboRenderNodeBase::GenerateReadRenderNode( eye1 )->execute();
    //    CG_Data::FboRenderNodeBase::GenerateReadRenderNode( eye2 )->execute();
    //    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    //    glClear( GL_COLOR_BUFFER_BIT );
    //    unbind();
    //}

    auto generateCameraRenderNode = [this, renderable] ( uint8_t eyeId ) {
            auto fboNode = std::make_shared<CG_Data::FboRenderNodeBase>( m_internal->fbos[ eyeId ] );
            //auto renderNode = std::make_shared<CameraRenderNode>();
            //auto pose = m_internal->m_projectionLayerViews[ eyeId ].pose;
            //auto glmQuat = glm::quat( pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z );
            //XrMatrix4x4f proj;
            //XrMatrix4x4f_CreateProjectionFov( &proj, GRAPHICS_OPENGL, m_internal->m_projectionLayerViews[ eyeId ].fov, 0.05f, 100.0f );
            //glm::mat4 *newProj = reinterpret_cast<glm::mat4 *>( &proj );


            //renderNode->camera = m_internal->m_camera;
            //renderNode->camera->translateCamera2( glm::vec3( pose.position.x, pose.position.y, pose.position.z ) );
            //renderNode->camera->setCameraOrientation( glmQuat );
            //renderNode->camera->setProjectionMatrix( *newProj );
            //renderNode->camera->update();

            //renderNode->target = renderable;
            //fboNode->writeTarget = renderNode;

            fboNode->writeTarget = std::make_shared<DebugRenderNode>( ( [this, eyeId, renderable] {
                auto renderNode = std::make_shared<CameraRenderNode>();
                auto pose = m_internal->m_projectionLayerViews[ eyeId ].pose;
                auto glmQuat = glm::quat( pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z );
                XrMatrix4x4f proj;
                XrMatrix4x4f_CreateProjectionFov( &proj, GRAPHICS_OPENGL, m_internal->m_projectionLayerViews[ eyeId ].fov, 0.05f, 100.0f );
                glm::mat4 *newProj = reinterpret_cast<glm::mat4 *>( &proj );

                renderNode->camera = m_internal->m_camera;
                renderNode->camera->translateCamera2( glm::vec3( pose.position.x, pose.position.y, pose.position.z ) );
                renderNode->camera->setCameraOrientation( glmQuat );
                renderNode->camera->setProjectionMatrix( *newProj );
                renderNode->camera->update();

                renderNode->target = renderable;
                renderNode->execute();
            } ) );

            fboNode->readTarget = std::make_shared<DebugRenderNode>( ( [this, eyeId] {
                glBlitFramebuffer( 0, 0, fboWidth, fboHeight, fboWidth * eyeId, 0, fboWidth * (eyeId + 1), fboHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            } ) );

            //fboNode->readTarget = std::make_shared<DebugRenderNode>( ( [this, eyeId] { glBlitFramebuffer( 0, 0, fboWidth, fboHeight, fboWidth * eyeId, 0, fboWidth * eyeId, fboHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR ); } ) );
        return fboNode;
    };

    const auto prePos = m_internal->m_camera->getCameraPosition();
    auto eye1 = generateCameraRenderNode( 1 );
    auto eye2 = generateCameraRenderNode( 0 );

    m_internal->m_camera->setCameraPosition( prePos );

    CG_Data::FboRenderNodeBase::GenerateDrawRenderNode( eye1 )->execute();
    m_internal->m_camera->setCameraPosition( prePos );
    CG_Data::FboRenderNodeBase::GenerateDrawRenderNode( eye2 )->execute();
    m_internal->m_camera->setCameraPosition( prePos );

    if ( m_internal->m_initialised && bind() ) {

        //glBindFramebuffer( GL_READ_FRAMEBUFFER, eye1->m_fbo->getID() );
        //CG_Data::FBO::ReadFramebufferStack.push_back( { eye1->m_fbo->getID(), { eye1->m_fbo->width, eye1->m_fbo->height } } );
        //auto a = CG_Data::FBO::FramebufferBindToken<GL_READ_FRAMEBUFFER>( { eye1->m_fbo->getID(), { eye1->m_fbo->width, eye1->m_fbo->height } } );
        glBindFramebuffer( GL_READ_FRAMEBUFFER, eye1->m_fbo->getID() );
        glViewport( 0, 0, eye1->m_fbo->width, eye1->m_fbo->height );
        //auto a = CG_Data::FBO::staticBind<GL_READ_FRAMEBUFFER>( { eye1->m_fbo->getID(), {eye1->m_fbo->width, eye1->m_fbo->height }}, CG_Data::FBO::ReadFramebufferStack );
       //UsingScopedToken( eye1->m_fbo->bind<GL_READ_FRAMEBUFFER>() ) {
            glBlitFramebuffer( 0, 0, fboWidth, fboHeight, 0, 0, fboWidth, fboHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            //glBlitFramebuffer( 0, 0, fboWidth, fboHeight, fboWidth * 1, 0, fboWidth * ( 1 + 1 ), fboHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR );
       // }

        //std::move( a ).release();
        //auto b = CG_Data::FBO::staticBind<GL_READ_FRAMEBUFFER>( { eye2->m_fbo->getID(), {eye2->m_fbo->width, eye2->m_fbo->height } }, CG_Data::FBO::ReadFramebufferStack );
        glBindFramebuffer( GL_READ_FRAMEBUFFER, eye2->m_fbo->getID() );
        glViewport( 0, 0, eye2->m_fbo->width, eye2->m_fbo->height );
        //UsingScopedToken( eye2->m_fbo->bind<GL_READ_FRAMEBUFFER>() ) {
            glBlitFramebuffer( 0, 0, fboWidth, fboHeight, fboWidth, 0, fboWidth * 2, fboHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        //}
            //std::move( b ).release();

        //CG_Data::FboRenderNodeBase::GenerateReadRenderNode( eye1 )->execute();
        //CG_Data::FboRenderNodeBase::GenerateReadRenderNode( eye2 )->execute();
        unbind();

        glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
    }
        //1
}

} // namespace GL_Engine