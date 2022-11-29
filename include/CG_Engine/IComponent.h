#ifndef I_COMPONENT_H
#define I_COMPONENT_H

namespace GL_Engine {
class CG_Instance;

class IComponent {
public:
    virtual void update() {};

protected:
    CG_Instance *getInstance() { return m_instance; }
private:
    friend class CG_Instance;

    // Called before component store is set up; don't reference other components.
    virtual void initialise() {};

private:
    CG_Instance *m_instance;
};

} // namespace GL_Engine 
#endif // I_COMPONENT_H