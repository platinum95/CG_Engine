#pragma once
#include "Common.h"
#include <vector>
#include <functional>
#include <iostream>
#include <tuple>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include "File_IO.h"


namespace GL_Engine{
	namespace CG_Data{
		/*-------------VBO Class------------*/
		/*
		*Handles everything to do with VBOs
		*/
		class VBO{
		public:
			VBO();

			//Construct a VBO by passing data, data size(in bytes), usage (i.e GL_STATIC_DRAW), and an optional target (i.e. GL_ARRAY_BUFFER)
			VBO(void* _Data, uint64_t _DataSize, GLenum _Usage, GLenum _Target = GL_ARRAY_BUFFER);

			~VBO();
			
			void Cleanup();

			//Returns the ID of the VBO
			const GLuint GetID() const;

			//Binds the VBO
			void BindVBO() const;

			//Sets the VBO data
			void SetVBOData(void* _Data, uint64_t _DataSize) const;

		protected:
			//The VBO target, e.g. GL_ARRAY_BUFFER
			GLenum Target;

			//The VBO usage, e.g. GL_STATIC_DRAW
			GLenum Usage;

			//The VBO ID
			GLuint ID;
		private:
			//Indicates whether or not the VBO has been initialised
			bool initialised{ false };
		};

		/*-------------VAO Class------------*/
		/*
		*Handles everything to do with VAOs
		*Keeps track of the VBOs assigned to the VAO
		*/
		class VAO{
		public:
			VAO();
			~VAO();
			void Cleanup();
			const GLuint GetID() const;
			void BindVAO() const;
			void AddVBO(std::unique_ptr<VBO> _VBO);
		protected:
			std::vector<std::unique_ptr<VBO>> VBOs;
		private:
			GLuint VAOId;
			bool initialised{ false };
		};


		/*-------------Texture Class------------*/
		/*
		*Handles everything to do with textures
		*/
		class Texture{
		public:
			Texture(void* _Data, GLint width, GLint height, GLuint _Unit, GLuint _ImageFormat, std::function<void()> _Parameters, GLenum _Target = GL_TEXTURE_2D);
			Texture(GLuint _Unit, GLenum _Target);
			~Texture();
			void Cleanup();

			void SetUnit(const GLuint _Unit);

			void Bind();

			const GLuint GetID() const;
			GLuint ID;
		protected:
			GLenum Target;
			GLuint Unit;
		private:
			bool Initialised{ false };
		};

		
		
		/*-------------Uniform Class------------*/
		/*
		*Handles everything to do with shader uniforms
		*/
		class Uniform{
		public:
			Uniform(GLint _Location, void* _Data, std::function<void(const CG_Data::Uniform&)> _Callback);
			Uniform();
			~Uniform();
			void Cleanup();

			void Update() const;
			void SetUpdateCallback(std::function<void(const CG_Data::Uniform&)> _callback);
			const GLint GetID() const;
			void SetData(const void* _Data);
			void SetID(GLint _ID);
			const void* GetData() const;

		private:
			bool NeedsUpdating{ false };
			bool Initialised{ false };
			const void *Data;
			GLint ID;
			std::function<void(const CG_Data::Uniform&)> UpdateCallback;
		};

		/*-------------UBO Class------------*/
		/*
		*Handles everything to do with uniform buffer objects
		*/
		class UBO : VBO {
		public:
			UBO();
			UBO(void* _Data, size_t _DataSize);
			~UBO();
			void UpdateUBO() const;
			const GLuint GetBindingPost()const;
		private:
			bool Initialised{ false };
			void* Data;
			size_t DataSize;
			GLuint BindingPost;
			static GLuint UBO_Count;
		};


		/*-------------FBO Class------------*/
		/*
		*Handles everything to do with framebuffer objects
		*/
		class FBO {
		public:
			class AttachmentBufferObject {
			public:
				GLuint ID;
				virtual void Bind() const = 0;
			};

			class RenderbufferObject : public AttachmentBufferObject {
			public:
				RenderbufferObject(uint16_t _Width, uint16_t _Height, GLenum _Type);
				void Bind() const;
			};

			class TexturebufferObject : public AttachmentBufferObject {
			public:
				TexturebufferObject(uint16_t _Width, uint16_t _Height, uint8_t _Unit);
				void Bind() const;
				const std::shared_ptr<Texture> GetTexture() const;
			private:
				std::shared_ptr<Texture> TextureObject;
			};

			enum AttachmentType {
				TextureAttachment, StencilAttachment, DepthAttachment
			};

			FBO(uint16_t _Width, uint16_t _Height);
			~FBO();
			void Cleanup();
			std::shared_ptr<AttachmentBufferObject> AddAttachment(AttachmentType _Attachment, uint16_t _Width, uint16_t _Height);
			void Bind(uint8_t _ColourAttachment = 0) const;

			void Bind(uint16_t _Count, const GLenum* _ColourAttachments) const;
			const GLuint GetID() const;
			void Unbind() const;

		private:
			uint16_t Width, Height;
			

		protected:
			GLuint ID;
			
		private:
			bool Initialised{ false }, complete{ false };
			uint8_t TextureAttachmentCount{ 0 };	
			std::vector<std::shared_ptr<AttachmentBufferObject>> Attachments;
		};
		
	}
	
}