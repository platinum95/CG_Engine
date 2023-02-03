#ifndef RENDERER_H
#define RENDERER_H

#include "IRenderable.h"

#include <functional>

namespace GL_Engine {

class TempSimpleShaderRenderNode;
class ModelAttribute;
class Shader;
struct RenderPass;
class SimpleModelEntity;

namespace CG_Data {
class UBO;
}

class Renderer : public IRenderable {
public:
    Renderer();
    ~Renderer();
    void Cleanup();

    std::shared_ptr< RenderPass > AddRenderPass();
    std::shared_ptr< RenderPass > AddRenderPass( Shader *_Shader );
    std::shared_ptr< RenderPass > AddRenderPass( Shader *_Shader, std::function<void( RenderPass &, void * )> _RenderFunction, void *_Data );
    void AddRenderPass( std::shared_ptr<RenderPass> _RPass );
    void AddUBO( CG_Data::UBO *_ubo );

    std::unique_ptr<IRenderable> AssignShader( std::shared_ptr<ModelAttribute> model, std::shared_ptr<CG_Data::UBO> lightUbo, std::shared_ptr<CG_Data::UBO> cameraUbo );
    std::unique_ptr<IRenderable> AssignShader2( std::shared_ptr<SimpleModelEntity> model, std::shared_ptr<CG_Data::UBO> lightUbo, std::shared_ptr<CG_Data::UBO> cameraUbo );

    const std::vector<std::shared_ptr<RenderPass>> &getRenderPasses() const;

    void Render();

    void execute() override { Render(); }

private:
    std::vector<std::shared_ptr<RenderPass>> renderPasses;
    static void DefaultRenderer( RenderPass &, void * );
    std::vector<CG_Data::UBO *> UBO_List;
    std::vector<std::shared_ptr<Shader>> m_shaders;
};

} // namespace GL_Engine
#endif // RENDERER_H