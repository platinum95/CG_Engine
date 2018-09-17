#include "Cubemap.h"

namespace GL_Engine {
	const float Cubemap::vertices[24] = {
		-1, 1, -1,		//0 T1
		-1, -1, -1,		//1 T1
		1, -1, -1,		//2 T2
		1, 1, -1,			//3 T1


		-1, 1, 1,			//4 T1
		-1, -1, 1,		//5 T1
		1, -1, 1,			//6 T2
		1, 1,  1			//7 T1

	};

	//GL_QUAD Clockwise
	const unsigned int Cubemap::indices[36] = {
		0, 1, 3, 1, 2, 3,	//F1 Front
		2, 3, 7, 2, 7, 6,	//F2 Right
		4, 5, 7, 5, 7, 6,	//F3 Back
		0, 1, 4, 1, 4, 5,	//F4 Left
		1, 2, 6, 1, 6, 5,	//F5 Bottom
		0, 7, 4, 0, 7, 3	//F6 Top
	};


	Cubemap::Cubemap(const std::vector<std::string> &_TextureFiles, Shader *_CubemapShader, Renderer *_Renderer) {
		GenerateCubemap(_TextureFiles);
		SetupArrayObjects();
		SetupRenderPass(_Renderer);
		CubemapShader = _CubemapShader;
		CubeRenderPass->shader = this->CubemapShader;
		CreateShader(_CubemapShader);
	}
	Cubemap::~Cubemap() {

	}

	void Cubemap::GenerateCubemap(const std::vector<std::string> &_TextureFiles) {
		MapTexture = std::make_shared<CG_Data::Texture>(GL_TEXTURE0, GL_TEXTURE_CUBE_MAP);
		MapTexture->Bind();

		GLenum type = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		for (auto str : _TextureFiles) {
			int width, height, nChannels;
			void *data = File_IO::LoadImageFile(str, width, height, nChannels, false);
			glTexImage2D(type++, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			File_IO::FreeImageData(data);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	const RenderPass *Cubemap::GetRenderPass() const { 
		return this->CubeRenderPass;
	}

	void Cubemap::CreateShader(Shader *_Shader) {
		
	}
	void Cubemap::SetupRenderPass(Renderer *_Renderer) {
		this->CubeRenderPass = _Renderer->AddRenderPass(this->CubemapShader, std::function<void(RenderPass &, void*)>(Cubemap::CubemapRenderer), nullptr);
		CubeRenderPass->SetDrawFunction([]() {glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); });
		CubeRenderPass->BatchVao = CubemapVAO;
		CubeRenderPass->Textures.push_back(MapTexture);
	}
	void Cubemap::SetupArrayObjects() {
		CubemapVAO = std::make_shared<CG_Data::VAO>();
		CubemapVAO->BindVAO();
		std::unique_ptr<CG_Data::VBO> indexVBO = std::make_unique<CG_Data::VBO>((void*)&indices[0], 36 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
		CubemapVAO->AddVBO(std::move(indexVBO));
		std::unique_ptr<CG_Data::VBO> meshVBO = std::make_unique<CG_Data::VBO>((void*)&vertices[0], 24 * sizeof(float), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
		CubemapVAO->AddVBO(std::move(meshVBO));
	}

	void Cubemap::CubemapRenderer(RenderPass& _Pass, void* _Data) {
		_Pass.shader->UseShader();
		_Pass.BatchVao->BindVAO();
		_Pass.Textures[0]->Bind();
		_Pass.DrawFunction();
	}

}