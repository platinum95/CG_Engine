R"(
#version 330 core

layout (location = 0) out vec4 frag_colour;
layout (location = 1) out vec4 BrightColor;

in float PassTime;
in vec3 PassColour;
in float PassOpacity;

void main(){
	frag_colour = vec4(PassColour, PassOpacity);
	float brightness = dot(frag_colour.rgb, vec3(0.01, 0.035, 0.0722));
	BrightColor = vec4(frag_colour.rgb, 1.0);
    if(brightness < 1.0)
        BrightColor = vec4(frag_colour.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

)"
