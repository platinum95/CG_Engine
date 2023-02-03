#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "Entity.h"

#include <string>

namespace GL_Engine {
struct RenderPass;
namespace CG_Data {
class UBO;
class VAO;
}

class ParticleSystem : public Entity
{
public:
    struct ParticleStats {
        uint32_t ParticleCount;
        glm::vec3 Position;
        glm::vec3 BaseDirection;
        float SpeedVariation = 1.0f;
        float DirectionVariation = 15.5f;
        float SizeRange[2] = { 0, 3.0 };
        glm::vec3 ColourRange[2] = { glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 1.0f, 1.0f, 1.0f ) };
        float OpacityRange[2] = { 0.3f, 1.0f };
        float LifetimeRange[2] = { 1.0f, 5.0f };
    };
    ParticleSystem();
    ~ParticleSystem();
    std::unique_ptr<RenderPass> GenerateParticleSystem( const ParticleStats &stats, std::shared_ptr<CG_Data::UBO> _CameraUBO );
    void UpdateTime( const float &_Diff );
    void SetTime( const float &_CurrentTime );
    const float &GetTime() const;

private:
    static void ParticleRenderer( RenderPass &_Pass, void *_Data );
    static const std::string ParticleSystemFSource;
    static const std::string ParticleSystemVSource;
    uint32_t ParticleCount;
    std::shared_ptr<CG_Data::UBO> cameraUBO;
    std::unique_ptr<Shader> ParticleShader;
    std::shared_ptr<CG_Data::VAO> ParticleVAO;
    float Time;
};

} // namespace GL_Engine
#endif // PARTICLE_SYSTEM_H