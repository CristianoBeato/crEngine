#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

namespace gui
{
    class crRenderer
    {
    public:
        struct geometry_t
        {
            uint32_t    cacheID;    // draw command id 
            uint32_t    textureID;  // draw texture id
        };

        crAutoPointer<crRenderer, TAG_IDLIB>    crRendererPointer;

        crRenderer( void );
        ~crRenderer( void );

    private:

    };
};

#endif //!__RENDERER_HPP__