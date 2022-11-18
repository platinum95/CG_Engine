#include "PostProcessing.h"
#include "CG_Engine.h"

namespace GL_Engine {

	float PostProcessing::VertexPositions[]{
		-1, 1, -1.0,
		-1, -1, -1.0,
		1, -1, -1.0,
		1, 1, -1.0
	};

	float PostProcessing::TextureCoordinates[]{
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0
	};
	unsigned int PostProcessing::Indices[]{
		0, 1, 3,
		1, 2, 3
	};


	PostProcessing::PostProcessing() {
		AttachmentStringComponents[1] = "void main(){\n";
	}
	PostProcessing::~PostProcessing() {
		ProcessingFBO.reset();
		OutputColourBuffer.reset();
		InputTexture.reset();
		ScreenVAO.reset();
		shader.cleanup();
	}
	void PostProcessing::Cleanup() {
		ProcessingFBO.reset();
		OutputColourBuffer.reset();
		InputTexture.reset();
		ScreenVAO.reset();
		shader.cleanup();
	}
	CG_Data::Uniform* PostProcessing::AddAttachment(PostprocessingAttachment _Attachment) {
		// TODO
		//switch (_Attachment)
		//{
		//case GaussianBlur:
		//{
			auto GaussianLambdaUpdater = [](const CG_Data::Uniform &u) {glUniform1fv(u.GetID(), 5, static_cast<const GLfloat*>(u.GetData())); };
			auto uni = shader.registerUniform("GaussianWeights", GaussianLambdaUpdater);
			AttachmentStringComponents[0] += "uniform float GaussianWeights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);\n";
			AttachmentStringComponents[1] += "BloomEffect();\n";
			return uni.get();
		//};
		//case SaturationAdjust:
		//{
		//
		//};
		//}
	}

	std::shared_ptr<CG_Data::Texture> PostProcessing::Compile(std::shared_ptr<CG_Data::Texture> _TextureInput, uint16_t _Width, uint16_t _Height) {

		this->Resolution = glm::vec2(_Width, _Height);
		InputTexture = _TextureInput;
		//std::string FragmentShader;
		std::stringstream FragmentStream(FragmentShader);
		FragmentStream << "#version 330\n";
		FragmentStream << AttachmentStringComponents[0];
		FragmentStream << FragmentShader;
		FragmentStream << AttachmentStringComponents[1];

		FragmentStream << "}\n";
		FragmentShader = FragmentStream.str();
		shader.registerShaderStage(this->VertexShader.c_str(), GL_VERTEX_SHADER);
		shader.registerShaderStage(this->FragmentShader.c_str(), GL_FRAGMENT_SHADER);
		shader.registerAttribute("vPosition", 0);
		shader.registerAttribute("vTextureCoord", 1);
		shader.registerTextureUnit("InputImage", 0);
		auto resoUni = shader.registerUniform("resolution");
		shader.compileShader();
		shader.useShader();
		glUniform2f(resoUni->GetID(), (float)_Width, (float)_Height);

		ScreenVAO = std::make_unique<CG_Data::VAO>();
		ScreenVAO->BindVAO();
		auto IndexVBO = std::make_unique<CG_Data::VBO>(&Indices[0], 6 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
		auto VertexVBO = std::make_unique<CG_Data::VBO>(VertexPositions, 4 * 3 * sizeof(float), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		auto TexVBO = std::make_unique<CG_Data::VBO>(TextureCoordinates, 4 * 2 * sizeof(float), GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		ScreenVAO->AddVBO(std::move(VertexVBO));
		ScreenVAO->AddVBO(std::move(TexVBO));
		ScreenVAO->AddVBO(std::move(IndexVBO));

		ProcessingFBO = std::make_unique<CG_Data::FBO>(_Width, _Height);
		auto FragColAttach = ProcessingFBO->addAttachment(CG_Data::FBO::AttachmentType::ColourTexture, _Width, _Height);
		ProcessingFBO->addAttachment(CG_Data::FBO::AttachmentType::DepthRenderbuffer, _Width, _Height);
		this->OutputColourBuffer = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(FragColAttach)->GetTexture();
		OutputColourBuffer->SetUnit(GL_TEXTURE0);
		FragmentShader.clear();
		VertexShader.clear();

		return this->OutputColourBuffer;

	}

	const std::shared_ptr<CG_Data::Texture> PostProcessing::GetOutputTexture() const {
		return this->OutputColourBuffer;
	}

	void PostProcessing::Process() {
		auto bindToken = ProcessingFBO->bind(0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.useShader();
		auto loc = glGetUniformLocation(shader.getShaderID(), "resolution");
		glUniform2f(loc, Resolution.x, Resolution.y);

		for (auto u : Uniforms)
			u->Update();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, InputTexture->GetID());

		ScreenVAO->BindVAO();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	const CG_Data::FBO *PostProcessing::GetFBO() const { 
		return this->ProcessingFBO.get(); 
	}
}