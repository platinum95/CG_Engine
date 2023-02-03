R"(
#version 410

// Main colour output buffer
layout (location = 0)
out vec4 FragColour;

layout (std140)
uniform LightData { 
    vec4 LightPosition;
    vec3 LightColour;
    float Brightness;
};

// Light should contain:
// i_s (specular intensity)
// i_d (diffuse intensity) -- currently included as Brightness

layout ( std140 )
uniform MaterialData {
    float ks; // Specular reflection constant
    float kd; // Diffuse reflection constant
    float ka; // Ambient reflection constant
    int a; // Shininess
    vec3 diffuseColour;
};

uniform sampler2D diffuseTexture;

in vec3 o_normalViewspace;
in vec3 o_vertexPosViewspace;
in vec3 o_lightPosViewspace;
in vec2 o_texCoord;

void main(){
    // Ambient
    float ambientStrength = 0.1;

    // Diffuse
    vec3 lightDir = vec3( normalize( o_lightPosViewspace.xyz - o_vertexPosViewspace ) );
    float diffuseStrength = max( dot( o_normalViewspace, lightDir ), 0.0 ) * Brightness;

    // Specular
    vec3 CamDir = normalize( vec3( 0, 0, 0 ) - o_vertexPosViewspace );
    vec3 ReflectDir = reflect( -lightDir, o_normalViewspace );
    float specStrength = ks * pow( max( dot( CamDir, ReflectDir ), 0.0 ), a );

    // Output
    float shadingStrength = ambientStrength + diffuseStrength + specStrength;
    vec3 lightShade = shadingStrength * LightColour;
    vec4 diffuseColour = texture( diffuseTexture, o_texCoord );

    vec3 result = lightShade * diffuseColour.xyz;
    FragColour = vec4( result, 1.0 );
    //FragColour = vec4( normalize( LightPosition ) );
}
)"