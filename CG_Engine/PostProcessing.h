#pragma once

#include "CG_Data.h"
#include "Shader.h"
#include <sstream>
#include <glm\vec2.hpp>
namespace GL_Engine {
	class PostProcessing{
	public:
		enum PostprocessingAttachment{
			GaussianBlur, SaturationAdjust, ContrastAdjust, BrightnessAdjust
		};

		PostProcessing();
		~PostProcessing();
		void Cleanup();
		CG_Data::Uniform* AddAttachment(PostprocessingAttachment _Attachment);

		std::shared_ptr<CG_Data::Texture> Compile(std::shared_ptr<CG_Data::Texture> _TextureInput, uint16_t _Width, uint16_t _Height);
		const std::shared_ptr<CG_Data::Texture> GetOutputTexture() const;
		void Process();
		const CG_Data::FBO *GetFBO() const;

	private:
		std::unique_ptr<CG_Data::FBO> ProcessingFBO;
		std::shared_ptr<CG_Data::Texture> OutputColourBuffer, InputTexture;
		Shader shader;
		std::unique_ptr<CG_Data::VAO> ScreenVAO;
		std::vector<CG_Data::Uniform*> Uniforms;
		std::string AttachmentStringComponents[2];
		std::map<PostprocessingAttachment, std::string> uniforms;
		glm::vec2 Resolution;
		std::string VertexShader = {
				#include "PostprocessingV.glsl" 
		};
		std::string FragmentShader = {
				#include "PostprocessingF.glsl" 
		};

		static float VertexPositions[12];
		static float TextureCoordinates[8];
		static unsigned int PostProcessing::Indices[6];

	};

}

