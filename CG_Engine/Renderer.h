#pragma once

#include "Entity.h"
namespace GL_Engine {

	class Renderer {
	public:
		Renderer();
		~Renderer();
		void Cleanup();

		RenderPass* AddRenderPass(Shader* _Shader);
		RenderPass* AddRenderPass(Shader* _Shader, std::function<void(RenderPass&, void*)> _RenderFunction, void* _Data);
		RenderPass* AddRenderPass(std::unique_ptr<RenderPass> _RPass);
		void AddUBO(CG_Data::UBO* _ubo);

		void Render() const;

	private:
		std::vector<std::unique_ptr<RenderPass>> renderPasses;
		static void DefaultRenderer(RenderPass&, void*);
		std::vector<CG_Data::UBO*> UBO_List;
	};

}