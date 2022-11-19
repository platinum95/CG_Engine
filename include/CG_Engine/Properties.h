#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "Common.h"

#include <typeinfo>

namespace GL_Engine {
namespace Properties {
struct GLFWproperties {
    uint32_t		width;
    uint32_t		height;
    const char *title;
    GLFWmonitor *monitor;
    GLFWwindow *window;
    GLFWwindow *share;
    bool			fullscreen;
};

struct GLADproperties {
    bool	success;
};

struct ShaderProperties {
    GLuint vertexID, fragmentID, geometryID, tesselationControlID, tesselationEvalID;
    GLuint programID;
    GLint vertexResult, fragmentResult, geometryResult, tesselationControlResult, tesselationEvalResult;
    GLint programResult;

};

struct BufferObjectProperties {
    unsigned int dimension;
    std::size_t unitSize;
    GLboolean transpose;
    GLenum target, usage, glType;
};

struct EntityProperties {
    float *vertices;
    unsigned int *indices;
    unsigned int vertexSize, indexSize;
    float *normals;
};

} // namespace Properties
} // namespace GL_Engine

#endif // PROPERTIES_H
