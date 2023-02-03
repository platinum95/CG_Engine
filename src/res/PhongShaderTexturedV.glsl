R"(
#version 330
/******************************************************************************
 Basic Phong shader, untextured
******************************************************************************/

layout (std140)
uniform CameraProjectionData {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 pvMatrix;
    vec4 cameraPosition;
    vec4 cameraOrientation;
    vec4 clippingPlane;
};

// TODO - refactor light
layout (std140)
uniform LightData { 
    vec4 LightPosition;
    vec3 LightColour;
    float Brightness;
};

uniform mat4 modelMatrix;

in vec3 i_position;
in vec3 i_normal;
in vec2 i_texCoord;

out vec3 o_normalViewspace;
out vec3 o_vertexPosViewspace;
out vec3 o_lightPosViewspace;
out vec2 o_texCoord;

void main() {
    o_texCoord = i_texCoord;

    // Vertex position in world-space
    vec4 vWorldPosition = modelMatrix * vec4( i_position, 1.0 );

    // Vertex position in view-space
    o_vertexPosViewspace = vec3( viewMatrix * vWorldPosition );

    // Normal in view-space
    o_normalViewspace = normalize( ( viewMatrix * modelMatrix * vec4( i_normal, 0.0 ) ).xyz );

    // Light position in view-space
    o_lightPosViewspace = ( viewMatrix * LightPosition ).xyz;

    // Final vertex position in screen-space
    //gl_Position = pvMatrix * vec4( i_position, 1.0 );
    gl_Position = pvMatrix * vWorldPosition;
}
)"
