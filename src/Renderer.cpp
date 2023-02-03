#include "Renderer.h"

#include "MaterialData.h"
#include "Shader.h"

#include <ranges>

namespace GL_Engine {

Renderer::Renderer() {
	this->renderPasses = std::vector<std::shared_ptr<RenderPass>>();
}


Renderer::~Renderer() {
	for ( auto &rp : renderPasses ) {
		rp.reset();
	}
}

void Renderer::Cleanup() {

}


std::unique_ptr<IRenderable> Renderer::AssignShader2( std::shared_ptr<SimpleModelEntity> model, std::shared_ptr<CG_Data::UBO> lightUbo, std::shared_ptr<CG_Data::UBO> cameraUbo ) {
	auto material = model->GetMaterial();
	cg_assert( material.HasComponent( MaterialComponent::Phong ) );

	if ( material.HasComponent( MaterialComponent::DiffuseTexture ) ) {
		const std::string vSource = {
			#include "res/PhongShaderTexturedV.glsl"
		};
		const std::string fSource = {
			#include "res/PhongShaderTexturedF.glsl"
		};

		auto shader = std::make_shared<Shader>();
		shader->registerShaderStage( vSource, GL_VERTEX_SHADER );
		shader->registerShaderStage( fSource, GL_FRAGMENT_SHADER );
		shader->registerAttribute( "i_position", 0 );
		shader->registerAttribute( "i_normal", 1 );
		shader->registerUBO( "LightData", std::move( lightUbo ) );
		shader->registerUBO( "CameraProjectionData", std::move( cameraUbo ) );
		shader->registerUboPlaceholder( "MaterialData" );

		auto matrixUniformUpdateLambda = [] ( const CG_Data::Uniform &u ) {
			glUniformMatrix4fv( u.GetID(), 1, GL_FALSE,
				static_cast<const GLfloat *>( u.GetData() ) );
		};
		shader->registerUniform( "modelMatrix", std::move( matrixUniformUpdateLambda ) );

		shader->compileShader();

		return std::make_unique<TempTexturedShaderRenderNode2<SimpleModelEntity>>( std::move( shader ), std::move( model ) );
	}
	else {
		const std::string vSource = {
			#include "res/PhongShaderV.glsl"
		};
		const std::string fSource = {
			#include "res/PhongShaderF.glsl"
		};

		auto shader = std::make_shared<Shader>();
		shader->registerShaderStage( vSource, GL_VERTEX_SHADER );
		shader->registerShaderStage( fSource, GL_FRAGMENT_SHADER );
		shader->registerAttribute( "i_position", 0 );
		shader->registerAttribute( "i_normal", 1 );
		shader->registerUBO( "LightData", std::move( lightUbo ) );
		shader->registerUBO( "CameraProjectionData", std::move( cameraUbo ) );
		shader->registerUboPlaceholder( "MaterialData" );

		auto matrixUniformUpdateLambda = [] ( const CG_Data::Uniform &u ) {
			glUniformMatrix4fv( u.GetID(), 1, GL_FALSE,
				static_cast<const GLfloat *>( u.GetData() ) );
		};
		shader->registerUniform( "modelMatrix", std::move( matrixUniformUpdateLambda ) );

		shader->compileShader();

		return std::make_unique<TempSimpleShaderRenderNode2<SimpleModelEntity>>( std::move( shader ), std::move( model ) );
	}
}

std::unique_ptr<IRenderable> Renderer::AssignShader( std::shared_ptr<ModelAttribute> model, std::shared_ptr<CG_Data::UBO> lightUbo, std::shared_ptr<CG_Data::UBO> cameraUbo ) {
	// TODO - shader cache
	// TODO - check shader type
	//const auto shadingMode = aiShadingMode_Phong;

	//switch ( shadingMode )
	//{
	//	case aiShadingMode_Flat :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_Gouraud :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_Phong :
	//	{
	auto material = model->GetMaterial();
	cg_assert( material.HasComponent( MaterialComponent::Phong ) );

	if ( material.HasComponent( MaterialComponent::DiffuseTexture ) ) {
		const std::string vSource = {
			#include "res/PhongShaderTexturedV.glsl"
		};
		const std::string fSource = {
			#include "res/PhongShaderTexturedF.glsl"
		};

		auto shader = std::make_shared<Shader>();
		shader->registerShaderStage( vSource, GL_VERTEX_SHADER );
		shader->registerShaderStage( fSource, GL_FRAGMENT_SHADER );
		shader->registerAttribute( "i_position", 0 );
		shader->registerAttribute( "i_normal", 1 );
		shader->registerUBO( "LightData", std::move( lightUbo ) );
		shader->registerUBO( "CameraProjectionData", std::move( cameraUbo ) );
		shader->registerUboPlaceholder( "MaterialData" );

		auto matrixUniformUpdateLambda = [] ( const CG_Data::Uniform &u ) {
			glUniformMatrix4fv( u.GetID(), 1, GL_FALSE,
				static_cast<const GLfloat *>( u.GetData() ) );
		};
		shader->registerUniform( "modelMatrix", std::move( matrixUniformUpdateLambda ) );

		shader->compileShader();

		return std::make_unique<TempTexturedShaderRenderNode>( std::move( shader ), std::move( model ) );
	}
	else {
		const std::string vSource = {
			#include "res/PhongShaderV.glsl"
		};
		const std::string fSource = {
			#include "res/PhongShaderF.glsl"
		};

		auto shader = std::make_shared<Shader>();
		shader->registerShaderStage( vSource, GL_VERTEX_SHADER );
		shader->registerShaderStage( fSource, GL_FRAGMENT_SHADER );
		shader->registerAttribute( "i_position", 0 );
		shader->registerAttribute( "i_normal", 1 );
		shader->registerUBO( "LightData", std::move( lightUbo ) );
		shader->registerUBO( "CameraProjectionData", std::move( cameraUbo ) );
		shader->registerUboPlaceholder( "MaterialData" );

		auto matrixUniformUpdateLambda = [] ( const CG_Data::Uniform &u ) {
			glUniformMatrix4fv( u.GetID(), 1, GL_FALSE,
				static_cast<const GLfloat *>( u.GetData() ) );
		};
		shader->registerUniform( "modelMatrix", std::move( matrixUniformUpdateLambda ) );

		shader->compileShader();

		return std::make_unique<TempSimpleShaderRenderNode>( std::move( shader ), std::move( model ) );
	}
	//	}
	//	case aiShadingMode_Blinn :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_Toon :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_OrenNayar :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_Minnaert :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_CookTorrance :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_NoShading :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_Fresnel :
	//	{
	//		break;
	//	}
	//	case aiShadingMode_PBR_BRDF:
	//	{
	//		break;
	//	}
	//	default:
	//	{
	//		return nullptr;
	//	}
	//};

	//return nullptr;
}

std::shared_ptr<RenderPass> GL_Engine::Renderer::AddRenderPass() {

	auto rPass = std::make_shared< RenderPass >();
	rPass->shader = nullptr;
	rPass->renderFunction = DefaultRenderer;
	rPass->Data = nullptr;
	this->renderPasses.push_back( rPass );
	return std::move( rPass );
}

std::shared_ptr<RenderPass> GL_Engine::Renderer::AddRenderPass( Shader *_Shader ) {
	auto rPass = std::make_shared<RenderPass>();
	rPass->renderFunction = DefaultRenderer;
	rPass->Data = nullptr;
	rPass->shader = _Shader;
	auto passOut = rPass.get();
	this->renderPasses.push_back( rPass );
	return std::move( rPass );
}

void GL_Engine::Renderer::AddRenderPass( std::shared_ptr<RenderPass> _RPass ) {
	auto passOut = _RPass.get();
	this->renderPasses.push_back( std::move( _RPass ) );
	return;
}

std::shared_ptr<RenderPass> GL_Engine::Renderer::AddRenderPass( Shader *_Shader, std::function<void( RenderPass &, void * )> _RenderFunction, void *_Data ) {
	auto rPass = std::make_shared<RenderPass>();
	rPass->renderFunction = _RenderFunction;
	rPass->Data = _Data;
	rPass->shader = _Shader;
	auto passOut = rPass.get();
	this->renderPasses.push_back( rPass );
	return std::move( rPass );
}

const std::vector< std::shared_ptr< RenderPass > > &GL_Engine::Renderer::getRenderPasses() const {
	return this->renderPasses;
}

void GL_Engine::Renderer::Render() {
	for ( auto ubo : this->UBO_List ) {
		ubo->UpdateUBO();
	}
	for ( auto &&pass : renderPasses ) {
		pass->renderFunction( *pass, pass->Data );
	}

}

void GL_Engine::Renderer::AddUBO( CG_Data::UBO *_ubo ) {
	// Make sure its a unique list
	if ( std::find( UBO_List.begin(),
		UBO_List.end(), _ubo ) == UBO_List.end() ) {
		this->UBO_List.push_back( _ubo );
	}
}


void Renderer::DefaultRenderer( RenderPass &_Pass, void *_Data ) {
	UsingScopedToken( _Pass.shader->useShader() ) {
		_Pass.BatchVao->BindVAO();
		for ( auto tex : _Pass.Textures ) {
			tex->Bind();
		}
		for ( auto &&batch : _Pass.batchUnits ) {
			if ( batch->active && batch->entity->isActive() ) {
				for ( auto l : _Pass.dataLink ) {
					l.uniform->SetData( batch->entity->GetData( l.eDataIndex ) );
					l.uniform->Update();
				}
				batch->entity->UpdateUniforms();
				batch->entity->update();
				_Pass.DrawFunction();
			}
		}
	}
}


BatchUnit *RenderPass::AddBatchUnit( Entity *_Entity ) {
	auto batchUnit = std::make_unique<BatchUnit>();
	batchUnit->entity = _Entity;
	auto pOut = batchUnit.get();
	batchUnits.push_back( std::move( batchUnit ) );
	return pOut;
}

void RenderPass::SetDrawFunction( std::function<void( void )> _dFunc ) {
	DrawFunction = _dFunc;
}

}