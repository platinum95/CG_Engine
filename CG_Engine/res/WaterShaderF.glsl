R"===(
#version 330

#define WAVE_STRENGTH	0.1
#define WAVE_SCALE		1
#define WAVE_SPEED		0.01

in vec4 ClipspaceCoord;
in mat3 models;
in vec3 Pos_ViewSpace;
in vec4 LightPosition_Viewspace;
in vec3 norms;


uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform float Time;

in vec2 texCoords;
out vec4 FragColour;

layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};

void main(){

	vec2 ndc = ClipspaceCoord.xy/ClipspaceCoord.w;
	ndc = vec2((ndc.x + 1.0) / 2.0, (ndc.y + 1.0) / 2.0);
	float time_normalised = mod(Time, 1.0/WAVE_SPEED);
	
	vec2 dudvVal = texture(dudvMap, (texCoords + time_normalised * WAVE_SPEED) * WAVE_SCALE).rg * 2.0 - 1.0;
	dudvVal = WAVE_STRENGTH * vec2(dudvVal.x * 2.0, dudvVal.y);
	vec2 reflectionCoord = vec2(ndc.x, -ndc.y) + dudvVal;
	vec2 refractionCoord = vec2(ndc.x, ndc.y) + dudvVal;

	reflectionCoord.y = clamp(reflectionCoord.y, -0.999, -0.01);
	reflectionCoord.x = clamp(reflectionCoord.x, 0.01, 0.9);
	refractionCoord = clamp(refractionCoord, 0.001, 0.999);

	vec4 reflectionColour = texture(reflectionTexture, reflectionCoord);
	vec4 refractionColour = texture(refractionTexture, refractionCoord);

	FragColour = mix(reflectionColour, refractionColour, 0.5);
}
)==="