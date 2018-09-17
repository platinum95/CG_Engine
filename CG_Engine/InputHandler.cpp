#include "InputHandler.h"

using namespace GL_Engine;

InputHandler::InputHandler() {
}


InputHandler::~InputHandler() {
}

MouseHandler::~MouseHandler() {

}



KeyHandler::KeyHandler() {
	this->KeyList = std::vector<KeyType*>();
}
KeyHandler::~KeyHandler() {
	for (auto key : KeyList)
		delete key;
	KeyList.clear();
}


//Adds an event to increment a value by a certain amount
void KeyHandler::AddKeyEvent(GLuint _Key, ClickType _ClickType, EventType _EventType, float* _Value, float _DeltaV) {
	KeyType *type = new KeyType;

	type->Key = _Key;
	type->clickType = _ClickType;
	type->eventType = _EventType;
	type->Value = _Value;
	type->ByAmount = _DeltaV;

	this->KeyList.push_back(type);

}

//Adds an event to call a user-defined event handler
void KeyHandler::AddKeyEvent(GLuint _Key, ClickType _ClickType, EventType _EventType, void(*_EventHandler)(GLuint, void*), void* _EventParameter) {
	KeyType *type = new KeyType;

	type->Key = _Key;
	type->clickType = _ClickType;
	type->eventType = _EventType;
	type->EventHandler = _EventHandler;
	type->FunctionParameter = _EventParameter;

	this->KeyList.push_back(type);

}

void GL_Engine::KeyHandler::Update(GLFWwindow *window) {
	for (auto *keyType : this->KeyList) {
		if (keyType->eventType == KEY_INCREMENT) {
			if (keyType->clickType == GLFW_CLICK) {

				if (!keyType->IsKeyDown && glfwGetKey(window, keyType->Key) == GLFW_PRESS) {
					keyType->IsKeyDown = true;
					*(keyType->Value) -= keyType->ByAmount;
				}
				if (keyType->IsKeyDown && glfwGetKey(window, keyType->Key) == GLFW_RELEASE) {
					keyType->IsKeyDown = false;
				}
			}
			else {
				if (glfwGetKey(window, keyType->Key) == keyType->clickType) {
					*(keyType->Value) += keyType->ByAmount;
				}
			}
		}
		else if (keyType->eventType == KEY_FUNCTION) {
			if (keyType->clickType == GLFW_CLICK) {
				if (!keyType->IsKeyDown && glfwGetKey(window, keyType->Key) == GLFW_PRESS) {
					keyType->IsKeyDown = true;
					keyType->EventHandler(keyType->Key, keyType->FunctionParameter);
				}
				if (keyType->IsKeyDown && glfwGetKey(window, keyType->Key) == GLFW_RELEASE) {
					keyType->IsKeyDown = false;
				}
			}
			else {
				if (glfwGetKey(window, keyType->Key) == keyType->clickType) {
					keyType->EventHandler(keyType->Key, keyType->FunctionParameter);
				}
			}
		}
		else if (keyType->clickType == KEY_IGNORE) {
			if (!keyType->IsKeyDown && glfwGetKey(window, keyType->Key) == GLFW_PRESS)
				keyType->IsKeyDown = true;
			if (keyType->IsKeyDown && glfwGetKey(window, keyType->Key) == GLFW_RELEASE) {
				keyType->IsKeyDown = false;
			}

		}
	}
}
