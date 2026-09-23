
#ifndef __SCRIPT_EDITOR_HPP__
#define __SCRIPT_EDITOR_HPP__

#include <SDL3/SDL_events.h>
#include "ScriptEditorMainDialog.hpp"

typedef struct scriptEventInfo_s 
{
	idStr		name;
	idStr		parms;
	idStr		help;
} scriptEventInfo_t;

class crScriptEditorMain
{
private:
    crScriptEditorMain( void ) = delete;
    ~crScriptEditorMain( void ) = delete;
    
    static idList<scriptEventInfo_t>    m_scriptEvents;
    static crScriptEditorMainDialog*    m_scriptEditorMainDialog;
public:
    /// @brief Open the script editor dialog
    /// @param spawnArgs 
    static void Init( const idDict *spawnArgs );

    /// @brief Close script editor dialog 
    /// @param  
    static void Shutdown( void );

    /// @brief Execute script editor dialog
    /// @param in_evt SDL_Event 
    static bool Run( const SDL_Event &in_evt );
};

#endif //!__SCRIPT_EDITOR_HPP__