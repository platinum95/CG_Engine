#ifndef CG_DATA_H
#define CG_DATA_H

#include "CG_Assert.h"
#include "Common.h"
#include "File_IO.h"
#include "glad.h"
#include "IRenderable.h"
#include "ScopedToken.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/vec2.hpp>

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

namespace GL_Engine {
namespace CG_Data {
static constexpr GLuint InvalidGlId = -1;// std::numeric_limits<GLuint>::max();

/*-------------VBO Class------------*/
/*
*Handles everything to do with VBOs
*/
class VBO {
public:
    VBO();

    //Construct a VBO by passing data, data size(in bytes), usage (i.e GL_STATIC_DRAW), and an optional target (i.e. GL_ARRAY_BUFFER)
    VBO( void *_Data, uint64_t _DataSize, GLenum _Usage, GLenum _Target = GL_ARRAY_BUFFER );

    ~VBO();

    void Cleanup();

    //Returns the ID of the VBO
    const GLuint GetID() const;

    //Binds the VBO
    void BindVBO() const;

    //Sets the VBO data
    void SetVBOData( const void *_Data, uint64_t _DataSize ) const;

protected:
    //The VBO target, e.g. GL_ARRAY_BUFFER
    GLenum Target{ GL_ARRAY_BUFFER };

    //The VBO usage, e.g. GL_STATIC_DRAW
    GLenum Usage{ GL_STATIC_DRAW };

    //The VBO ID
    GLuint ID{ InvalidGlId };
private:
    //Indicates whether or not the VBO has been initialised
    bool initialised{ false };
};

/*-------------VAO Class------------*/
/*
*Handles everything to do with VAOs
*Keeps track of the VBOs assigned to the VAO
*/
class VAO {
public:
    VAO();
    ~VAO();
    void Cleanup();
    const GLuint GetID() const;
    void BindVAO() const;
    void AddVBO( std::unique_ptr<VBO> _VBO );
    GLuint getIndexCount() const;
protected:
    std::vector<std::unique_ptr<VBO>> VBOs;
    GLuint numIndices{ 0 };
private:
    GLuint VAOId{ InvalidGlId };
    bool initialised{ false };
};


/*-------------Texture Class------------*/
/*
*Handles everything to do with textures
*/
class Texture {
public:
    Texture( void *_Data, GLint width, GLint height, GLuint _Unit, GLuint _ImageFormat, std::function<void()> _Parameters, GLenum _Target = GL_TEXTURE_2D );
    Texture( GLuint _Unit, GLenum _Target );
    Texture( GLuint _Unit, GLenum _Target, std::function<void()> _Parameters );
    Texture( GLuint _id, GLuint _unit, GLenum _target );
    ~Texture();
    void Cleanup();

    void SetUnit( const GLuint _Unit );

    void Bind() const;

    const GLuint GetID() const;
    GLuint ID{ InvalidGlId };
protected:
    GLenum Target{ GL_ARRAY_BUFFER };
    GLuint Unit{ GL_TEXTURE0 };
private:
    bool Initialised{ false };
};



/*-------------Uniform Class------------*/
/*
*Handles everything to do with shader uniforms
*/
class Uniform {
public:
    Uniform( GLint _Location, void *_Data, std::function<void(const CG_Data::Uniform&)> _Callback );
    Uniform();
    ~Uniform();
    void Cleanup();

    void Update() const;
    void SetUpdateCallback( std::function<void(const CG_Data::Uniform&)> _callback );
    const GLint GetID() const;
    void SetData( const void *_Data );
    void SetID( GLint _ID );
    const void* GetData() const;

private:
    bool NeedsUpdating{ false };
    bool Initialised{ false };
    const void *Data{ nullptr };
    GLint ID;
    std::function<void(const CG_Data::Uniform&)> UpdateCallback;
};

/*-------------UBO Class------------*/
/*
*Handles everything to do with uniform buffer objects
*/
class UBO : VBO {
public:
    UBO( const void *_Data, size_t _DataSize );

    template <typename T>
    UBO( const T *data ) : UBO( static_cast<const void*>( data ), sizeof( T ) ) {}

    ~UBO();
    void UpdateUBO() const;
    const GLuint GetBindingPost() const;
    void setData( const void *_data );
private:
    bool Initialised{ false };
    const void *Data;
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
    // TODO - move to a component

    struct FramebufferBindData {
        GLuint id{ InvalidGlId };
        glm::vec<2, uint16_t> viewportSize{ 0, 0 };
        bool operator==( const FramebufferBindData& ) const = default;
    };

    static std::list<FramebufferBindData> FramebufferStack;
    static std::list<FramebufferBindData> ReadFramebufferStack;
    static std::list<FramebufferBindData> DrawFramebufferStack;

    static void staticUnbind( FramebufferBindData &id, std::list<FramebufferBindData> &stack );

    template<GLenum fb = GL_FRAMEBUFFER>
    static void staticUnbind( FramebufferBindData &id ) {
        if constexpr ( fb == GL_FRAMEBUFFER ) {
            staticUnbind( id, FramebufferStack );
        }
        else if constexpr ( fb == GL_DRAW_FRAMEBUFFER ) {
            staticUnbind( id, DrawFramebufferStack );
        }
        else if constexpr ( fb == GL_READ_FRAMEBUFFER ) {
            staticUnbind( id, ReadFramebufferStack );
        }
    }

    template<GLenum fb>
    using FramebufferBindToken = ScopedToken<FramebufferBindData, FBO::staticUnbind<fb>>;

    template<GLenum fb>
    static FramebufferBindToken<fb> staticBind( FramebufferBindData &&data, std::list<FramebufferBindData> &stack ){
        glBindFramebuffer( fb, data.id );
        stack.push_back( data );
        return FramebufferBindToken<fb>( std::move( data ) );
    }

    template<GLenum fb=GL_FRAMEBUFFER>
    static FramebufferBindToken<fb> staticBind( FramebufferBindData &&id ) {
        if constexpr ( fb == GL_FRAMEBUFFER ) {
            return staticBind<GL_FRAMEBUFFER>( std::move( id ), FramebufferStack );
        }
        else if constexpr ( fb == GL_DRAW_FRAMEBUFFER ) {
            return staticBind<GL_DRAW_FRAMEBUFFER>( std::move( id ), DrawFramebufferStack );
        }
        else if constexpr ( fb == GL_READ_FRAMEBUFFER ) {
            return staticBind<GL_READ_FRAMEBUFFER>( std::move( id ), ReadFramebufferStack );
        }
    }

    class AttachmentBufferObject {
    public:
        GLuint ID;
        virtual void bind() const = 0;
    };

    class RenderbufferObject : public AttachmentBufferObject {
    public:
        RenderbufferObject( uint16_t _Width, uint16_t _Height, GLenum _Type );
        void bind() const;
    };

    class TexturebufferObject : public AttachmentBufferObject {
    public:
        TexturebufferObject( uint16_t _Width, uint16_t _Height, uint8_t _Unit );
        TexturebufferObject( std::shared_ptr<Texture> _tex );

        void bind() const;
        const std::shared_ptr<Texture> GetTexture() const;

    private:
        std::shared_ptr<Texture> TextureObject;
    };

    enum AttachmentType {
        ColourRenderbuffer,
        StencilRenderbuffer,
        DepthRenderbuffer,
        ColourTexture,
        DepthTexture
    };

    FBO( uint16_t _width, uint16_t _height );
    ~FBO();
    void cleanup();

    std::shared_ptr<AttachmentBufferObject> addAttachment( AttachmentType _attachment, uint16_t _width, uint16_t _height );

    template<GLenum fb = GL_FRAMEBUFFER>
    FBO::FramebufferBindToken<fb> bind( uint8_t _ColourAttachment = 0 ) const {
        cg_assert( this->complete ); //"Attempting to bind incomplete framebuffer!\n"
        if constexpr ( fb == GL_READ_FRAMEBUFFER ) {
            return bindRead();
        }
        else {
            GLenum attachment = GL_COLOR_ATTACHMENT0 + _ColourAttachment;
            return bind<fb>( 1, &attachment );
        }
    }

    template<GLenum fb = GL_FRAMEBUFFER>
    FBO::FramebufferBindToken<fb> bind( uint16_t _Count, const GLenum *_ColourAttachments ) const {
        cg_assert( this->complete );

        glBindTexture( GL_TEXTURE_2D, 0 );
        auto bindToken = staticBind<fb>( { this->ID, glm::vec2( this->width, this->height ) } );
        glClearColor( 0.0, 0.0, 0.0, 1.0 );
        for ( int i = 0; i < _Count; i++ ) {
            glDrawBuffer( _ColourAttachments[i] );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
        }
        glDrawBuffers( _Count, _ColourAttachments );
        glViewport( 0, 0, this->width, this->height );

        return bindToken;
    }

    FBO::FramebufferBindToken<GL_READ_FRAMEBUFFER> bindRead() const {
        cg_assert( this->complete );
        return staticBind<GL_READ_FRAMEBUFFER>( { this->ID, glm::vec2( this->width, this->height ) } );
    }

    const GLuint getID() const;
    void unbind() const;

public:
    uint16_t width, height;

protected:
    GLuint ID;

private:
    bool initialised{ false }, complete{ false };
    uint8_t textureAttachmentCount{ 0 };
    std::vector<std::shared_ptr<AttachmentBufferObject>> attachments;
};

class FboRenderNodeBase;

template<auto func>
class FboRenderable : public IRenderable {
public:
    FboRenderable( std::shared_ptr<FboRenderNodeBase> base )
        : m_base( base ) {}

    void execute() {
        (m_base.get()->*func)();
    }

private:
    std::shared_ptr<FboRenderNodeBase> m_base;
};

class FboRenderNodeBase {
public:
    FboRenderNodeBase( std::shared_ptr<FBO> fbo )
        : m_fbo( fbo ) {}

    static std::shared_ptr<IRenderable> GenerateReadRenderNode( std::shared_ptr<FboRenderNodeBase> base ) {
        return std::make_shared<FboRenderable<&FboRenderNodeBase::executeRead>>( std::move( base ) );
    }

    static std::shared_ptr<IRenderable> GenerateDrawRenderNode( std::shared_ptr<FboRenderNodeBase> base ) {
        return std::make_shared<FboRenderable<&FboRenderNodeBase::executeRenderWrite>>( std::move( base ) );
    }

    static std::shared_ptr<IRenderable> GenerateBlitRenderNode( std::shared_ptr<FboRenderNodeBase> base ) {
        return std::make_shared<FboRenderable<&FboRenderNodeBase::executeBlitWrite>>( std::move( base ) );
    }

private:
    void executeRenderWrite() {
        UsingScopedToken( m_fbo->bind<GL_FRAMEBUFFER>() ) {
            writeTarget->execute();
        }
    }

    void executeRead() {
        UsingScopedToken( m_fbo->bind<GL_READ_FRAMEBUFFER>() ) {
            readTarget->execute();
        }
    }

    void executeBlitWrite() {
        UsingScopedToken( m_fbo->bind<GL_DRAW_FRAMEBUFFER>() ) {
            writeTarget->execute();
        }
    }

public:
    std::shared_ptr<IRenderable> writeTarget, readTarget;
public:
    std::shared_ptr<FBO> m_fbo;
};

} // namespace CG_Data
} // namespace GL_Engine

#endif // CG_DATA_H