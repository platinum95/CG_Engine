#ifndef SCOPED_TOKEN_H
#define SCOPED_TOKEN_H

#include <functional>

#define UsingScopedToken( getterFn ) if ( auto _scopedToken = getterFn; true )

template<std::equality_comparable C, auto UnbindFn>
    requires std::is_invocable_v<decltype(UnbindFn), C&>
class [[nodiscard]] ScopedToken {
public:
    ScopedToken() = default;
    ScopedToken( ScopedToken &&token ) = default;
    ScopedToken& operator=( ScopedToken &&rhs ) noexcept {
        std::move( this )->~ScopedToken();
        m_id = std::exchange( rhs.m_id, C() );
        return *this;
    };

    ~ScopedToken() {
        if ( isValid() ) {
            UnbindFn( m_id );
            m_id = C();
        }
    }

    bool isValid() { return m_id != C(); };

    void release() && {};

    ScopedToken( const ScopedToken & ) = delete;
    ScopedToken &operator=( const ScopedToken & ) = delete;

    ScopedToken( C &&id ) : m_id( std::forward<C>( id ) ) {};

private:
    C m_id;
};

#endif