#pragma once
#include "Properties.h"

namespace GL_Engine{
	class CG_Engine{
	public:
		CG_Engine();
		~CG_Engine();
		static bool CG_CreateWindow(Properties::GLFWproperties *_DisplayProperties);
		static bool CG_StartGlad(Properties::GLADproperties * _GladProperties);
		static uint16_t ViewportWidth, ViewportHeight;
	private:

	};
}

