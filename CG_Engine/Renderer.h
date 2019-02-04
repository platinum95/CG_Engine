#pragma once

#include "Entity.h"
namespace GL_Engine {

	class Renderer {
	public:
		Renderer();
		~Renderer();
		void Cleanup();

		std::shared_ptr< RenderPass > AddRenderPass( Shader* _Shader );
		std::shared_ptr< RenderPass > AddRenderPass( Shader* _Shader, std::function<void(RenderPass&, void*)> _RenderFunction, void* _Data );
		void AddRenderPass( std::shared_ptr< RenderPass > _RPass );
		void AddUBO( CG_Data::UBO* _ubo );

		const std::vector< std::shared_ptr< RenderPass > > & getRenderPasses() const;

		void Render() const;

	private:
		std::vector< std::shared_ptr< RenderPass > > renderPasses;
		static void DefaultRenderer(RenderPass&, void*);
		std::vector<CG_Data::UBO*> UBO_List;
	};

}