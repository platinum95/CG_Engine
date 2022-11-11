#pragma once

#include "CG_Data.h"
#include "Renderer.h"
#include "File_IO.h"
#include "Shader.h"
#include "Camera.h"
#include <filesystem>

namespace GL_Engine {
	class Cubemap{
	public:
		Cubemap( const std::vector< std::filesystem::path > & _textureFiles,
				 Shader *_CubemapShader, Renderer *_Renderer );
		~Cubemap();

		void GenerateCubemap( const std::vector<std::filesystem::path> 
								& _textureFiles );

		const std::shared_ptr< RenderPass > GetRenderPass() const;

		std::shared_ptr<CG_Data::Texture> getTextureMap();
		static const float vertices[24];
		static const unsigned int indices[36];

	private:
		void CreateShader(Shader *_Shader);
		void SetupRenderPass(Renderer *_Renderer);
		void SetupArrayObjects();

		std::shared_ptr<CG_Data::Texture> MapTexture;
		std::shared_ptr< RenderPass >  CubeRenderPass;
		Shader *CubemapShader;
		std::shared_ptr<CG_Data::VAO> CubemapVAO;


		static void CubemapRenderer(RenderPass&, void*);


	};


	class EnvironmentMap{
	public:
		EnvironmentMap( const glm::vec3 &centre, uint16_t fbSize );

		~EnvironmentMap();
		void cleanup();

		void setStaticRenderer( std::shared_ptr< Renderer > _staticRenderer );
		void setDynamicRenderer( std::shared_ptr< Renderer > _dynamicRenderer );

		void renderStaticMap();
		void renderDynamicMap();

		std::shared_ptr< CG_Data::Texture > getStaticTexture();
		std::shared_ptr< CG_Data::Texture > getDynamicTexture();

		const std::shared_ptr< CG_Data::UBO > getCameraUbo() const;
		const Camera & getCamera() const;

	private:

		std::shared_ptr< CG_Data::Texture > staticColourTex, staticDepthTex;
		std::shared_ptr< CG_Data::Texture > dynamicColourTex, dynamicDepthTex;
		std::unique_ptr< CG_Data::FBO > dynamicFbo;

		uint16_t fbSize;

		std::shared_ptr< Renderer > staticRenderer, dynamicRenderer;
		Camera envCamera;




	};

}


