R"(
#version 330 core

layout (std140) uniform CameraProjectionData
{ 
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
  mat4 PV_Matrix;
  vec4 CameraPosition;
  vec4 CameraOrientation;
  vec4 ClippingPlane;
};

in vec3 Velocity;
in float Time;
in float Size;
in vec3 Colour;
in float Opacity;
in float Lifetime;

uniform float CurrentTime;
uniform vec3 EmitterPosition;
uniform vec3 EmitterDirection;
uniform mat4 model;
uniform vec3 Gravity;

out float PassTime;
out vec3 PassColour;
out float PassOpacity;


void main(){
	PassColour = Colour;
	float elapsed_time = CurrentTime - Time;
	elapsed_time = mod (elapsed_time, 5.0);
	PassOpacity = 1.0;
	float lifetime_decay = Lifetime / 3.5;
	if(elapsed_time > lifetime_decay){
		float decay_time = Lifetime - elapsed_time;
		PassOpacity = max(decay_time/lifetime_decay, 0);
	}
	PassTime = elapsed_time;

	vec3 ePos = vec3(0, 0, 0);
	vec3 Position = Velocity * elapsed_time;
	vec3 GravityOffset = Gravity * elapsed_time * elapsed_time;
	ePos = ePos + Position + GravityOffset;

	vec4 WorldPosition = model * vec4(ePos, 1.0);// model * vec4(ePos, 1.0);
	gl_ClipDistance[0] = dot(WorldPosition, ClippingPlane);

	gl_Position = PV_Matrix * WorldPosition;
	gl_PointSize = Size;

}
)"
