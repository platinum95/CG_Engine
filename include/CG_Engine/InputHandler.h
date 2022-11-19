#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "Common.h"

#include <vector>

namespace GL_Engine {
class InputHandler {
public:
    InputHandler();
    ~InputHandler();
};

class KeyHandler {
public:
    enum ClickType {
        GLFW_CLICK, GLFW_HOLD, GLFW_CLICK_HOLD
    };
    enum EventType {
        KEY_INCREMENT, KEY_FUNCTION, KEY_IGNORE
    };
    struct KeyType {
        void(*EventHandler)(GLuint, void*);
        float *Value;
        void *FunctionParameter;
        float ByAmount;
        GLuint Key;
        bool IsKeyDown{ false };
        ClickType clickType;
        EventType eventType;
    };
    KeyHandler();
    ~KeyHandler();

    //Adds an event to increment a value by a certain amount
    void AddKeyEvent( GLuint _Key, ClickType _ClickType, EventType _EventType, float *_Value, float _DeltaV );

    //Adds an event to call a user-defined event handler
    void AddKeyEvent( GLuint _key, ClickType _ClickType, EventType _EventType, void(*_EventHandler)(GLuint, void*), void *_EventParameter );

    void Update( GLFWwindow *window );

private:
    std::vector<KeyType*> KeyList;
};

class MouseHandler {
    MouseHandler();
    ~MouseHandler();
};

} // namespace GL_Engine 
#endif // INPUT_HANDLER_H