
#ifndef __MENU_SCREEN_PDA_USER_DATA_HPP__
#define __MENU_SCREEN_PDA_USER_DATA_HPP__

/*
================================================
idMenuScreen_PDA_UserData
================================================
*/
class idMenuScreen_PDA_UserData : public idMenuScreen
{
public:

	idMenuScreen_PDA_UserData() {}
	
	virtual ~idMenuScreen_PDA_UserData() {}
	virtual void					Initialize( idMenuHandler* data );
	virtual void					Update();
	virtual void					ShowScreen( const mainMenuTransition_t transitionType );
	virtual void					HideScreen( const mainMenuTransition_t transitionType );
	virtual bool					HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	idMenuWidget_PDA_UserData* 		GetUserData()
	{
		return &pdaUserData;
	}
	idMenuWidget_PDA_Objective* 	GetObjective()
	{
		return &pdaObjectiveSimple;
	}
	idMenuWidget_PDA_AudioFiles* 	GetAudioFiles()
	{
		return &pdaAudioFiles;
	}
	
private:
	idMenuWidget_PDA_UserData 			pdaUserData;
	idMenuWidget_PDA_Objective 		pdaObjectiveSimple;
	idMenuWidget_PDA_AudioFiles 		pdaAudioFiles;
};

#endif //__MENU_SCREEN_PDA_USER_DATA_HPP__