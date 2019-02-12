#pragma 
#include "Entity.h"
#include <filesystem>

namespace GL_Engine {

	/*-------------ModelLoader Class------------*/
	/*
	*Handles the loading and memory of models from file
	*/
	
	class ModelLoader {
	public:
		// Load the attributes of a model from a given file path
		ModelAttribList loadModel( const std::string & _modelPath, unsigned int _flags );

		// Load the attributes of a given rigged model (with bones) 
		// from a given file path
		std::unique_ptr< RiggedModel > 
			loadRiggedModel( const std::string &_modelPath, 
							 unsigned int _flags );

		// Cleanup references etc.
		void cleanup();

		// Load a model's material from a given Assimp model object
		static std::vector< std::shared_ptr< CG_Data::Texture > >
			loadMaterial( const aiMaterial *material,
						  const aiTextureType _Type,
						  const std::filesystem::path &_PathBase,
						  std::vector< std::shared_ptr< CG_Data::Texture > >
						  	& _textures );

		// Load a model's texture from file
		static std::shared_ptr< CG_Data::Texture > 
			loadTexture( const std::filesystem::path & _Path,
						 GLuint _Unit );

	private:
		Assimp::Importer aImporter;

		// Texture cache for shared textures
		static std::map < std::string, std::shared_ptr< CG_Data::Texture > >
			cachedTextures ;
	};
	

}