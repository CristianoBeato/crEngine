
#ifndef __MENU_HANDLER_PDA_HPP__
#define __MENU_HANDLER_PDA_HPP__

#include "MenuWidget_PDA_VideoInfo.hpp"
#include "MenuWidget_PDA_Objective.hpp"
#include "MenuWidget_PDA_UserData.hpp"
#include "MenuWidget_PDA_AudioFiles.hpp"
#include "MenuScreen_PDA_Inventory.hpp"
#include "MenuWidget_PDA_EmailInbox.hpp"
#include "MenuScreen_PDA_UserEmails.hpp"
#include "MenuScreen_PDA_VideoDisks.hpp"
#include "MenuScreen_PDA_UserData.hpp"

/*
================================================
idMenuHandler_PDA
================================================
*/
class idMenuHandler_PDA : public idMenuHandler
{
public:
	idMenuHandler_PDA() :
		audioLogPlaying( false ),
		videoPlaying( false ),
		audioFile( nullptr )
	{
	}
	virtual ~idMenuHandler_PDA();
	
	virtual void			Update();
	virtual void			ActivateMenu( bool show );
	virtual void			TriggerMenu();
	virtual void			Initialize( const char* swfFile, idSoundWorld* sw );
	virtual bool			HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual idMenuScreen* 	GetMenuScreen( int index );
	void					UpdateAudioLogPlaying( bool playing );
	void					UdpateVideoPlaying( bool playing );
	void					ClearVideoPlaying()
	{
		videoPlaying = false;
	}
	
	bool					PlayPDAAudioLog( int pdaIndex, int audioIndex );
	virtual void			Cleanup();
	
protected:

	bool							audioLogPlaying;
	bool							videoPlaying;
	idList< idList< idStr, TAG_IDLIB_LIST_MENU >, TAG_IDLIB_LIST_MENU >		pdaNames;
	idList< idStr, TAG_IDLIB_LIST_MENU >					navOptions;
	const idDeclAudio* 				audioFile;
	idMenuWidget_ScrollBar pdaScrollBar;
	idMenuWidget_DynamicList pdaList;
	idMenuWidget_NavBar navBar;
	idMenuWidget_CommandBar commandBarWidget;
};

#endif //!__MENU_HANDLER_PDA_HPP__