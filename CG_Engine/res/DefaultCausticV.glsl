
R"===(
#version 330
layout (std140) uniform CameraProjectionData{ 
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 pvMatrix;
    vec4 cameraPosition;
    vec4 cameraOrientation;
    vec4 clippingPlane; 
};
in vec3 vPosition;
in vec3 vNormal;
uniform mat4 modelMatrix;
uniform sampler2D receiverTex;
uniform uint surfaceArea;
out vec4 outCol;
out float flux;

#define NUM_RP_ITER 20

// Get world position from depth map
vec3 texPosToWorldPos( vec2 texPos ){
    float depth = texture( receiverTex, texPos ).r;
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePosition = vec4( texPos * 2.0 - 1.0, z, 1.0 );
    vec4 viewSpacePosition = inverse( projectionMatrix ) * clipSpacePosition;
    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = inverse( viewMatrix ) * viewSpacePosition;
    return worldSpacePosition.xyz;
}

vec3 adjRange( vec3 inVec ){
    return ( inVec + 1.0 ) / 2.0;
}

// Estimate the receiver intersection point from refracted ray
vec4 estimateIntersection( vec3 vWorldPos, vec3 refrRay ){
    float dist = 1.0;
    vec3 recPos;
    for( int i = 0; i < NUM_RP_ITER; i++ ){
        vec3 P1 = vWorldPos + dist * refrRay;
        vec4 texPt = pvMatrix * vec4( P1, 1.0 );
        //vec2 tc = ( texPt.xy / texPt.w + vec2( 1.0, 1.0 ) ) / 2.0;
        vec2 tc = 0.5 * texPt.xy / texPt.w + vec2( 0.5, 0.5 );
        // tc.y = 1.0f - tc.y;
        tc = clamp( tc, 0.0, 1.0 );
        recPos = texPosToWorldPos( tc );
        dist = distance( vWorldPos, recPos );
    }
    return vec4( recPos, dist );
}
void main(){
    vec4 vWorldPos = modelMatrix * vec4( vPosition, 1.0 );
    mat3 normalModelMatrix = mat3( transpose(
        inverse( viewMatrix * modelMatrix ) ) );
    vec3 normalWorldspace = normalize( ( normalModelMatrix * vNormal ).xyz ) ;

    vec3 incident = -cameraOrientation.xyz;
    vec3 refrRay;
    float objRefrInd = 1.15;
    if( dot( incident, normalWorldspace ) > 0.0 ){
        // Back face
        normalWorldspace = -normalWorldspace;
        refrRay = normalize( refract( incident,
                                      normalWorldspace,
                                      objRefrInd ) );
    }else{
        // Front face
        refrRay = normalize( refract( incident,
                                      normalWorldspace,
                                      1.0 / objRefrInd ) );
    }
    vec4 vPosDevspace = pvMatrix * vWorldPos;
    vec4 intersectEst = estimateIntersection( vWorldPos.xyz, refrRay );
    gl_Position = pvMatrix *\
        vec4( intersectEst.xyz, 1.0 );
    gl_PointSize = 2;
    gl_Position.zw = vPosDevspace.zw;
    float texArea = 4096 * 4096;
    float dist = intersectEst.w;
    float distAtten = max( 1.0 - ( dist / 10.0 ), 0.01 );
    float surfArea = float( surfaceArea ) / texArea;
    flux = (-dot( normalWorldspace, incident ));
}

)==="