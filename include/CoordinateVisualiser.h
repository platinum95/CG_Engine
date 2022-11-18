#ifndef COORDINATE_VISUALISER_H
#define COORDINATE_VISUALISER_H

#include "CG_Data.h"
#include "Shader.h"

#include <glm/mat3x3.hpp>

namespace GL_Engine {
class CoordinateVisualiser {
public:
    CoordinateVisualiser() = default;
    ~CoordinateVisualiser();
    void initialise();
    void cleanup();
    void render();
    void setCoordinateSystem( glm::mat3 coordinateSystem );

private:
    bool m_initialised{ false };

    Shader m_shader;
    std::unique_ptr<CG_Data::VAO> m_vao; // TODO - defer-init object so they're not ptrs.
    std::unique_ptr<CG_Data::VBO> m_coordinateVbo, m_indexVbo; // TODO - defer-init object so they're not ptrs.
};

} // namespace Gl_Engine
#endif