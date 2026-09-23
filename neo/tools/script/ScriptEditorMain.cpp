
#include "precompiled.h"
#include "ScriptEditorMain.hpp"

idList<scriptEventInfo_t>    crScriptEditorMain::m_scriptEvents;
crScriptEditorMainDialog*    crScriptEditorMain::m_scriptEditorMainDialog;

void crScriptEditorMain::Init(const idDict *spawnArgs)
{
    if ( idRenderSystem::Get()->IsFullScreen() ) 
	{
		common->Printf( "Cannot run the script editor in fullscreen mode.\n" "Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

    m_scriptEditorMainDialog = new crScriptEditorMainDialog();
    m_scriptEditorMainDialog->Create();
}

void crScriptEditorMain::Shutdown(void)
{
    if( m_scriptEditorMainDialog )
    {
        m_scriptEditorMainDialog->Destroy();
        delete m_scriptEditorMainDialog;
        m_scriptEditorMainDialog = nullptr;
    }
}

bool crScriptEditorMain::Run( const SDL_Event &in_evt )
{    
    if( !m_scriptEditorMainDialog )
        return false;

    // Its a valid event 
    return m_scriptEditorMainDialog->Update( in_evt );
}
