#ifndef CG_ASSERT_Hsadasd
#define CG_ASSERT_Hsadasd

#include "Log.h"
// TODO - other compilers/archictectures/build configs

#define cg_break() __debugbreak();

#define cg_assert( condition ) \
    if ( !(condition) ) [[unlikely]] { \
        Log::Error( "Assertion failed: " #condition ); \
        cg_break() \
    }

#define cg_verify( condition ) cg_assert( condition )


#endif // CG_ASSERT_H