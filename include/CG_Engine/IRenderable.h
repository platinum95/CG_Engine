#ifndef I_RENDERER_H
#define I_RENDERER_H

#include <functional>
#include <memory>
#include <vector>

namespace GL_Engine {

class IRenderable {
public:
    virtual void execute() = 0;
};

class StaticRenderNode : public IRenderable {
public:
    void addNode( std::shared_ptr<IRenderable> renderable ) {
        m_children.push_back( std::move( renderable ) );
    }

    void execute() override {
        for ( const auto& child : m_children ) {
            child->execute();
        }
    }

private:
    std::vector<std::shared_ptr<IRenderable>> m_children;
};

class DebugRenderNode : public IRenderable {
public:
    DebugRenderNode( std::function<void()> targetFn )
        : m_targetFn( std::move( targetFn ) ) {}

    void execute() {
        m_targetFn();
    }

private:
    std::function<void()> m_targetFn;
};

} // namespace GL_Engine
#endif // I_RENDERER_H