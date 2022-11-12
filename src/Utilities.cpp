#include "Utilities.h"

namespace GL_Engine {

	glm::mat4 Utilities::AiToGLMMat4(const aiMatrix4x4& in_mat) {
		glm::mat4 tmp;
		tmp[0][0] = in_mat.a1;	tmp[1][0] = in_mat.b1;
		tmp[2][0] = in_mat.c1;	tmp[3][0] = in_mat.d1;
		tmp[0][1] = in_mat.a2;	tmp[1][1] = in_mat.b2;
		tmp[2][1] = in_mat.c2;	tmp[3][1] = in_mat.d2;
		tmp[0][2] = in_mat.a3;	tmp[1][2] = in_mat.b3;
		tmp[2][2] = in_mat.c3;	tmp[3][2] = in_mat.d3;
		tmp[0][3] = in_mat.a4;	tmp[1][3] = in_mat.b4;
		tmp[2][3] = in_mat.c4;	tmp[3][3] = in_mat.d4;
		return glm::transpose(tmp);
	}

}


std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
	os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
	os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
	os << "(" << v.x << ", " << v.y << ")";
	return os;
}