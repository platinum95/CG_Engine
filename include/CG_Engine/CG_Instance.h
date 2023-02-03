#ifndef CG_INSTANCE_H
#define CG_INSTANCE_H

#include "IComponent.h"

#include <algorithm>
#include <type_traits>

namespace GL_Engine {

class CG_Instance {
public:
    virtual void initialise() {}

    template<typename C>
        requires std::is_base_of_v<IComponent, C>
    C *getComponent() {
        auto found = std::ranges::find_if( m_components, [] ( const auto& component ) { return typeid( *component.get() ) == typeid( C ); } );
        cg_assertMsg( found != m_components.end(), "CG_Instance::getComponent -- Failed to find component of type {}", typeid( C ).name() );
        return found == m_components.end() ? nullptr : static_cast<C*>( found->get() );
    }

    void tick() {
        for ( auto &component : m_components ) {
            component->update();
        }
        update();
    }

protected:
    virtual void update() {}
    void registerComponent( std::unique_ptr<IComponent> component );

    template <typename C, typename... Args>
        requires std::is_base_of_v<IComponent, C>
    C* registerComponent( Args... args ) {
        auto component = std::make_unique<C>( args... );
        auto ptr = component.get();
        component->m_instance = this;
        m_components.push_back( std::move( component ) );
        return ptr;
    }

    void lockComponents() {
        for ( auto &component : m_components ) {
            component->initialise();
        }
    }
private:
    std::vector<std::unique_ptr<IComponent>> m_components;
};

} //namespace GL_Engine
#endif // CG_INSTANCE_H