#ifndef CG_LOG_H
#define CG_LOG_H

#include <format>
#include <iostream>

namespace std {
template <class... _Types>
using format_string = std::_Fmt_string<_Types...>;
}

namespace GL_Engine {
class Log {
public:
    enum class Level {
        Debug,
        Verbose,
        Info,
        Warning,
        Error
    };

    template<Level level, typename... Args>
    static inline void LogMsg( const std::format_string<Args...> fmt, Args&&... args ) {
        std::string formatted = std::format( std::move( fmt ), std::forward<Args>( args )... );

        if constexpr ( level == Level::Debug ) {
            formatted = std::format( "Debug: {}", formatted );
        }
        else if constexpr ( level == Level::Verbose ) {
            formatted = std::format( "Verbose: {}", formatted );
        }
        else if constexpr ( level == Level::Info ) {
            formatted = std::format( "Info: {}", formatted );
        }
        else if constexpr ( level == Level::Warning ) {
            formatted = std::format( "Warning: {}", formatted );
        }
        else if constexpr ( level == Level::Error ) {
            formatted = std::format( "Error: {}", formatted );
        }

        std::cout << formatted << std::endl;
    }

    template<typename... Args>
    static inline void Debug( const std::format_string<Args...> fmt, Args&&... args ) {
        LogMsg<Level::Debug>( std::move(fmt), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static inline void Verbose( const std::format_string<Args...> fmt, Args&&... args ) {
        LogMsg<Level::Verbose>( std::move( fmt ), std::forward<Args>( args )... );
    }

    template<typename... Args>
    static inline void Info( const std::format_string<Args...> fmt, Args&&... args ) {
        LogMsg<Level::Info> ( std::move( fmt ), std::forward<Args>( args )... );
    }

    template<typename... Args>
    static inline void Warning( const std::format_string<Args...> fmt, Args&&... args ) {
        LogMsg<Level::Warning>( std::move( fmt ), std::forward<Args>( args )... );
    }

    template<typename... Args>
    static inline void Error( const std::format_string<Args...> fmt, Args&&... args ) {
        LogMsg<Level::Error> ( std::move( fmt ), std::forward<Args>( args )... );
    }
};

}

#endif // CG_LOG_H
