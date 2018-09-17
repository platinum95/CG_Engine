#include "CG_Engine.h"
#include "Common.h"

namespace GL_Engine{

	CG_Engine::CG_Engine(){

	}


	CG_Engine::~CG_Engine(){
		
	}
	uint16_t CG_Engine::ViewportWidth = 0;
	uint16_t CG_Engine::ViewportHeight = 0;

	bool CG_Engine::CG_CreateWindow(Properties::GLFWproperties *_DisplayProperties){

		if (!glfwInit()){
			return false;
		}
		if (_DisplayProperties->monitor == NULL)
			_DisplayProperties->monitor = _DisplayProperties->fullscreen ? glfwGetPrimaryMonitor() : NULL;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		_DisplayProperties->window = glfwCreateWindow(_DisplayProperties->width, _DisplayProperties->height,
													 _DisplayProperties->title, _DisplayProperties->monitor, 
													 _DisplayProperties->share); // Windowed
		if (!_DisplayProperties->window)
			return false;

		glfwMakeContextCurrent(_DisplayProperties->window);
		ViewportHeight = _DisplayProperties->height;
		ViewportWidth = _DisplayProperties->width;
		return true;
	}

	bool CG_Engine::CG_StartGlad(Properties::GLADproperties * _GladProperties){
		_GladProperties->success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		return _GladProperties->success;
	}

	


}