#include "Renderer.h"

using namespace GL_Engine;

Renderer::Renderer() {
	this->renderPasses = std::vector<std::shared_ptr<RenderPass>>();
}


Renderer::~Renderer() {
	for (auto &rp : renderPasses) {
		rp.reset();
	}
}

void Renderer::Cleanup() {

}

std::shared_ptr<RenderPass> GL_Engine::Renderer::AddRenderPass() {
	
	auto rPass =  std::make_shared< RenderPass >();
	rPass->shader = nullptr;
	rPass->renderFunction = DefaultRenderer;
	rPass->Data = nullptr;
	this->renderPasses.push_back( rPass );
	return std::move( rPass );	
}

std::shared_ptr<RenderPass> GL_Engine::Renderer::AddRenderPass( Shader* _Shader ) {
	auto rPass =  std::make_shared<RenderPass>() ;
	rPass->renderFunction = DefaultRenderer;
	rPass->Data = nullptr;
	rPass->shader = _Shader;
	auto passOut = rPass.get();
	this->renderPasses.push_back( rPass );
	return std::move( rPass );	
}

void GL_Engine::Renderer::AddRenderPass(std::shared_ptr<RenderPass> _RPass) {
	auto passOut = _RPass.get();
	this->renderPasses.push_back( std::move( _RPass ) );
	return;
}

std::shared_ptr<RenderPass> GL_Engine::Renderer::AddRenderPass(Shader* _Shader, std::function<void(RenderPass&, void*)> _RenderFunction, void * _Data) {
	auto rPass = std::make_shared<RenderPass>();
	rPass->renderFunction = _RenderFunction;
	rPass->Data = _Data;
	rPass->shader = _Shader;
	auto passOut = rPass.get();
	this->renderPasses.push_back( rPass );
	return std::move( rPass );
}

const std::vector< std::shared_ptr< RenderPass > > & GL_Engine::Renderer::getRenderPasses() const{
	return this->renderPasses;
}

void GL_Engine::Renderer::Render() {
	for (auto ubo : this->UBO_List) {
		ubo->UpdateUBO();
	}
	for (auto&& pass : renderPasses) {
		pass->renderFunction(*pass, pass->Data);
	}
	
}

void GL_Engine::Renderer::AddUBO(CG_Data::UBO* _ubo) {
	// Make sure its a unique list
	if ( std::find( UBO_List.begin(),
					UBO_List.end(), _ubo) == UBO_List.end()) {
  		this->UBO_List.push_back(_ubo);
	}
	
}


void Renderer::DefaultRenderer(RenderPass& _Pass, void* _Data) {
	_Pass.shader->useShader();
	_Pass.BatchVao->BindVAO();
	for (auto tex : _Pass.Textures) {
		tex->Bind();
	}
	for (auto&& batch : _Pass.batchUnits) {
		if (batch->active && batch->entity->isActive()) {
			for (auto l : _Pass.dataLink) {
				l.uniform->SetData(batch->entity->GetData(l.eDataIndex));
				l.uniform->Update();
			}
			batch->entity->UpdateUniforms();
			batch->entity->update();
			_Pass.DrawFunction();
		}
	}
	
}


BatchUnit* RenderPass::AddBatchUnit(Entity* _Entity) {
	auto batchUnit = std::make_unique<BatchUnit>();
	batchUnit->entity = _Entity;
	auto pOut = batchUnit.get();
	batchUnits.push_back(std::move(batchUnit));
	return pOut;
}

void RenderPass::SetDrawFunction(std::function<void(void)> _dFunc) {
	DrawFunction = _dFunc;
}