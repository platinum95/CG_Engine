#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <assimp/scene.h>
#include <iostream>

namespace GL_Engine {

	class Utilities{
	public:
		Utilities();
		~Utilities();

		static glm::mat4 AiToGLMMat4(const aiMatrix4x4& in_mat);

	};

}

static std::ostream& operator<<(std::ostream& os, const glm::vec4& v);
static std::ostream& operator<<(std::ostream& os, const glm::vec3& v);
static std::ostream& operator<<(std::ostream& os, const glm::vec2& v);
