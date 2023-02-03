#ifndef MATERIAL_DATA_H
#define MATERIAL_DATA_H

#include "CG_Data.h"

#include <glm/vec3.hpp>

#include <map>
#include <variant>

namespace GL_Engine {

enum class MaterialComponent {
    DiffuseColour,
    DiffuseTexture,
    NormalTexture,
    Phong
};

struct PhongMaterialBufferData {
    float ks; // Specular reflection
    float kd; // Diffuse reflection
    float ka; // Ambient reflection (may be global?)
    int a; // Shininess
    glm::vec3 diffuseColour;
};

struct MaterialTextureData {

};

using MaterialDataTypes = std::variant<
    PhongMaterialBufferData,
    MaterialTextureData,
    std::shared_ptr<CG_Data::Texture> // TODO - remove
>;


struct Material {
    bool HasComponent( MaterialComponent component ) const {
        return m_components.contains( component );
    }

    template<typename T>
    void AddComponent( MaterialComponent component, T&& data ) {
        cg_assert( !HasComponent( component ) );

        m_components.emplace( component, std::forward<T>( data ) );
    }

    template <MaterialComponent component, typename T>
    const T* GetComponent() const {
        if ( !m_components.contains( component ) ) {
            return nullptr;
        }
        auto &comp = m_components.at( component );

        if constexpr ( component == MaterialComponent::DiffuseTexture || component == MaterialComponent::NormalTexture ) {
            std::shared_ptr<CG_Data::Texture> texPtr = std::get<std::shared_ptr<CG_Data::Texture>>( comp );
            return texPtr.get();
        }
        else if constexpr ( component == MaterialComponent::Phong ) {
            const PhongMaterialBufferData &buffer = std::get<PhongMaterialBufferData>( comp );
            return &buffer;
        }
        else {
            return nullptr; // TODO
        }
    }

private:
    std::map<MaterialComponent, MaterialDataTypes> m_components;
};

} // GL_Engine

#endif // MATERIAL_DATA_H