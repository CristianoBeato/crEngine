
#ifndef __DIALOG_COMMON_HPP__
#define __DIALOG_COMMON_HPP__

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include "Gwen/Controls.h"
#include "Gwen/Input/SDL3.h"

class crToolsDialogCommon : public Gwen::Controls::Canvas
{
public:
    crToolsDialogCommon( void );
    ~crToolsDialogCommon( void );

    void    Create( void );
    void    Destroy( void );
    void    Render( void );
    bool    Update( const SDL_Event *in_evt );

private:
    SDL_WindowID                 m_dialogWindowID;
    SDL_Window*                  m_dialogWindow;   // Script Editor window
    SDL_GLContext                m_renderContext;  // Render Context
    Gwen::Input::SDL3            m_eventManager;
};

#endif //!__DIALOG_COMMON_HPP__