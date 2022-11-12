
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

// Number of Newton-Raphson iterations
#define NUM_RP_ITER 20

// Get world position from receiver depth map
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


// Estimate the receiver intersection point from refracted ray
vec4 estimateIntersection( vec3 vWorldPos, vec3 refrRay ){
    float dist = 1.0;
    vec3 recPos;
    // Taken from the caustics paper (and adjusted slightly).
    // Uses the Newton Raphson root finding algorithm
    for( int i = 0; i < NUM_RP_ITER; i++ ){
        vec3 P1 = vWorldPos + dist * refrRay;
        vec4 texPt = pvMatrix * vec4( P1, 1.0 );
        vec2 tc = 0.5 * texPt.xy / texPt.w + vec2( 0.5, 0.5 );
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

    vec3 incident = cameraOrientation.xyz;
    vec3 refrRay;
    float objRefrInd = 1.04;
    if( dot( incident, normalWorldspace ) > 0.0 ){
        // Back face
        normalWorldspace = -normalWorldspace;
        refrRay = normalize( refract( incident,
                                      normalWorldspace,
                                      1.0/objRefrInd ) );
    }else{
        // Front face
        refrRay = normalize( refract( incident,
                                      normalWorldspace,
                                      objRefrInd ) );
    }
    vec4 vPosDevspace = pvMatrix * vWorldPos;
    // Find the worldspace point at which we intersect
    vec4 intersectEst = estimateIntersection( vWorldPos.xyz, refrRay );
    // Convert this point to our NDC
    gl_Position = pvMatrix *\
        vec4( intersectEst.xyz, 1.0 );
    float dist = distance( vWorldPos.xyz, intersectEst.xyz );
    // Scale size based on distance to receiver
    gl_PointSize = 5 * max( 2, dist );
    gl_Position.zw = vPosDevspace.zw;

    // Can make estimator on this to scale the intensity
    flux = 1.0f;
}

)==="