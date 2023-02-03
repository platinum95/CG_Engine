#ifndef TERRAIN_H
#define TERRAIN_H

#include "CG_Data.h"
#include "Entity.h"
#include "Renderer.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace GL_Engine {

namespace CG_Data {
class VBO;
class VAO;
}

struct ChunkData {
    std::vector<float> Heights;
    std::vector<glm::vec3> Normals, Tangents, Bitangents;
};
struct MeshData {
    std::vector<unsigned int> Indices;
    std::vector<glm::vec2> Mesh, TexCoords;
    unsigned int MeshSize, DivisionCount;
};
struct MeshBaseVBOs {
    std::shared_ptr<CG_Data::VBO> MeshVBO, IndexVBO, TexVBO;
};
class TerrainGenerator {
public:
    static MeshData CreateMesh( uint32_t MeshSize, uint32_t Divisions );
    static std::vector<float> GenerateHeights( int MeshSize, int DivisionCount, int GridX, int GridZ );
    static std::vector<glm::vec3> GetSmoothNormals( const MeshData &meshData, const std::vector<float> Heights );
    static ChunkData GenerateChunk( int GridX, int GridZ, const MeshData &meshData );
    static float getHeight( int x, int z );

private:
    static float getSmoothNoise( int x, int z );
    static float getNoise( int x, int z );
    static float interpolate( float a, float b, float blend );
    static float getInterpolatedNoise( float x, float z );

};
class TerrainChunk : public CG_Data::VAO {
public:
    TerrainChunk( const MeshBaseVBOs &baseVBOs, const MeshData &meshData, int GridX, int GridZ );
    glm::vec2 WorldPos;
    glm::vec2 WorldGridPosition;
    glm::mat4 Translation;
private:

};

class Terrain {
private:
    struct TerrainPack {
        std::vector<std::shared_ptr<TerrainChunk>> TerrainChunks;
        unsigned int translationUniformLocation;
        Entity terrainEntity;
    };
public:
    Terrain( uint32_t _MeshSize, uint32_t _DivisionCount );
    std::shared_ptr<TerrainChunk> GenerateChunk( int xGrid, int zGrid );

    std::unique_ptr<RenderPass> GetRenderPass( Shader *_GroundShader, bool isProj = false );
    TerrainPack tPack;
    std::shared_ptr<CG_Data::VBO> MeshVBO, IndexVBO, TexcoordVBO;
    MeshData meshData;
    MeshBaseVBOs baseVBOs;
    uint32_t MeshSize, DivisionCount;
private:
    static void TerrainRenderer( RenderPass &Pass, void *_Data );
    static void TerrainProjRenderer( RenderPass &rPass, void *_data );
};

} //namespace GL_Engine
#endif // TERRAIN_H