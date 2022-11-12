#pragma once
#ifndef WATER_H
#define WATER_H
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "CgTime.h"

namespace GL_Engine{
class Water : public Entity {
public:
    Water( uint16_t _fbWidth, uint16_t _fbHeight, 
           std::filesystem::path _dudvPath,
           std::vector< std::shared_ptr< Renderer > > _renderers,
           std::shared_ptr< Camera > _sceneCamera,
           std::shared_ptr< CG_Data::VAO > _waterData );

    void cleanup();
    ~Water();
    
    std::shared_ptr< RenderPass > getRenderPass();
    std::vector< std::shared_ptr< Renderer > > renderers;
    std::shared_ptr< CG_Data::Texture > refrTex, reflTex;
    std::shared_ptr< CG_Data::Texture > dudvTexture;

private:
    std::unique_ptr< Shader > waterShader;
    std::unique_ptr< CG_Data::FBO > waterFbo;
    //std::shared_ptr< CG_Data::Texture > dudvTexture;
    
    std::shared_ptr< RenderPass > waterRenderPass;
    std::shared_ptr< Camera > sceneCamera;
    //std::shared_ptr< CG_Data::Texture > refrTex, reflTex;
    std::shared_ptr< CG_Data::VAO > waterVao;

    static void defaultWaterRenderer( RenderPass & _rPass, void * _data );
    static std::vector< Water * > waterObjects;
    Stopwatch< std::chrono::microseconds > waterStopwatch;
    float time;

};
}
#endif