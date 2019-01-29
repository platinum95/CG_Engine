#include "Camera.h"

#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <iostream>

namespace GL_Engine {

	Camera::Camera() {
		Orientation = glm::quatLookAt(glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		GenerateViewMatrix();
	}


	Camera::~Camera() {
	}

	const glm::mat4 &Camera::SetProjectionMatrix(float _NearPlane, float _FarPlane, float _FOV, float _AspectRatio) {
		float fov_radians = glm::radians(_FOV);
		float range = tan(fov_radians / 2.0f) * _NearPlane;
		float Sx = _NearPlane / (range * _AspectRatio);
		float Sy = _NearPlane / range;
		float Sz = -(_FarPlane + _NearPlane) / (_FarPlane - _NearPlane);
		float Pz = -(2 * _FarPlane * _NearPlane) / (_FarPlane * _NearPlane);
		float ConstructMatrix[] = {
			Sx, 0, 0, 0,
			0, Sy, 0, 0,
			0, 0, Sz, -1,
			0, 0, Pz, 0
		};
		this->ProjectionMatrix = glm::perspective(fov_radians, _AspectRatio, _NearPlane, _FarPlane);// glm::make_mat4(ConstructMatrix);
		return this->ProjectionMatrix;
	}
	void Camera::SetProjectionMatrix(glm::mat4 &_Projection) {
		this->ProjectionMatrix = _Projection;
	}

	void Camera::ReflectCamera() {
		static Camera backup;
		static bool reflected = false;
		if (reflected == false) {
			backup = *this;
			this->CameraPosition.y *= -1.0;
			//float planeAngle = asin(ForwardVector.y / glm::length(ForwardVector));
			//volatile glm::quat test = glm::quat(ForwardVector);

			//planeAngle *= -1.0;
			//std::cout << ForwardVector.x << " " << ForwardVector.y << " " << ForwardVector.z << std::endl;
			//this->PitchBy(-2.0f * glm::degrees(planeAngle));
			this->RotationEuler.x = -this->RotationEuler.x;
			reflected = true;
			this->GenerateViewMatrix();
		}
		else {
			reflected = false;
			*this = backup;
		}
	}
	const glm::vec4 &Camera::SetCameraPosition(const glm::vec4 &_Position) {
		this->CameraPosition = _Position;
		this->ViewMatrix[3] = this->CameraPosition;
        this->UpdateViewMatrix = true;
		return this->CameraPosition;
	}
	const glm::vec4 &Camera::TranslateCamera(const glm::vec4 &_Translation) {
		this->CameraPosition += glm::vec4(this->ForwardVector, 0.0) * _Translation.z;
		this->CameraPosition += glm::vec4(this->RightVector, 0.0) * _Translation.x;
		this->CameraPosition += glm::vec4(this->UpVector, 0.0) * _Translation.y;
		this->UpdateViewMatrix = true;
		return this->CameraPosition;
	}

	void Camera::PitchBy(float _Pitch) {
		this->RotationEuler.x += _Pitch;
		float Radians = glm::radians(_Pitch);
		glm::quat Versor = glm::angleAxis(Radians, this->RightVector);

		Orientation = Versor * Orientation;
		this->ForwardVector = glm::rotate(Versor, this->ForwardVector);
		this->UpVector = glm::rotate(Versor, this->UpVector);
		this->UpdateViewMatrix = true;
	}
	void Camera::RollBy(float _Roll) {
		this->RotationEuler.z += _Roll;
		float Radians = glm::radians(_Roll);
		glm::quat Versor = glm::angleAxis(Radians, this->ForwardVector);

		Orientation = Versor * Orientation;
		this->RightVector = glm::rotate(Versor, this->RightVector);
		this->UpVector = glm::rotate(Versor, this->UpVector);
		this->UpdateViewMatrix = true;
	}
	void Camera::YawBy(float _Yaw) {
		this->RotationEuler.y += _Yaw;
		float Radians = glm::radians(_Yaw);
		glm::quat Versor = glm::angleAxis(Radians, this->UpVector);

		Orientation = Versor * Orientation;
		this->ForwardVector = glm::rotate(Versor, this->ForwardVector);
		this->RightVector = glm::rotate(Versor, this->RightVector);
		this->UpdateViewMatrix = true;
	}

	const glm::mat4 &Camera::GetViewMatrix() {
		if (UpdateViewMatrix)
			GenerateViewMatrix();
		return this->ViewMatrix;
	}
	const glm::mat4 &Camera::GetProjectionMatrix() const {
		return this->ProjectionMatrix;
	}
	const glm::vec4 &Camera::GetCameraPosition() const {
		return this->CameraPosition;
	}

	const glm::vec3 &Camera::GetForwardVector() const {
		return this->ForwardVector;
	}

	const glm::quat &Camera::GetOrientation() const {
		return this->Orientation;
	}



	void Camera::GenerateViewMatrix() {
		Orientation = glm::normalize( Orientation );

		// Get rotational component from quaternion oerientation
		glm::mat4 R = glm::toMat4( Orientation );
		
		// Get translation component
		glm::mat4  T = glm::translate( glm::mat4( 1.0 ), 
									   glm::vec3( -CameraPosition ) );

		// Reset orientation vectors based on rotation matrix
		this->ForwardVector = glm::vec3( R * glm::vec4( 0, 0, 1, 0 ) );
		this->UpVector = glm::vec3( R * glm::vec4( 0, 1, 0, 0 ) );
		this->RightVector =	glm::vec3( R * glm::vec4( 1, 0, 0, 0 ) );

		// Construct view matrix from translation and rotation components
		this->ViewMatrix = glm::inverse( R ) * T;
		this->UpdateViewMatrix = false;
	}

}
