#ifndef UTILITIES_H
#define UTILITIES_H

#include <assimp/scene.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <iostream>

namespace GL_Engine {

class Utilities {
public:
    Utilities();
    ~Utilities();

    static glm::mat4 AiToGLMMat4( const aiMatrix4x4 &in_mat );
};

} //namespace GL_Engine

static std::ostream &operator<<( std::ostream &os, const glm::vec4 &v );
static std::ostream &operator<<( std::ostream &os, const glm::vec3 &v );
static std::ostream &operator<<( std::ostream &os, const glm::vec2 &v );

#endif //UTILITIES_H
