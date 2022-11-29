#include "CG_Data.h"
#include "CG_Assert.h"

#include <glm/vec3.hpp>

#include <stdexcept>

namespace GL_Engine {
namespace CG_Data {

#pragma region VBO
VBO::VBO() {
    glGenBuffers( 1, &this->ID );
    initialised = true;
    Target = GL_ARRAY_BUFFER;
}
VBO::VBO( void *_Data, uint64_t _DataSize, GLenum _Usage, GLenum _Target ) {
    glGenBuffers( 1, &this->ID );
    Target = _Target;
    Usage = _Usage;
    if ( _DataSize > 0 ) {
        SetVBOData( _Data, _DataSize );
    }
    initialised = true;
}

VBO::~VBO() {
    if ( initialised ) {
        glDeleteBuffers( 1, &this->ID );
        initialised = false;
    }
}

void VBO::Cleanup() {
    if ( initialised ) {
        glDeleteBuffers( 1, &this->ID );
        initialised = false;
    }
}

const GLuint VBO::GetID() const {
    return this->ID;
}
void VBO::BindVBO() const {
    glBindBuffer( Target, this->ID );
}

void VBO::SetVBOData( void *_Data, uint64_t _DataSize ) const {
    glBindBuffer( Target, this->ID );
    glBufferData( Target, _DataSize, _Data, Usage );
}
#pragma endregion

#pragma region VAO

VAO::VAO() {
    glGenVertexArrays( 1, &this->VAOId );
    initialised = true;
}

VAO::~VAO() {
    if ( initialised ) {
        this->BindVAO();
        for ( auto &vbo : this->VBOs ) {
            vbo.reset();
        }
        VBOs.clear();
        glDeleteVertexArrays( 1, &this->VAOId );
        initialised = false;
    }
}

void VAO::Cleanup() {
    if ( initialised ) {
        this->BindVAO();
        for ( auto &vbo : this->VBOs ) {
            vbo.reset();
        }
        VBOs.clear();
        glDeleteVertexArrays( 1, &this->VAOId );
        initialised = false;
    }
}

GLuint VAO::getIndexCount() const {
    return this->numIndices;
}

const GLuint VAO::GetID() const {
    return this->VAOId;
}

void VAO::BindVAO() const {
    glBindVertexArray( this->VAOId );
}
void VAO::AddVBO( std::unique_ptr<VBO> _VBO ) {
    this->VBOs.push_back( std::move( _VBO ) );
}
#pragma endregion

#pragma region Texture
Texture::Texture( void *_Data, GLint width, GLint height, GLuint _Unit, GLuint _ImageFormat,
    std::function<void()> _Parameters, GLenum _Target ) {
    glGenTextures( 1, &this->ID );
    this->Target = _Target;
    this->Unit = _Unit;
    glActiveTexture( this->Unit );
    glBindTexture( this->Target, this->ID );
    _Parameters();

    glTexImage2D( this->Target, 0, GL_RGBA, width, height, 0, _ImageFormat, GL_UNSIGNED_BYTE, _Data );
    glGenerateMipmap( this->Target );
    Initialised = true;
}

Texture::Texture( GLuint _Unit, GLenum _Target ) {
    glGenTextures( 1, &this->ID );
    this->Target = _Target;
    this->Unit = _Unit;
    Initialised = true;
}

Texture::Texture( GLuint _Unit, GLenum _Target, std::function<void()> _Parameters ) {
    glGenTextures( 1, &this->ID );
    this->Target = _Target;
    this->Unit = _Unit;
    glActiveTexture( this->Unit );
    glBindTexture( this->Target, this->ID );
    _Parameters();
    Initialised = true;
}

Texture::Texture( GLuint _id, GLuint _unit, GLenum _target ) {
    this->Target = _target;
    this->ID = _id;
    this->Unit = _unit;
    this->Initialised = true;
}

Texture::~Texture() {
    if ( Initialised ) {
        glDeleteTextures( 1, &this->ID );
        Initialised = false;
    }
}
void Texture::Cleanup() {
    if ( Initialised ) {
        glDeleteTextures( 1, &this->ID );
        Initialised = false;
    }
}

void Texture::SetUnit( const GLuint _Unit ) {
    this->Unit = _Unit;
}

void Texture::Bind() {
    glActiveTexture( this->Unit );
    glBindTexture( this->Target, this->ID );
}

const GLuint Texture::GetID() const { return this->ID; }

#pragma region Uniform
Uniform::Uniform( GLint _Location, void *_Data, std::function<void( const CG_Data::Uniform & )> _Callback ) {
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
    if ( Initialised ) {
        Initialised = false;
    }
}
void Uniform::Cleanup() {
    if ( Initialised ) {
        Initialised = false;
    }
}
const GLint Uniform::GetID() const {
    return this->ID;
}

void Uniform::SetID( GLint _ID ) {
    this->ID = _ID;
}

void Uniform::SetData( const void *_Data ) {
    this->NeedsUpdating = true;
    this->Data = _Data;
}

const void *Uniform::GetData() const {
    return this->Data;
}

void Uniform::Update() const {
    if ( !Initialised )
        return;
    UpdateCallback( *this );
}
void Uniform::SetUpdateCallback(
    std::function< void( const CG_Data::Uniform &u )> _callback ) {
    this->UpdateCallback = _callback;
    this->Initialised = true;
}
#pragma endregion

#pragma region UBO
GLuint UBO::UBO_Count = 0;
UBO::UBO( void *_Data, size_t _DataSize ) {
    this->Data = _Data;
    this->DataSize = _DataSize;
    this->Target = GL_UNIFORM_BUFFER;
    this->Usage = GL_DYNAMIC_DRAW;
    this->SetVBOData( Data, DataSize );
    this->BindingPost = UBO_Count++;
    glBindBufferBase( GL_UNIFORM_BUFFER, this->BindingPost, this->ID );
}
UBO::~UBO() {
    this->Cleanup();
}

void UBO::setData( void *_data ) {
    this->Data = _data;
    this->UpdateUBO();
}

void UBO::UpdateUBO() const {
    glBindBuffer( GL_UNIFORM_BUFFER, this->ID );
    GLvoid *UBO_Pointer = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
    memcpy( UBO_Pointer, this->Data, this->DataSize );
    glUnmapBuffer( GL_UNIFORM_BUFFER );
}
const GLuint UBO::GetBindingPost()const {
    return this->BindingPost;
}

#pragma endregion

#pragma region FBO
FBO::RenderbufferObject::RenderbufferObject( uint16_t _width,
    uint16_t _height,
    GLenum _type ) {
    glGenRenderbuffers( 1, &this->ID );
    glBindRenderbuffer( GL_RENDERBUFFER, this->ID );

    glRenderbufferStorage( GL_RENDERBUFFER, _type, _width,
        _height );

}

void FBO::RenderbufferObject::bind() const {

}

FBO::TexturebufferObject::TexturebufferObject(
    std::shared_ptr< Texture > _tex ) {

    this->ID = _tex->GetID();
    this->TextureObject = std::move( _tex );
}

FBO::TexturebufferObject::TexturebufferObject( uint16_t _Width,
    uint16_t _Height,
    uint8_t _Unit ) {
    auto parameters = [] () {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            GL_NEAREST );
    };
    TextureObject =
        std::make_shared< Texture >( nullptr, _Width, _Height,
            GL_TEXTURE0 + _Unit, GL_RGBA,
            parameters, GL_TEXTURE_2D );
    this->ID = TextureObject->GetID();
}
void FBO::TexturebufferObject::TexturebufferObject::bind() const {

}
const std::shared_ptr< Texture >
FBO::TexturebufferObject::GetTexture() const {
    return this->TextureObject;
}


FBO::FBO( uint16_t _Width, uint16_t _Height ) {
    glGenFramebuffers( 1, &this->ID );
    this->width = _Width;
    this->height = _Height;
    initialised = true;
}
FBO::~FBO() {
    if ( initialised ) {
        this->cleanup();
        initialised = false;
    }
}
void FBO::cleanup() {
    if ( initialised ) {
        glDeleteFramebuffers( 1, &this->ID );
        initialised = false;
    }
}
std::shared_ptr< FBO::AttachmentBufferObject >
FBO::addAttachment( AttachmentType _Attachment, uint16_t _Width,
    uint16_t _Height ) {
    auto bindToken = staticBind<GL_FRAMEBUFFER>( { this->ID, glm::vec2( this->width, this->height ) } );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );
    switch ( _Attachment ) {
    case ColourTexture:
    {
        std::shared_ptr< TexturebufferObject > TexObj =
            std::make_shared< TexturebufferObject >(
                _Width, _Height, textureAttachmentCount );
        glFramebufferTexture2D( GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 +
            textureAttachmentCount++,
            GL_TEXTURE_2D, TexObj->ID, 0 );
        this->attachments.push_back( std::move( TexObj ) );
        break;
    }
    case DepthTexture:
    {
        GLuint depthTextureId;
        glGenTextures( 1, &depthTextureId );
        glBindTexture( GL_TEXTURE_2D, depthTextureId );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            GL_NEAREST );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
            _Width, _Height, 0, GL_DEPTH_COMPONENT,
            GL_FLOAT, nullptr );
        auto depthTexture = std::make_shared< CG_Data::Texture >(
            depthTextureId, GL_TEXTURE0, GL_TEXTURE_2D );

        glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            depthTexture->GetID(), 0 );

        std::shared_ptr< TexturebufferObject > texObj =
            std::make_shared< TexturebufferObject >(
                depthTexture );
        this->attachments.push_back( std::move( texObj ) );
        break;
    }

    case ColourRenderbuffer:
    {
        std::shared_ptr< RenderbufferObject > rbo =
            std::make_shared< RenderbufferObject >(
                _Width, _Height, GL_RGBA );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_RENDERBUFFER, rbo->ID );
        this->attachments.push_back( std::move( rbo ) );
        break;
    }

    case StencilRenderbuffer:
    {
        std::shared_ptr< RenderbufferObject > rbo =
            std::make_shared< RenderbufferObject >(
                _Width, _Height, GL_STENCIL_INDEX8 );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER,
            GL_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, rbo->ID );
        this->attachments.push_back( std::move( rbo ) );
        break;
    }
    case DepthRenderbuffer:
    {
        std::shared_ptr< RenderbufferObject > rbo =
            std::make_shared< RenderbufferObject >(
                _Width, _Height, GL_DEPTH_COMPONENT32 );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, rbo->ID );
        this->attachments.push_back( std::move( rbo ) );
        break;
    }
    }
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) ==
        GL_FRAMEBUFFER_COMPLETE ) {
        this->complete = true;
    }

    std::move( bindToken ).release();

    std::shared_ptr< AttachmentBufferObject > abo =
        attachments.back();
    return abo;

}

const GLuint FBO::getID() const {
    return this->ID;
}

//void FBO::unbind() const {
//	staticUnbind( this->ID );
//}

std::list<FBO::FramebufferBindData> FBO::FramebufferStack;
std::list<FBO::FramebufferBindData> FBO::ReadFramebufferStack;
std::list<FBO::FramebufferBindData> FBO::DrawFramebufferStack;


void FBO::staticUnbind( FBO::FramebufferBindData &data, std::list<FramebufferBindData> &stack ) {
    const auto entry = std::find( stack.crbegin(), stack.crend(), data );
    if ( entry == stack.crend() ) {
        throw std::runtime_error( "Attempting to unbind untracked framebuffer!\n" );
    }

    if ( entry == stack.crbegin() ) {
        stack.pop_back();
        const auto newFboIdIt = stack.crbegin();
        const auto newFboId = newFboIdIt == stack.crend() ? FramebufferBindData{ 0, glm::vec2( 1920, 1080 ) } : *newFboIdIt;
        glBindFramebuffer( GL_FRAMEBUFFER, newFboId.id );
        glViewport( 0, 0, newFboId.viewportSize.x, newFboId.viewportSize.y );
    }
    else {
        stack.erase( std::next( entry ).base() );
    }
}

}
}