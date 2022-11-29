#ifndef CG_ASSERT_Hsadasd
#define CG_ASSERT_Hsadasd

#include "Log.h"
// TODO - other compilers/archictectures/build configs

#define cg_break() __debugbreak();

#define cg_assertMsg( condition, msg, ... ) \
    if ( !(condition) ) [[unlikely]] { \
        GL_Engine::Log::Error( "Assertion failed: " msg, __VA_ARGS__ ); \
        cg_break() \
    }

#define cg_assert( condition ) cg_assertMsg( condition, "Assertion failed: " #condition )

#define cg_verify( condition ) cg_assert( condition )


#endif // CG_ASSERT_H