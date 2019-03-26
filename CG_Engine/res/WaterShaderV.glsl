R"===(
#version 330  

layout (std140) uniform cameraProjectionData
{ 
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
  mat4 PV_Matrix;
  vec4 CameraPosition;
  vec4 CameraOrientation;
  vec4 ClippingPlane;
};

layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};


in vec3 vPosition;
in vec2 tCoord;


uniform mat4 modelMatrix;

out vec4 ClipspaceCoord;
out vec2 texCoords;


void main(){
	vec4 vertexPos = vec4( vPosition, 1 );
    vec4 vWorldPos = modelMatrix * vertexPos;

	texCoords = tCoord;//vec2((vPosition.x + 1.0) / 2.0, (vPosition.z + 1.0) / 2.0);

	ClipspaceCoord = PV_Matrix * vWorldPos;

	gl_Position = ClipspaceCoord;
}
)==="