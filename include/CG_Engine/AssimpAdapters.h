#ifndef ASSIMP_ADAPTERS_H
#define ASSIMP_ADAPTERS_H

#include "CG_Assert.h"
#include "CG_Data.h"
#include "File_IO.h"
#include "glad.h"
#include "MaterialData.h"

#include <assimp/material.h>

#include <filesystem>
#include <iterator>
#include <variant>
#include <vector>


namespace GL_Engine {

class AssimpAdapters {
public:
    // TODO - something more generic
    static PhongMaterialBufferData CreateMaterialBufferData( aiMaterial *material ) {
        PhongMaterialBufferData data;
        aiShadingMode shadingMode;

        cg_assert( material->Get( AI_MATKEY_SHADING_MODEL, shadingMode ) == aiReturn::aiReturn_SUCCESS );
        cg_assert( shadingMode == aiShadingMode::aiShadingMode_Phong );

        cg_assert( material->Get( AI_MATKEY_SHININESS, data.a ) == aiReturn::aiReturn_SUCCESS ); // Phong 'a'
        cg_assert( material->Get( AI_MATKEY_SHININESS_STRENGTH, data.ks ) == aiReturn::aiReturn_SUCCESS ); // Probably 'ks' -- confirm?
        //cg_assert( material->Get( AI_MATKEY_SHININESS, data.a ) == aiReturn::aiReturn_SUCCESS );
        aiColor3D diffuse;
        cg_assert( material->Get( AI_MATKEY_COLOR_DIFFUSE, diffuse ) == aiReturn::aiReturn_SUCCESS );
        data.diffuseColour = glm::vec3( diffuse.r, diffuse.g, diffuse.b );
        
        // No diffuse, no ambient....
        data.kd = 0.5f;
        data.ka = 0.1f;

        return data;
    }

    static Material CreateMaterial( aiMaterial *aiMaterial ) {
        Material material;
        aiShadingMode shadingMode;

        aiMaterial->Get( AI_MATKEY_SHADING_MODEL, shadingMode );
        if ( shadingMode == aiShadingMode::aiShadingMode_Phong ) {
            PhongMaterialBufferData phongData;
            cg_assert( aiMaterial->Get( AI_MATKEY_SHININESS, phongData.a ) == aiReturn::aiReturn_SUCCESS ); // Phong 'a'
            cg_assert( aiMaterial->Get( AI_MATKEY_SHININESS_STRENGTH, phongData.ks ) == aiReturn::aiReturn_SUCCESS ); // Probably 'ks' -- confirm?
            //cg_assert( material->Get( AI_MATKEY_SHININESS, data.a ) == aiReturn::aiReturn_SUCCESS );
            aiColor3D diffuse;
            cg_assert( aiMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, diffuse ) == aiReturn::aiReturn_SUCCESS );
            phongData.diffuseColour = glm::vec3( diffuse.r, diffuse.g, diffuse.b );

            // No diffuse, no ambient....
            phongData.kd = 1.0f;
            phongData.ka = 0.5f;

            material.AddComponent( MaterialComponent::Phong, std::move( phongData ) );
        }

        if ( auto diffuseTexCnt = aiMaterial->GetTextureCount( aiTextureType_DIFFUSE ); diffuseTexCnt > 0 ) {
            // TODO - for now just take the first texture.
            aiString texPathAiStr;
            aiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &texPathAiStr );

            // Convert separator to x-platform.
            // TODO - move to utility
            std::string texPathStr = std::string( texPathAiStr.C_Str() );
            std::replace( texPathStr.begin(), texPathStr.end(), '\\', '/' );

            auto texRelPath = std::filesystem::path( texPathStr );
            cg_assert( !texRelPath.empty() );

            // TODO - Inventory system. For now, take path as relative to CWD.
            // TODO - Texture system for caching textures.

            int width, height, nChannels;
            void *data = File_IO::LoadImageFile( std::filesystem::path( "./Assets/Scenes/Ballet" ) / texPathStr, width, height, nChannels, true );
            GLint format = GL_RGB;
            switch ( nChannels ) {
            case 0:
                break;
            case 1:
                format = GL_RED;
                break;
            case 2:
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
            }
            auto paramFunc = [] () {
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            };

            // TODO - should be new texture class
            std::shared_ptr<CG_Data::Texture> newTexture =
                std::make_shared<CG_Data::Texture>( data, width, height, GL_TEXTURE0, format, std::move( paramFunc ), GL_TEXTURE_2D );
            free( data );
            //File_IO::FreeImageData(data);

            material.AddComponent( MaterialComponent::DiffuseTexture, std::move( newTexture ) );
        }


        return material;
    }
};

template<typename T>
class AssimpContainer {
public:
    struct Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;

        Iterator( pointer *ptr ) : m_ptr( ptr ) {}

        reference operator*() const { return **m_ptr; }
        pointer operator->() { return *m_ptr; }

        Iterator &operator++() {
            ++m_ptr;
            return *this;
        }

        Iterator operator++( int ) {
            Iterator tmp = *this;
            ++( *this );
            return tmp;
        }

        friend bool operator== ( const Iterator &a, const Iterator &b ) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= ( const Iterator &a, const Iterator &b ) { return a.m_ptr != b.m_ptr; };

    private:
        pointer *m_ptr;
    };

public:

    AssimpContainer( T **data, size_t numElements )
        : m_data( data )
        , m_count( numElements )
    {}

    Iterator begin() {
        return Iterator( &m_data[ 0 ] );
    }
    Iterator end() {
        return Iterator( &m_data[ m_count ] );
    }

    const Iterator cbegin() {
        return Iterator( &m_data[ 0 ] );
    }
    const Iterator cend() {
        return Iterator( &m_data[ m_count ] );
    }

private:
    T **m_data;
    size_t m_count;
};

template<typename T>
std::vector<T> CopyData( char *data, size_t dataLen ) {
    const size_t numEntries = dataLen / sizeof( T );

    auto dataVec = std::vector<T>( numEntries );
    std::memcpy( dataVec.data(), data, dataLen );
    return dataVec;
}

struct AssimpMaterial {
    struct Property {
        std::string key;
        int32_t textureSemantic;
        int32_t textureIndex;
        std::variant<std::vector<float>, std::vector<double>, std::string, std::vector<int32_t>, std::vector<uint8_t>, aiShadingMode> data;

        Property( aiMaterialProperty *matProperty ) {
            key = std::string( matProperty->mKey.C_Str() );
            textureSemantic = matProperty->mSemantic;
            textureIndex = matProperty->mIndex;

            switch ( matProperty->mType ) {
            case aiPTI_Float:
            {
                data = CopyData<float>( matProperty->mData, matProperty->mDataLength );
                break;
            }
            case aiPTI_Double:
            {
                data = CopyData<double>( matProperty->mData, matProperty->mDataLength );
                break;
            }
            case aiPTI_String:
            {
                data = std::string( matProperty->mData + 4 );
                break;
            }
            case aiPTI_Integer:
            {
                data = CopyData<int32_t>( matProperty->mData, matProperty->mDataLength );
                break;
            }
            case aiPTI_Buffer:
            {
                if ( key == "$mat.shadingm" ) {
                    cg_assert( matProperty->mDataLength == sizeof( aiShadingMode ) );
                    data = *reinterpret_cast<aiShadingMode*>( matProperty->mData );
                }
                else {
                    data = CopyData<uint8_t>( matProperty->mData, matProperty->mDataLength );
                }
                break;
            }
            };
        }
    };

    AssimpMaterial() = default;

    AssimpMaterial( aiMaterial *material ) {
        auto properties = AssimpContainer( material->mProperties, material->mNumProperties );
        m_properties.reserve( material->mNumProperties );

        for ( auto &property : properties ) {
            m_properties.push_back( &property );
        }
    }

    std::vector<Property> m_properties;
    aiShadingMode m_shadingMode;
};

}; // namespace CG_Engine
#endif //ASSIMP_ADAPTERS_H