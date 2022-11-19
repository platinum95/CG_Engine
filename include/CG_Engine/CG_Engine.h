#ifndef CG_ENGINE_H
#define CG_ENGINE_H

#include "Properties.h"

namespace GL_Engine {

class CG_Engine {
public:
    CG_Engine();
    ~CG_Engine();
    static bool CG_CreateWindow( Properties::GLFWproperties *_DisplayProperties );
    static bool CG_StartGlad( Properties::GLADproperties *_GladProperties );
    static uint32_t ViewportWidth, ViewportHeight;
};

} // namespace GL_Engine

#endif // CG_ENGINE_H
