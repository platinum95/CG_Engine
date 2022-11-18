#include "Camera.h"

#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <iostream>

namespace GL_Engine {

    Camera::Camera() {
        // Initialise the camera (look forward)
        orientation = glm::quatLookAt( glm::vec3( 0, 0, 1 ), 
                                       glm::vec3( 0, 1, 0 ) );
        this->cameraPosition = glm::vec3( 0, 0, 0 );
        generateViewMatrix();

    }

    void Camera::initialise(){
        // Create the data UBO
        this->cameraUbo = std::make_shared< CG_Data::UBO >( 
                            ( void * ) & this->cameraUboData, 
                            sizeof( this->cameraUboData ) );
        this->updateViewMatrix = true;
        this->update();
    }


    Camera::~Camera() {
        this->cameraUbo.reset();
    }

    const glm::mat4 & Camera::setProjectionMatrix( float _nearPlane,
                                                   float _farPlane,
                                                   float _fov,
                                                   float _aspectRatio ){
        this->nearPlane = _nearPlane;
        this->farPlane = _farPlane;
        this->fov = _fov;
        this->aspectRatio = _aspectRatio;
        // Construct the projection matrix using GLM
        float fov_radians = glm::radians(_fov);
        this->projectionMatrix = glm::perspective( fov_radians,
                                                   _aspectRatio,
                                                   _nearPlane,
                                                   _farPlane );

        // Copy the project matrix into the UBO struct
        memcpy( this->cameraUboData.projectionMatrix,
                glm::value_ptr( this->projectionMatrix ),
                sizeof( float ) * 16 );

        // Need to re-set the PV matrix too
        this->updateViewMatrix = true;
        return this->projectionMatrix;
    }

    void Camera::setProjectionMatrix( const glm::mat4 &_projection ) {
        this->projectionMatrix = _projection;

        // Copy the project matrix into the UBO struct
        memcpy( this->cameraUboData.projectionMatrix,
                glm::value_ptr( this->projectionMatrix ),
                sizeof( float ) * 16 );

        // Need to re-set the PV matrix too
        this->updateViewMatrix = true;
    }
    // Get the versor required to rotate start to dest. Taken from SO.
    glm::quat RotationBetweenVectors( glm::vec3 start, glm::vec3 dest ){
        start = glm::normalize(start);
        dest = glm::normalize(dest);

        float cosTheta = dot(start, dest);
        glm::vec3 rotationAxis;

        if (cosTheta < -1 + 0.001f){
            // special case when vectors in opposite directions:
            // there is no "ideal" rotation axis
            // So guess one; any will do as long as it's perpendicular to start
            rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);

            if (glm::length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
                rotationAxis = glm::cross( glm::vec3(1.0f, 0.0f, 0.0f), start );

            rotationAxis = normalize(rotationAxis);
            return glm::angleAxis(glm::radians(180.0f), rotationAxis);
        }

        rotationAxis = cross(start, dest);

        float s = sqrt( (1+cosTheta)*2 );
        float invs = 1 / s;

        return glm::quat(
            s * 0.5f, 
            rotationAxis.x * invs,
            rotationAxis.y * invs,
            rotationAxis.z * invs
        );
    }

    void Camera::reflectCamera() {
        static Camera backup;
        static bool reflected = false;
        if (reflected == false) {
            backup = *this;
            this->cameraPosition.y *= -1.0;
            glm::vec3 newForward = glm::reflect( this->forwardVector, glm::vec3( 0, 1, 0 ) );
            glm::quat rot = RotationBetweenVectors( this->forwardVector, newForward );
            this->orientation = rot * orientation;
            this->rotationEuler.x = -this->rotationEuler.x;
            reflected = true;
            this->updateViewMatrix = true;
            this->update();
            //this->generateViewMatrix();
        }
        else {
            reflected = false;
            *this = backup;
        }
    }
    void Camera::setCameraPosition( const glm::vec3 &_position ) {
        this->cameraPosition = _position;
        this->viewMatrix[3] = glm::vec4( this->cameraPosition, 1.0 );
        this->updateViewMatrix = true;
        return;
    }

    void Camera::setCameraOrientation( const glm::quat &orientation ) {
        this->orientation = orientation;
        glm::mat4 R = glm::toMat4( this->orientation );

        // Reset orientation vectors based on rotation matrix
        this->forwardVector = glm::vec3( R * glm::vec4( 0, 0, -1, 0 ) );
        this->upVector = glm::vec3( R * glm::vec4( 0, 1, 0, 0 ) );
        this->rightVector = glm::vec3( R * glm::vec4( 1, 0, 0, 0 ) );
        this->updateViewMatrix = true;
    }

    const glm::vec3 & 
    Camera::translateCamera( const glm::vec3 &_translation ) {
        this->cameraPosition += this->forwardVector * _translation.z;
        this->cameraPosition += this->rightVector * _translation.x;
        this->cameraPosition += this->upVector * _translation.y;
        this->updateViewMatrix = true;
        return this->cameraPosition;
    }

    void Camera::pitchBy( float _pitch ) {
        this->rotationEuler.x += _pitch;
        float radians = glm::radians( _pitch );
        glm::quat versor = glm::angleAxis( radians, this->rightVector );

        orientation = versor * orientation;
        this->forwardVector = glm::rotate(versor, this->forwardVector);
        this->upVector = glm::rotate(versor, this->upVector);
        this->updateViewMatrix = true;
    }

    void Camera::rollBy( float _roll ) {
        this->rotationEuler.z += _roll;
        float Radians = glm::radians( _roll );
        glm::quat versor = glm::angleAxis( Radians, this->forwardVector );

        orientation = versor * orientation;
        this->rightVector = glm::rotate( versor, this->rightVector );
        this->upVector = glm::rotate( versor, this->upVector );
        this->updateViewMatrix = true;
    }

    void Camera::yawBy( float _yaw ) {
        this->rotationEuler.y += _yaw;
        float Radians = glm::radians( _yaw );
        glm::quat versor = glm::angleAxis( Radians, this->upVector );

        orientation = versor * orientation;
        this->forwardVector = glm::rotate( versor, this->forwardVector );
        this->rightVector = glm::rotate( versor, this->rightVector );
        this->updateViewMatrix = true;
    }

    void Camera::environDirect( GLuint direction ){
        switch( direction ){
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: {
                orientation = glm::quatLookAt( glm::vec3( 1, 0, 0 ),
                                               glm::vec3( 0, -1, 0 ) );
                break;
            }
            
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: {
                orientation = glm::quatLookAt( glm::vec3( -1, 0, 0 ),
                                               glm::vec3( 0, -1, 0 ) );
                break;
            }
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: {
                orientation = glm::quatLookAt( glm::vec3( 0, 1, 0 ),
                                               glm::vec3( 0, 0, 1 ) );
                break;
            }
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: {
                orientation = glm::quatLookAt( glm::vec3( 0, -1, 0 ),
                                               glm::vec3( 0, 0, -1 ) );
                break;
            }
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X: {
                orientation = glm::quatLookAt( glm::vec3( 0, 0, -1 ),
                                               glm::vec3( 0, -1, 0 ) );
                break;
            }
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: {
                orientation = glm::quatLookAt( glm::vec3( 0, 0, 1 ),
                                               glm::vec3( 0, -1, 0 ) );
                break;
            }
        }
        this->updateViewMatrix = true;
    }


    const std::shared_ptr< CG_Data::UBO > Camera::update( bool force ){
        // No need to continue if the camera hasn't moved
        if( updateViewMatrix || force ){
            generateViewMatrix();
            auto vMatrix = this->viewMatrix;
            auto & uboData = this->cameraUboData;
            
            // Copy over the view matrix
            memcpy( uboData.viewMatrix, glm::value_ptr( vMatrix ),
                    sizeof( float ) * 16 );

            // Copy over the PV matrix
            memcpy( uboData.pvMatrix, glm::value_ptr( this->pvMatrix ),
                    sizeof( float ) * 16 );
            
            // Copy over the camera orientation
            memcpy( uboData.cameraOrientation,
                    glm::value_ptr( glm::vec4( this->forwardVector, 0.0 ) ),
                    sizeof( float ) * 4 );
            
            // Finally, copy over the camera position
            memcpy( uboData.cameraPosition,
                    glm::value_ptr( glm::vec4( this->cameraPosition, 1.0 ) ),
                    sizeof(float) * 4);
        }

        return this->cameraUbo;
    }

    const std::shared_ptr< CG_Data::UBO > Camera::getCameraUbo() const{
        return this->cameraUbo;
    }

    const CameraUboData * Camera::getCameraUboData() const {
        return & this->cameraUboData;
    }

    const glm::mat4 & Camera::getViewMatrix() {
        if (updateViewMatrix){
            generateViewMatrix();
        }
        return this->viewMatrix;
    }

    const glm::mat4 & Camera::getProjectionMatrix() const {
        return this->projectionMatrix;
    }

    const glm::vec3 & Camera::getCameraPosition() const {
        return this->cameraPosition;
    }

    const glm::vec3 & Camera::getForwardVector() const {
        return this->forwardVector;
    }

    const glm::vec3 & Camera::getUpVector() const {
        return this->upVector;
    }

    const glm::vec3 & Camera::getRightVector() const {
        return this->rightVector;
    }

    const glm::quat & Camera::getOrientation() const {
        return this->orientation;
    }



    void Camera::generateViewMatrix() {
        orientation = glm::normalize( orientation );

        // Get rotational component from quaternion oerientation
        glm::mat4 R = glm::toMat4( orientation );
        
        // Get translation component
        glm::mat4  T = glm::translate( glm::mat4( 1.0 ), 
                                       glm::vec3( -cameraPosition ) );

        // Reset orientation vectors based on rotation matrix
        this->forwardVector = glm::vec3( R * glm::vec4( 0, 0, -1, 0 ) );
        this->upVector = glm::vec3( R * glm::vec4( 0, 1, 0, 0 ) );
        this->rightVector =	glm::vec3( R * glm::vec4( 1, 0, 0, 0 ) );

        // Construct view matrix from translation and rotation components
        this->viewMatrix =  glm::inverse( R ) * T;
        this->updateViewMatrix = false;

        // Generate PV matrix
        this->pvMatrix = this->projectionMatrix * this->viewMatrix;
    }

    float Camera::getFarPlane(){
        return this->farPlane;
    }
    float Camera::setFarPlane( float _farPlane ){
        return this->farPlane = _farPlane;
    }
    float Camera::getNearPlane(){
        return this->nearPlane;
    }
    float Camera::setNearPlane( float _nearPlane ){
        return this->nearPlane = _nearPlane;
    }
    float Camera::getFov(){
        return this->fov;
    }
    float Camera::setFov( float _fov ){
        return this->fov = _fov;
    }
    float Camera::getAspectRatio(){
        return this->aspectRatio;
    }
    float Camera::setAspectRatio( float _aspectRatio ){
        return this->aspectRatio = _aspectRatio;
    }

}
