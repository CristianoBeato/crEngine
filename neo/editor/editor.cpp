
#include "idlib/precompiled.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "editor.hpp"

crEditor::crEditor( void )
{
}

crEditor::~crEditor( void )
{
}

int main( int argc, const char* argv[] )
{
    try
    {
        /* code */
    }
    catch(const idException& e)
    {
        SDL_Log( e.GetError() );
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}