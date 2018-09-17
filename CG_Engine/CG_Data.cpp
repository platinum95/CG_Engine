#include "CG_Data.h"
#include <stdexcept>
#include <glm/vec3.hpp>
#include "CG_Engine.h"

namespace GL_Engine{
	namespace CG_Data{

#pragma region VBO
		VBO::VBO(){
			glGenBuffers(1, &this->ID);
			initialised = true;
			Target = GL_ARRAY_BUFFER;
		}
		VBO::VBO(void* _Data, uint64_t _DataSize, GLenum _Usage, GLenum _Target){
			glGenBuffers(1, &this->ID);
			Target = _Target;
			Usage = _Usage;
			SetVBOData(_Data, _DataSize);
			initialised = true;
		}

		VBO::~VBO(){
			if (initialised) {
				glDeleteBuffers(1, &this->ID);
				initialised = false;
			}
		}

		void VBO::Cleanup() {
			if (initialised) {
				glDeleteBuffers(1, &this->ID);
				initialised = false;
			}
		}

		const GLuint VBO::GetID() const{
			return this->ID;
		}
		void VBO::BindVBO() const{
			glBindBuffer(Target, this->ID);
		}

		void VBO::SetVBOData(void* _Data, uint64_t _DataSize) const{
			glBindBuffer(Target, this->ID);
			glBufferData(Target, _DataSize, _Data, Usage);
		}
#pragma endregion

#pragma region VAO

		VAO::VAO(){
			glGenVertexArrays(1, &this->VAOId);
			initialised = true;
		}

		VAO::~VAO(){
			if (initialised) {
				this->BindVAO();
				for (auto &vbo : this->VBOs){
					vbo.reset();
				}
				VBOs.clear();
				glDeleteVertexArrays(1, &this->VAOId);
				initialised = false;
			}
		}

		void VAO::Cleanup(){
			if (initialised) {
				this->BindVAO();
				for (auto &vbo : this->VBOs){
					vbo.reset();
				}
				VBOs.clear();
				glDeleteVertexArrays(1, &this->VAOId);
				initialised = false;
			}
		}

		const GLuint VAO::GetID() const{
			return this->VAOId;
		}

		void VAO::BindVAO() const{
			glBindVertexArray(this->VAOId);
		}
		void VAO::AddVBO(std::unique_ptr<VBO> _VBO) {
			this->VBOs.push_back(std::move(_VBO));
		}
#pragma endregion

#pragma region Texture
		Texture::Texture(void* _Data, GLint width, GLint height, GLuint _Unit, GLuint _ImageFormat, 
							std::function<void()> _Parameters, GLenum _Target) {
			glGenTextures(1, &this->ID);
			this->Target = _Target;
			this->Unit = _Unit;
			glActiveTexture(this->Unit);
			glBindTexture(this->Target, this->ID);
			_Parameters();

			glTexImage2D(this->Target, 0, GL_RGBA, width, height, 0, _ImageFormat, GL_UNSIGNED_BYTE, _Data);
			glGenerateMipmap(this->Target);
			Initialised = true;
		}

		Texture::Texture(GLuint _Unit, GLenum _Target) {
			glGenTextures(1, &this->ID);
			this->Target = _Target;
			this->Unit = _Unit;
			Initialised = true;
		}
		Texture::~Texture() {
			if (Initialised) {
				glDeleteTextures(1, &this->ID);
				Initialised = false;
			}
		}
		void Texture::Cleanup() {
			if (Initialised) {
				glDeleteTextures(1, &this->ID);
				Initialised = false;
			}
		}

		void Texture::SetUnit(const GLuint _Unit) {
			this->Unit = _Unit;
		}

		void Texture::Bind() {
			glActiveTexture(this->Unit);
			glBindTexture(this->Target, this->ID);
		}

		const GLuint Texture::GetID() const { return this->ID; }

#pragma region Uniform
		Uniform::Uniform(GLint _Location, void* _Data, std::function<void(const CG_Data::Uniform&)> _Callback){
			this->ID = _Location;
			this->Data = _Data;
			this->UpdateCallback = _Callback;
			this->Initialised = true;
		}
		Uniform::Uniform() {
			this->ID = -1;
			this->Data = nullptr;
			this->UpdateCallback = nullptr;
			this->Initialised = false;
		}
		Uniform::~Uniform() {
			if (Initialised) {
				Initialised = false;
			}
		}
		void Uniform::Cleanup() {
			if (Initialised) {
				Initialised = false;
			}
		}
		const GLint Uniform::GetID() const {
			return this->ID;
		}

		void Uniform::SetID(GLint _ID) {
			this->ID = _ID;
		}

		void Uniform::SetData(const void* _Data) {
			this->NeedsUpdating = true;
			this->Data = _Data;
		}

		const void* Uniform::GetData() const {
			return this->Data; 
		}

		void Uniform::Update() const{
			if (!Initialised)
				return;
			UpdateCallback(*this);
		}
		void Uniform::SetUpdateCallback(std::function<void(const CG_Data::Uniform &u)> _callback){
			this->UpdateCallback = _callback;
			this->Initialised = true;
		}
#pragma endregion

#pragma region UBO
		GLuint UBO::UBO_Count = 0;
		UBO::UBO() {};
		UBO::UBO(void* _Data, size_t _DataSize) {
			this->Data = _Data;
			this->DataSize = _DataSize;
			this->Target = GL_UNIFORM_BUFFER;
			this->Usage = GL_DYNAMIC_DRAW;
			this->SetVBOData(Data, DataSize);
			this->BindingPost = UBO_Count++;
			glBindBufferBase(GL_UNIFORM_BUFFER, this->BindingPost, this->ID);
		}
		UBO::~UBO() {
			this->Cleanup();
		}

		void UBO::UpdateUBO() const {
			glBindBuffer(GL_UNIFORM_BUFFER, this->ID);
			GLvoid* UBO_Pointer = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
			memcpy(UBO_Pointer, this->Data, this->DataSize);
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}
		const GLuint UBO::GetBindingPost()const {
			return this->BindingPost;
		}

#pragma endregion

#pragma region FBO
			FBO::RenderbufferObject::RenderbufferObject(uint16_t _Width, uint16_t _Height, GLenum _Type) {
				glGenRenderbuffers(1, &this->ID);
				glBindRenderbuffer(GL_RENDERBUFFER, this->ID);
				glRenderbufferStorage(GL_RENDERBUFFER, _Type, _Width, _Height);

			}
			void FBO::RenderbufferObject::Bind() const {

			}

			FBO::TexturebufferObject::TexturebufferObject(uint16_t _Width, uint16_t _Height, uint8_t _Unit) {
				auto parameters = []() {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				};
				TextureObject = std::make_shared<Texture>(nullptr, _Width, _Height, GL_TEXTURE0 + _Unit, GL_RGB, parameters, GL_TEXTURE_2D);
				this->ID = TextureObject->GetID();
			}
			void FBO::TexturebufferObject::TexturebufferObject::Bind() const {

			}
			const std::shared_ptr<Texture> FBO::TexturebufferObject::GetTexture() const { 
				return this->TextureObject; 
			}
		
	
			FBO::FBO(uint16_t _Width, uint16_t _Height) {
				glGenFramebuffers(1, &this->ID);
				this->Width = _Width;
				this->Height = _Height;
				Initialised = true;
			}
			FBO::~FBO() {
				if (Initialised) {
					this->Cleanup();
					Initialised = false;
				}
			}
			void FBO::Cleanup() {
				if (Initialised) {
					glDeleteFramebuffers(1, &this->ID);
					Initialised = false;
				}
			}
			std::shared_ptr<FBO::AttachmentBufferObject> FBO::AddAttachment(AttachmentType _Attachment, uint16_t _Width, uint16_t _Height) {
				glBindFramebuffer(GL_FRAMEBUFFER, this->ID);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				switch (_Attachment) {
				case TextureAttachment: {
					std::shared_ptr<TexturebufferObject> TexObj = std::make_shared<TexturebufferObject>(_Width, _Height, TextureAttachmentCount);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + TextureAttachmentCount++, GL_TEXTURE_2D, TexObj->ID, 0);
					this->Attachments.push_back(std::move(TexObj));
					break;
				}
				case StencilAttachment: {
					std::shared_ptr<RenderbufferObject> rbo = std::make_shared<RenderbufferObject>(_Width, _Height, GL_STENCIL_INDEX8);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo->ID);
					this->Attachments.push_back(std::move(rbo));
					break;
				}
				case DepthAttachment: {
					std::shared_ptr<RenderbufferObject> rbo = std::make_shared<RenderbufferObject>(_Width, _Height, GL_DEPTH_COMPONENT32);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo->ID);
					this->Attachments.push_back(std::move(rbo));
					break;
				}
				}
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
					this->complete = true;
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				std::shared_ptr<AttachmentBufferObject> abo = Attachments.back();
				return abo;

			}
			void FBO::Bind(uint8_t _ColourAttachment) const {
				if (!this->complete)
					throw std::runtime_error("Attempting to bind incomplete framebuffer!\n");
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, this->ID);
				glDrawBuffer(GL_COLOR_ATTACHMENT0 + _ColourAttachment);
				glClearColor(0.0, 0.0, 0.0, 1.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				glViewport(0, 0, Width, Height);
			}

			void FBO::Bind(uint16_t _Count, const GLenum* _ColourAttachments) const {
				if (!this->complete)
					throw std::runtime_error("Attempting to bind incomplete framebuffer!\n");
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, this->ID);
				glClearColor(0.0, 0.0, 0.0, 1.0);
				for (int i = 0; i < _Count; i++) {
					glDrawBuffer(_ColourAttachments[i]);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				}
				glDrawBuffers(_Count, _ColourAttachments);
				glViewport(0, 0, Width, Height);
			}
			const GLuint FBO::GetID() const {
				return this->ID;
			}
			void FBO::Unbind() const {
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(0, 0, CG_Engine::ViewportWidth, CG_Engine::ViewportHeight);
			}

	}
}