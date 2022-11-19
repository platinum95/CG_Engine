#ifndef CAMERA_H
#define CAMERA_H

#include "CG_Data.h"
#include "glad.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <memory>

namespace GL_Engine {

    struct CameraUboData {
        float viewMatrix[16];
        float projectionMatrix[16];
        float pvMatrix[16];
        float cameraPosition[4];
        float cameraOrientation[4];
        float clippingPlane[4];
    };

    class Camera {
    public:
        Camera();
        ~Camera();

        // Initialise the gl components of the camera (MUST BE CALLED)
        void initialise();

        // Create a new projection matrix based on the given parameters
        const glm::mat4 & setProjectionMatrix( float _nearPlane, float _farPlane, float _fov, float _aspectRatio );

        // Set the camera's projection matrix to the given matrix
        void setProjectionMatrix( const glm::mat4 &_projection );

        // Move the camera tot he given position
        void setCameraPosition( const glm::vec3 &_position );

        void setCameraOrientation( const glm::quat &orientation );

        // Move the camera by the given vector
        const glm::vec3 & translateCamera( const glm::vec3 &_Translation );

        // Update the internal structures, return the given camera data
        const std::shared_ptr<CG_Data::UBO> update( bool force=false );

        // Get the camera UBO
        const std::shared_ptr<CG_Data::UBO> getCameraUbo() const;

        // Get the camera UBO data
        const CameraUboData* getCameraUboData() const;

        // Get the current view matrix
        const glm::mat4& getViewMatrix();

        // Get the current projection matrix
        const glm::mat4& getProjectionMatrix() const;

        // Get the current position of the camera
        const glm::vec3& getCameraPosition() const;

        // Get the current forward direction of the camera
        const glm::vec3& getForwardVector() const;

        // Get the current up direction of the camera
        const glm::vec3& getUpVector() const;

        // Get the current right direction of the camera
        const glm::vec3& getRightVector() const;

        // Get the current quaternion orientation of the camera
        const glm::quat& getOrientation() const;

        // Reflect the camera about the 0-horizontal plane
        void reflectCamera();
    
        // Pitch the camera (about right-vector) by a given number of degrees
        void pitchBy( float _pitchDegrees );

        // Roll the camera (about forward-vector) by a given number of degrees
        void rollBy( float _rollDegrees );

        // Yaw the camera (about up-vector) by a given number of degrees
        void yawBy( float _yawDegrees );

        // Direct the camera to look in a given orientation for 
        // environment mapping
        void environDirect( GLuint direction );

        float getFarPlane();
        float setFarPlane( float _farPlane );
        float getNearPlane();
        float setNearPlane( float _nearPlane );
        float getFov();
        float setFov( float _fov );
        float getAspectRatio();
        float setAspectRatio( float _aspectRatio );

    private:
        void generateViewMatrix();
        glm::mat4 viewMatrix, projectionMatrix, pvMatrix;
        glm::vec3 cameraPosition{ 0, 0, 0 };
        glm::quat orientation; 
        glm::vec3 forwardVector{ 0.0f, 0.0f, 1.0f }, rightVector{ 1.0f, 0.0f, 0.0f }, upVector{ 0.0f, 1.0f, 0.0f };
        glm::vec3 rotationEuler{ 0.0f, 0.0f, 0.0f };

        CameraUboData cameraUboData;

        std::shared_ptr<CG_Data::UBO> cameraUbo;

        bool updateViewMatrix{ true };
        bool updateProjMatrix{ true };

        float farPlane, nearPlane, fov, aspectRatio;
    };
}

#endif // CAMERA_H
