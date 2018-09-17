#pragma once

#include "CG_Data.h"
#include "Renderer.h"
#include "File_IO.h"
#include "Shader.h"
namespace GL_Engine {
	class Cubemap{
	public:
		Cubemap(const std::vector<std::string> &_TextureFiles, Shader *_CubemapShader, Renderer *_Renderer);
		~Cubemap();

		void GenerateCubemap(const std::vector<std::string> &_TextureFiles);

		const RenderPass *GetRenderPass() const;

	private:
		void CreateShader(Shader *_Shader);
		void SetupRenderPass(Renderer *_Renderer);
		void SetupArrayObjects();

		std::shared_ptr<CG_Data::Texture> MapTexture;
		RenderPass* CubeRenderPass;
		Shader *CubemapShader;
		std::shared_ptr<CG_Data::VAO> CubemapVAO;

		static const float vertices[24];
		static const unsigned int indices[36];

		static void CubemapRenderer(RenderPass&, void*);


	};

}

