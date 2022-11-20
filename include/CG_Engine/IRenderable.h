#ifndef I_RENDERER_H
#define I_RENDERER_H

#include <memory>
#include <vector>

namespace GL_Engine {

class IRenderable {
public:
    virtual void execute() = 0;
};

class StaticRenderNode : public IRenderable {
public:
    void addNode( std::shared_ptr<IRenderable> &&renderable ) {
        m_children.push_back( renderable );
    }

    void execute() override {
        for ( const auto& child : m_children ) {
            child->execute();
        }
    }

private:
    std::vector<std::shared_ptr<IRenderable>> m_children;
};

} // namespace GL_Engine
#endif // I_RENDERER_H