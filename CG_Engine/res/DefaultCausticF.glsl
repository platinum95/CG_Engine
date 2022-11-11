R"===(

#version 330
out vec4 fOut;
in vec4 outCol;
uniform sampler2D splatterTex;
in float flux;
void main(){
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
    //if( dot( circCoord, circCoord ) > 1.0 ){
    //    discard;
    //}
    vec4 texVal = texture( splatterTex, gl_PointCoord );
    // Hardcoded scale of intensity
    texVal.a *= 0.1 * flux;
    fOut = texVal;//outCol;//vec4(1,1,1,1);
}

)==="