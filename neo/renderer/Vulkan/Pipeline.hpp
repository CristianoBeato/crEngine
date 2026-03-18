/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos.

This file is part of the crEngine GPL Source Code ("crEngine Source Code").

crEngine Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

crEngine Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with crEngine Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#ifndef __VK_PIPELINE_HPP__
#define __VK_PIPELINE_HPP__

inline constexpr uint32_t VERTEX_BINDING = 0;
inline constexpr uint32_t VERTEX_ATTRIBUTE_POS = 0; // float32 XYZ 
inline constexpr uint32_t VERTEX_ATTRIBUTE_TEX = 1; // float16 ST
inline constexpr uint32_t VERTEX_ATTRIBUTE_NOR = 2; // unorm8 NX NY NZ NW 
inline constexpr uint32_t VERTEX_ATTRIBUTE_TAN = 3; // unorm8 TA TB TC TD
inline constexpr uint32_t VERTEX_ATTRIBUTE_JOI = 4; // uint 8 J0 J1 J2 J3
inline constexpr uint32_t VERTEX_ATTRIBUTE_WEI = 5; // unorm8 W0 W1 W2 W3

class vkPipeline
{
public:
    /// @brief pipeline creation state flags
    enum pipeline_state_e : uint64_t
    {
        PLS_DEFAULT = 0,

        /// @brief Face cull
        PLS_CULLFACE_BITS                   = 3ull << 0, // 0 and 1
        PLS_CULLFACE_TWO                    = 0ull << 0,
        PLS_CULLFACE_BACK                   = 1ull << 0, 
        PLS_CULLFACE_FRONT                  = 2ull << 0, 

        /// @brief  Blend source
        PLS_SRCBLEND_BITS					= 7ull << 2, // 2 to 4
        PLS_SRCBLEND_ONE					= 0ull << 2,
        PLS_SRCBLEND_ZERO					= 1ull << 2,
        PLS_SRCBLEND_DST_COLOR				= 2ull << 2,
        PLS_SRCBLEND_ONE_MINUS_DST_COLOR	= 3ull << 2,
        PLS_SRCBLEND_SRC_ALPHA				= 4ull << 2,
        PLS_SRCBLEND_ONE_MINUS_SRC_ALPHA	= 5ull << 2,
        PLS_SRCBLEND_DST_ALPHA				= 6ull << 2,
        PLS_SRCBLEND_ONE_MINUS_DST_ALPHA	= 7ull << 2,

        /// @brief  Blend destination
        PLS_DSTBLEND_BITS					= 7ull << 5, // 5 to 7
        PLS_DSTBLEND_ZERO					= 0ull << 5,
        PLS_DSTBLEND_ONE					= 1ull << 5,
        PLS_DSTBLEND_SRC_COLOR				= 2ull << 5,
        PLS_DSTBLEND_ONE_MINUS_SRC_COLOR	= 3ull << 5,
        PLS_DSTBLEND_SRC_ALPHA				= 4ull << 5,
        PLS_DSTBLEND_ONE_MINUS_SRC_ALPHA	= 5ull << 5,
        PLS_DSTBLEND_DST_ALPHA				= 6ull << 5,
        PLS_DSTBLEND_ONE_MINUS_DST_ALPHA	= 7ull << 5,

        /// @brief Blend operation
        PLS_BLENDOP_BITS					= 3ull << 8, // 8 and 9
        PLS_BLENDOP_ADD						= 0ull << 8,
        PLS_BLENDOP_SUB						= 1ull << 8,
        PLS_BLENDOP_MIN						= 2ull << 8,
        PLS_BLENDOP_MAX						= 3ull << 8,

        /// @brief Depth func
        PLS_DEPTHFUNC_BITS					= 3ull << 10, // 10 to 11
        PLS_DEPTHFUNC_ALWAYS				= 0ull << 10,
        PLS_DEPTHFUNC_LESS					= 1ull << 10,
        PLS_DEPTHFUNC_GREATER				= 2ull << 10,
        PLS_DEPTHFUNC_EQUAL					= 3ull << 10,

        /// @brief stencil function
        PLS_STENCIL_FUNC_BITS				= 7ull << 12, // 12 to 14
        PLS_STENCIL_FUNC_ALWAYS				= 0ull << 12,
        PLS_STENCIL_FUNC_LESS				= 1ull << 12,
        PLS_STENCIL_FUNC_LEQUAL				= 2ull << 12,
        PLS_STENCIL_FUNC_GREATER			= 3ull << 12,
        PLS_STENCIL_FUNC_GEQUAL				= 4ull << 12,
        PLS_STENCIL_FUNC_EQUAL				= 5ull << 12,
        PLS_STENCIL_FUNC_NOTEQUAL			= 6ull << 12,
        PLS_STENCIL_FUNC_NEVER				= 7ull << 12,

        /// @brief stencil fail operation
        PLS_STENCIL_OP_FAIL_BITS			= 7ull << 15, // 15 to 17
        PLS_STENCIL_OP_FAIL_KEEP			= 0ull << 15,
        PLS_STENCIL_OP_FAIL_ZERO			= 1ull << 15,
        PLS_STENCIL_OP_FAIL_REPLACE			= 2ull << 15,
        PLS_STENCIL_OP_FAIL_INCR			= 3ull << 15,
        PLS_STENCIL_OP_FAIL_DECR			= 4ull << 15,
        PLS_STENCIL_OP_FAIL_INVERT			= 5ull << 15,
        PLS_STENCIL_OP_FAIL_INCR_WRAP		= 6ull << 15,
        PLS_STENCIL_OP_FAIL_DECR_WRAP		= 7ull << 15,

        /// @brief stencil depth fail operation
        PLS_STENCIL_OP_ZFAIL_BITS			= 7ull << 18, // 18 to 20
        PLS_STENCIL_OP_ZFAIL_KEEP			= 0ull << 18, 
        PLS_STENCIL_OP_ZFAIL_ZERO			= 1ull << 18,
        PLS_STENCIL_OP_ZFAIL_REPLACE		= 2ull << 18,
        PLS_STENCIL_OP_ZFAIL_INCR			= 3ull << 18,
        PLS_STENCIL_OP_ZFAIL_DECR			= 4ull << 18,
        PLS_STENCIL_OP_ZFAIL_INVERT			= 5ull << 18,
        PLS_STENCIL_OP_ZFAIL_INCR_WRAP		= 6ull << 18,
        PLS_STENCIL_OP_ZFAIL_DECR_WRAP		= 7ull << 18,

        /// @brief stencil pass operation
        PLS_STENCIL_OP_PASS_BITS			= 7ull << 21, // 21 to 23
        PLS_STENCIL_OP_PASS_KEEP			= 0ull << 21, 
        PLS_STENCIL_OP_PASS_ZERO			= 1ull << 21,
        PLS_STENCIL_OP_PASS_REPLACE			= 2ull << 21,
        PLS_STENCIL_OP_PASS_INCR			= 3ull << 21,
        PLS_STENCIL_OP_PASS_DECR			= 4ull << 21,
        PLS_STENCIL_OP_PASS_INVERT			= 5ull << 21,
        PLS_STENCIL_OP_PASS_INCR_WRAP		= 6ull << 21,
        PLS_STENCIL_OP_PASS_DECR_WRAP		= 7ull << 21,

        PLS_STENCIL_OP_BITS					= PLS_STENCIL_OP_FAIL_BITS | PLS_STENCIL_OP_ZFAIL_BITS | PLS_STENCIL_OP_PASS_BITS,

        PLS_ALPHATEST_FUNC_BITS				= 3ull << 24, // 24 and 25
        PLS_ALPHATEST_FUNC_ALWAYS			= 0ull << 24,
        PLS_ALPHATEST_FUNC_LESS				= 1ull << 24,
        PLS_ALPHATEST_FUNC_GREATER			= 2ull << 24,
        PLS_ALPHATEST_FUNC_EQUAL			= 3ull << 24,

        /// @brief Channel mask
        PLS_DEPTHMASK						= 1ull << 26,
        PLS_REDMASK							= 1ull << 27,
        PLS_GREENMASK						= 1ull << 28,
        PLS_BLUEMASK						= 1ull << 29,
        PLS_ALPHAMASK						= 1ull << 30,
        PLS_COLORMASK						= ( PLS_REDMASK | PLS_GREENMASK | PLS_BLUEMASK | PLS_ALPHAMASK ),

        /// @brief Polygon mode
        PLS_POLYMODE_LINE					= 1ull << 31,
        PLS_POLYGON_OFFSET					= 1ull << 32,

        // stencil bits
        PLS_STENCIL_FUNC_REF_SHIFT			= 33,   //
        PLS_STENCIL_FUNC_REF_BITS			= 0xFFll << PLS_STENCIL_FUNC_REF_SHIFT,

        PLS_STENCIL_FUNC_MASK_SHIFT			= 41,   //
        PLS_STENCIL_FUNC_MASK_BITS			= 0xFFll << PLS_STENCIL_FUNC_MASK_SHIFT,

        PLS_COLOR_ATTACHAMENT               = 1ull << 49,
        PLS_NORMAL_ATTACHAMENT              = 1ull << 50,
        PLS_MATERIAL_ATTACHAMENT            = 1ull << 51,
        
        PLS_OVERRIDE						= 1ull << 63		// override the render prog state
    };

    vkPipeline( void );
    ~vkPipeline( void );
    bool    Create( const uint64_t m_flags, const vkProgramp in_vertexProgram, const vkProgramp in_fragmentProgram, const vkPipeline* in_reference );
    void    Destroy( void );
    
    void    Bind( void );

    bool operator==( const vkPipeline &p );
    const uint64_t Flags( void ) const { return m_flags; }
    const uint32_t VertexProgramID( void ) const { return m_vProgram != nullptr ? m_vProgram->ID() : UINT32_MAX; }
    const uint32_t FragmentProgramID( void ) const { return m_fProgram != nullptr ? m_fProgram->ID() : UINT32_MAX; }

    operator VkPipeline( void ) const { return m_pipeline; }
    
private:
    uint64_t    m_flags;
    VkPipeline  m_pipeline;
    vkProgramp  m_vProgram;
    vkProgramp  m_fProgram;
};

typedef vkPipeline* vkPipelinep;

#endif //!__VK_PIPELINE_HPP__