#include "CoordinateVisualiser.h"

#include <array>

namespace {
constexpr std::string_view cx_vertexShaderGl = R"""(
#version 330 core

const mat4 OrthographicProjection = mat4( 
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, -1, 0,
    0, 0, 0, 1
);

const vec4 indexColours[3] = vec4[3](
    vec4( 1.0, 0.0, 0.0, 1.0 ),
    vec4( 0.0, 1.0, 0.0, 1.0 ),
    vec4( 0.0, 0.0, 1.0, 1.0 )
);

in vec3 v_vertexPosition;
flat out vec4 f_lineColour;

void main() {
    f_lineColour = indexColours[ max( 0, gl_VertexID - 1 ) ];

    gl_Position = OrthographicProjection * vec4( v_vertexPosition, 1.0 );
}

)""";

constexpr std::string_view cx_fragmentShaderGl = R"""(
#version 330 core

flat in vec4 f_lineColour;

out vec4 fragColour;

void main() {
    fragColour = f_lineColour;
}

)""";

constexpr std::array<GLuint, 6> cx_indices = {
    0, 1,
    0, 2,
    0, 3
};

constexpr auto cx_indicesDataLen = static_cast<uint64_t>( cx_indices.size() * sizeof( decltype( cx_indices )::value_type ) );

constexpr std::array<float, 3> cx_origin = {
    0, 0, 0
};

constexpr uint64_t cx_coordSystemDataLen = 3 * 3 * sizeof( float );
constexpr uint64_t cx_originDataLen = static_cast<uint64_t>( cx_origin.size() * sizeof( decltype( cx_origin )::value_type ) );
}

namespace GL_Engine {

//-----------------------------------------------------------------------------
void CoordinateVisualiser::initialise() {
    m_shader.registerShaderStage( std::string( cx_vertexShaderGl ), GL_VERTEX_SHADER );
    m_shader.registerShaderStage( std::string( cx_fragmentShaderGl ), GL_FRAGMENT_SHADER );
    m_shader.registerAttribute( "v_vertexPosition", 0 );
    m_shader.compileShader();
    m_shader.useShader();

    const auto defaultCoordinateSystem = glm::mat3( 1.0f );

    m_vao = std::make_unique<CG_Data::VAO>();
    m_vao->BindVAO();
    m_indexVbo = std::make_unique<CG_Data::VBO>( (void*) cx_indices.data(), cx_indicesDataLen, GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER );

    m_coordinateVbo = std::make_unique<CG_Data::VBO>( nullptr, cx_coordSystemDataLen + cx_originDataLen, GL_DYNAMIC_DRAW, GL_ARRAY_BUFFER );
    GLint buffSize{ 0 };
    m_coordinateVbo->BindVBO();
    glGetBufferParameteriv( GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffSize );
    glBufferSubData( GL_ARRAY_BUFFER, 0, cx_originDataLen, (void*)cx_origin.data() );
    setCoordinateSystem( defaultCoordinateSystem );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
    glEnableVertexAttribArray( 0 );

    m_initialised = true;
}

//-----------------------------------------------------------------------------
CoordinateVisualiser::~CoordinateVisualiser() {
    if ( m_initialised ) {
        cleanup();
    }
}

//-----------------------------------------------------------------------------
void CoordinateVisualiser::cleanup() {

    if ( !m_initialised ) {
        return;
    }

    m_indexVbo->Cleanup();
    m_coordinateVbo->Cleanup();
    m_vao->Cleanup();
    m_shader.cleanup();

    m_initialised = false;
}

//-----------------------------------------------------------------------------
void CoordinateVisualiser::setCoordinateSystem( glm::mat3 coordinateSystem ) {
    m_coordinateVbo->BindVBO();
    glBufferSubData( GL_ARRAY_BUFFER, cx_originDataLen, cx_coordSystemDataLen, (void*)&coordinateSystem );
}

//-----------------------------------------------------------------------------
void CoordinateVisualiser::render() {
    m_shader.useShader();
    m_vao->BindVAO();
    glProvokingVertex( GL_LAST_VERTEX_CONVENTION );
    glDrawElements( GL_LINES, static_cast<GLsizei>( 6 ), GL_UNSIGNED_INT, nullptr );
}

} // namespace GL_Engine