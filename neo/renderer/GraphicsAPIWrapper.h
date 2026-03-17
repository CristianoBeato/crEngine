/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#ifndef __GRAPHICSAPIWRAPPER_H__
#define __GRAPHICSAPIWRAPPER_H__

/*
================================================================================================

	Graphics API wrapper/helper functions

	This wraps platform specific graphics API functionality that is used at run-time. This
	functionality is wrapped to avoid excessive conditional compilation and/or code duplication
	throughout the run-time rendering code that is shared on all platforms.

	Most other graphics API functions are called for initialization purposes and are called
	directly from platform specific code implemented in files in the platform specific folders:

	renderer/OpenGL/
	renderer/Vulkan/

================================================================================================
*/

class idImage;
//class idTriangles;
class idRenderModelSurface;
class idDeclRenderProg;
class idRenderTexture;

inline constexpr int MAX_OCCLUSION_QUERIES = 4096;
// returned by GL_GetDeferredQueryResult() when the query is from too long ago and the result is no longer available
inline constexpr int OCCLUSION_QUERY_TOO_OLD				= -1;

inline constexpr int MAX_MULTITEXTURE_UNITS = 8;
inline constexpr uint32_t MAX_UNIFORM_BLOCKS = 4086; // these are the maximum entities by draw call 
inline constexpr uint32_t MAX_LIGHT_BLOCKS = 2048;

struct float4
{
    float x = 0.0;
    float y = 0.0; 
    float z = 0.0;
    float w = 0.0;
};

struct int4
{
    int x = 0;
    int y = 0;
    int z = 0;
    int w = 0;
};

struct alignas( 16 ) textureLocationBlock_t
{
    uint32_t samplersLocation[MAX_MULTITEXTURE_UNITS];
};

/*
================================================================================================

	Platform Specific Context

================================================================================================
*/

#define USE_CORE_PROFILE

struct wrapperContext_t
{
};


/*
================================================
wrapperConfig_t
================================================
*/
struct wrapperConfig_t
{
	// rendering options and settings
	bool			disableStateCaching;
	bool			lazyBindPrograms;
	bool			lazyBindParms;
	bool			lazyBindTextures;
	bool			stripFragmentBranches;
	bool			skipDetailTris;
	bool			singleTriangle;
	// values for polygon offset
	float			polyOfsFactor;
	float			polyOfsUnits;
	// global texture filter settings
	int				textureMinFilter;
	int				textureMaxFilter;
	int				textureMipFilter;
	float			textureAnisotropy;
	float			textureLODBias;
};

/*
================================================
wrapperStats_t
================================================
*/
struct wrapperStats_t
{
	int				c_queriesIssued;
	int				c_queriesPassed;
	int				c_queriesWaitTime;
	int				c_queriesTooOld;
	int				c_programsBound;
	int				c_drawElements;
	int				c_drawIndices;
	int				c_drawVertices;
};

/*
================================================================================================

	API

================================================================================================
*/

void			GL_SetWrapperContext( const wrapperContext_t& context );
void			GL_SetWrapperConfig( const wrapperConfig_t& config );
void			GL_SetTimeDelta( uint64_t delta );	// delta from GPU to CPU microseconds
void			GL_StartFrame( int frame );			// inserts a timing mark for the start of the GPU frame
void			GL_EndFrame();						// inserts a timing mark for the end of the GPU frame
void			GL_WaitForEndFrame();				// wait for the GPU to reach the last end frame marker
void			GL_GetLastFrameTime( uint64_t& startGPUTimeMicroSec, uint64_t& endGPUTimeMicroSec );	// GPU time between GL_StartFrame() and GL_EndFrame()
void			GL_GetDepthPassRect( idScreenRect& rect );

void			GL_Flush();		// flush the GPU command buffer
void			GL_Finish();	// wait for the GPU to have executed all commands
void			GL_CheckErrors_Extended(const char* file, int line);
#define GL_CheckErrors() GL_CheckErrors_Extended( __FILE__,  __LINE__)

wrapperStats_t	GL_GetCurrentStats();
void			GL_ClearStats();

#if 0
// BEATO Begin:

/// crBuffer
/// @brief base class abstraction for common graphic buffer storage
///
class crBuffer : public crResourceState
{
public:
    enum access_t : uint8_t
    {
        BUFFER_ACCESS_NONE,
        BUFFER_ACCESS_WRITE,
        BUFFER_ACCESS_READ
    };

    enum type_t : uint8_t
    {
        BUFFER_TYPE_UNDEFINED,  // unknow buffer
        BUFFER_TYPE_INDEX,      // primitive index buffer 
        BUFFER_TYPE_VERTEX,     // vertex buffer
        BUFFER_TYPE_SHADER,     // shader storage data
        BUFFER_TYPE_COMMANDS,   // indirec draw comand buffer
        BUFFER_TYPE_PIXEL       // pixel storage buffer
    };

    crBuffer( void );
    ~crBuffer( void );

    /// @brief create a fixed size buffer storage, data will remain maped till buffer are Destroyeds
    /// @param in_type  the buffer storage usage type 
    /// @param in_acess buffer acess for read or write 
    /// @param in_size buffer size
    /// @return true on sucess 
    virtual bool    Create( const type_t in_type, const access_t in_acess, const size_t in_size ) = 0;
    
    /// @brief try recreate the buffer and copy old content from old buffer
    /// @param in_newSize new buffer size 
    /// @return true on sucess
    virtual bool    Resize( const size_t in_newSize ) = 0;
    
    /// @brief Destroy the buffer releasing the map and the memory 
    virtual void    Destroy( void ) = 0;
    
    /// @brief Copy the content of the source buffer to this 
    /// @param in_source the read source buffer 
    /// @param in_srcOffset the read source offset 
    /// @param in_dstOffset the write offset 
    /// @param in_size the ammount of bytes to by copy
    virtual void    CopyBuffer( const crBuffer* in_source, const uintptr_t in_srcOffset, const uintptr_t in_dstOffset, const size_t in_size ) const = 0;
    
    /// @brief flush buffer data ( send to device )
    /// @param in_offset 
    /// @param in_size 
    virtual void    Flush( const uintptr_t in_offset, const size_t in_size ) const = 0;
    
    /// @brief Send data to buffer, perform a cpu copy of the data to the dst buffer 
    /// @param in_data 
    /// @param in_offset 
    /// @param in_size 
    virtual void    Upload( const void* in_data, const uintptr_t in_offset, const size_t in_size ) const;
    
    /// @brief 
    /// @param in_data 
    /// @param in_offset 
    /// @param in_size 
    virtual void    Download( void* in_data, const uintptr_t in_offset, const size_t in_size ) const;
    
    /// @brief 
    /// @param  
    /// @return 
    virtual void*   Handle( void ) const = 0;
    
    type_t      Type( void ) const { return m_type; }
    size_t      Size( void ) const { return m_size; }
    void*       Map( void ) const { return m_data; }
    
protected:
    type_t      m_type;     // type of buffer data storage
    access_t    m_access;   // buffer access type
    size_t      m_size;     // bufer whole size
    void*       m_data;     // pointer from buffer mapped data
};

class crFramebuffer
{
public:
    struct Attachament_t
    {
        uint32_t    buffer;
        uint32_t    colorAttachament;
        uint32_t    depthStencil;
        crTexture*  attachament;
    };
   
    crFramebuffer( void );
    ~crFramebuffer( void );

    virtual bool Create(    const uint32_t in_bufferCount,
                            const uint32_t in_width,
                            const uint32_t in_height,
                            const Attachament_t* in_attachaments, 
                            const uint32_t in_numAttachaments );

    virtual bool    Resize( const uint32_t in_width, const uint32_t in_height ) = 0;
    
    virtual void    Destroy( void ) = 0;

    virtual void*   Handle( void ) const = 0;

    const uint32_t  Width( void ) const { return m_width; }
    const uint32_t  Height( void ) const { return m_height; }

protected:
    uint32_t m_width;
    uint32_t m_height;
};

/// @brief this is a wrapper/workarround to implemment pipelines 
class crPipeline
{
public:
    enum Face_t : uint8_t
    {
        FC_BACK,
        FC_FRONT,
        FC_TWO_FACES
    };

    enum PolygonMode_t : uint8_t
    {
        PM_POINT,   // render as point
        PM_LINE,    // render as line
        PM_FILL     // render as filled triangle
    };

    enum DepthFunc_t : uint8_t
    {
        DF_NONE,
        DF_ALWAYS,  // aways pass 
        DF_LESS,    // pass if less
        DF_GREATER, // pass if grater
        DF_EQUAL,   // pass if equal
    };

    enum ColorMask_t : uint8_t
    {
        CM_RED_MASK     = 1 << 1,
        CM_GREEN_MASK   = 1 << 2,
        CM_BLUE_MASK    = 1 << 3,
        CM_ALPHA_MASK   = 1 << 4,
    };

    enum BlendSource_t : uint8_t
    {
        BLEND_SRC_ONE,
        BLEND_SRC_ZERO,
        BLEND_SRC_DST_COLOR,
        BLEND_SRC_ONE_MINUS_DST_COLOR,
        BLEND_SRC_SRC_ALPHA,
        BLEND_SRC_ONE_MINUS_SRC_ALPHA,
        BLEND_SRC_DST_ALPHA,
        BLEND_SRC_ONE_MINUS_DST_ALPHA,
    };

    enum BlendDestination_t : uint8_t
    {
        BLEND_DST_ZERO,
        BLEND_DST_ONE,
        BLEND_DST_SRC_COLOR,
        BLEND_DST_ONE_MINUS_SRC_COLOR,
        BLEND_DST_SRC_ALPHA,
        BLEND_DST_ONE_MINUS_SRC_ALPHA,
        BLEND_DST_DST_ALPHA,
        BLEND_DST_ONE_MINUS_DST_ALPHA,
    };

    enum BlendOperation_t : uint8_t
    {
        BLEND_OP_ADD,
        BLEND_OP_SUB,
        BLEND_OP_MIN,
        BLEND_OP_MAX,
    };

    enum StencilFunc_t : uint8_t
    {
        STENCIL_FUNC_ALWAYS,
        STENCIL_FUNC_LESS,
        STENCIL_FUNC_LEQUAL,
        STENCIL_FUNC_GREATER,
        STENCIL_FUNC_GEQUAL,
        STENCIL_FUNC_EQUAL,
        STENCIL_FUNC_NOTEQUAL,
        STENCIL_FUNC_NEVER
    };

    enum StencilOperation_t : uint8_t
    {
        STENCIL_OP_KEEP,
        STENCIL_OP_ZERO,
        STENCIL_OP_REPLACE,
        STENCIL_OP_INCR,
        STENCIL_OP_DECR,
        STENCIL_OP_INVERT,
        STENCIL_OP_INCR_WRAP,
        STENCIL_OP_DECR_WRAP
    };

    enum AlphaFunc_t : uint8_t 
    {
        ALPHATEST_FUNC_ALWAYS,
        ALPHATEST_FUNC_LESS,
        ALPHATEST_FUNC_GREATER,
        ALPHATEST_FUNC_EQUAL
    };

    struct PipelineInfo_t
    {
        Face_t                  faceCull;
        Face_t                  polygonModeFace;
        PolygonMode_t           polygonMode;
        DepthFunc_t             depthFunc;
        uint8_t                 colorMask;
        BlendSource_t           blendSource;
        BlendDestination_t      blendDestination;
        BlendOperation_t        blendOperation;
        Face_t                  stencilFace;
        StencilOperation_t      stencilPass;
        StencilOperation_t      stencilFail;
        StencilOperation_t      stencilZFail;
        AlphaFunc_t             alphaFunc;
        uint32_t                numPrograms;
        crProgram**             shaderPrograms;

        bool operator==(const PipelineKey_s &o) const = default;
    };
    
    crPipeline( void );
    ~crPipeline( void );

    virtual bool    Create( const PipelineInfo_t in_pipelineInfo ) = 0;
    virtual void    Destroy( void ) = 0;

    PipelineInfo_t  PipelineInfo( void ) const { return m_pipelineConfiguration; }

protected:
    PipelineInfo_t  m_pipelineConfiguration;
};

class crCommandBuffer
{
public:
    /// @brief Begin command recording for this frame 
    virtual void    Begin( void ) = 0;
    
    /// @brief End command recording 
    virtual void    End( void ) = 0;
    
    /// @brief Submit command queue, and swap buffer
    virtual void    Submit( void ) = 0;
};

/// @brief Command buffer dedicate to transfer operations
class crTransferCommandBuffer : public crCommandBuffer
{
public:
    crTransferCommandBuffer( void );
    ~crTransferCommandBuffer( void );

    /// @brief Copy data texture from texture
    virtual void    CopyTexture( const crTexture* in_src, const crTexture* in_dst, const idList<crTexture::subImage_t> in_subImages ) = 0;
    
    /// @brief Upload texture from buffer
    virtual void    CopyBufferToTexture( const crBuffer* in_buffer, const crTexture* in_texture, const idList<crTexture::subImage_t> in_subImages ) = 0;
    
    /// @brief Download texture to buffer 
    virtual void    CopyTextureToBuffer( const crBuffer* in_buffer, const crTexture* in_texture, const idList<crTexture::subImage_t> in_subImages ) = 0;

    /// @brief Made a copy from source buffer to destination buffer
    virtual void    CopyBuffer( const crBuffer* in_srcBuffer, const crBuffer* in_dstBuffer, const uintptr_t in_offset, const size_t in_size ) = 0;
};

/// @brief Graphic command buffer
class crGraphicCommandBuffer : public crCommandBuffer
{
public:
    crGraphicCommandBuffer( void );
    ~crGraphicCommandBuffer( void );
    virtual void    LineWidth( const float in_lineWidth ) const = 0;
    virtual void    BindFrameBuffer( const crFramebuffer* in_framebuffef ) = 0;
    virtual void    BindIndexBuffer( const crBuffer* in_buffer ) = 0;
    virtual void    BindVertexBuffers( const crBuffer* in_buffer, uint32_t in_binding, const uintptr_t in_offsets, const size_t in_sizes, const size_t in_strides ) = 0;
    virtual void    BindPipeline( const crPipeline* in_pipeline ) = 0;
    virtual void    EndRenderPass( void ) const = 0;
    virtual void    Draw(  const uint32_t in_vertexCount, const uint32_t in_instanceCount, const uint32_t in_firstVertex, const uint32_t in_firstInstance ) const = 0;
    virtual void    DrawIndexed( const uint32_t in_indexCount, const uint32_t in_instanceCount, const uint32_t in_firstIndex, const int32_t in_vertexOffset, const uint32_t in_firstInstance ) const = 0;
    virtual void    Dispatch(  const uint32_t in_groupCountX, const uint32_t in_groupCountY, const uint32_t in_groupCountZ ) const = 0;
    virtual void    Clear( bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a ) = 0;
    virtual void    PolygonOffset( const float scale, const float bias, const bool enable ) = 0;
    virtual void    DepthBoundsTest( const float zmin, const float zmax, const bool enable ) = 0;
    virtual void    FaceCull( const crPipeline::Face_t in_cullType ) = 0;
    virtual void    Scissor( const int x, const int y, const int w, const int h ) const = 0;
    virtual void    Viewport( const int x, const int y, const int w, const int h ) const = 0;

    ID_INLINE void  Scissor( const idScreenRect& rect ) const;
    ID_INLINE void  Viewport( const idScreenRect& rect ) const;
    ID_INLINE void  ViewportAndScissor( const idScreenRect& rect ) const;
};


ID_INLINE void crGraphicCommandBuffer::Scissor( const idScreenRect& rect ) const
{
	Scissor( rect.x1, rect.y1, rect.x2 - rect.x1 + 1, rect.y2 - rect.y1 + 1 );
}

ID_INLINE void	crGraphicCommandBuffer::Viewport( const idScreenRect& rect ) const
{
	Viewport( rect.x1, rect.y1, rect.x2 - rect.x1 + 1, rect.y2 - rect.y1 + 1 );
}

ID_INLINE void	crGraphicCommandBuffer::ViewportAndScissor( const idScreenRect& rect ) const
{
	Viewport( rect );
	Scissor( rect );
}

class crSwapchain
{    
public:
    crSwapchain( const uint32_t in_width, const uint32_t in_height );
    ~crSwapchain( void );
    virtual bool    Recreate( const uint32_t in_width, const uint32_t in_height ) = 0;
    virtual void    AcquireImage( void ) = 0;
    virtual void    PresentImage( void ) = 0;

protected:
    uint32_t    m_frame;        // current frame
    uint32_t    m_width; 
    uint32_t    m_height;
};

///
///
class crSamplerSlot
{
public:
	crSamplerSlot( void ) : m_index( 0xFFFFFFFF ) {}

    void    SetIndex( const uint32_t in_index ) { m_index = in_index; }
    uint32_t GetIndex( void ) const { return m_index; }

    ///
    operator uint32_t( void ) const { return m_index; }  

private:
    uint32_t m_index;  // logic index 
};

///
///
class crBindlessSampleStorage
{
public:
    virtual crSamplerSlot*  BindTexture( const crTexture* in_texture, const crSampler* in_sampler ) = 0;
    virtual void            FreeSlot( crSamplerSlot* &in_handle ) = 0;
    
protected:
    uint32_t                m_lastTextureIndex;
    idList<crSamplerSlot>   m_freeList;
}

/// @brief 
class crShaderStorage
{
public:
    crShaderStorage( void );
    ~crShaderStorage( void );
    virtual void                    Submit( void );
    virtual void                    SubmitLight( void );
    virtual void                    Begin( void );
    virtual void                    End( void );
    vertexUniformBlock_t*           GetvertexUniformBlock( void ) { return &m_vertexUniformBlock; };
    fragmentUniformBlock_t*         GetFragmentUniformBlock( void ) { return &m_fragmentUniformBlock; };
    lightUnifomBlock_t*             GetLightUnifomBlock( void ) { return &m_lightUnifomBlock; }
    textureLocationBlock_t*         GetTextureLocationBlock( void ) { return &m_textureLocationBlock; }

protected:
    uint32_t                m_frame;
    uint32_t                m_currentVBlock;
    uint32_t                m_currentFSBlock;
    uint32_t                m_currentLSBlock;
    crBuffer*               m_VTSSBO;
    crBuffer*               m_FGSSBO;
    crBuffer*               m_LHSSBO;
    crBuffer*               m_TLSSBO;
    vertexUniformBlock_t    m_vertexUniformBlock;
    fragmentUniformBlock_t  m_fragmentUniformBlock;  
    lightUnifomBlock_t      m_lightUnifomBlock;      
    textureLocationBlock_t  m_textureLocationBlock;
};
// BEATO End
#endif

#endif // !__GRAPHICSAPIWRAPPER_H__
