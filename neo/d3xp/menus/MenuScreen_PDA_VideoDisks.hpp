#ifndef __MENU_SCREEN_PDA_VIDEO_DISKS_HPP__
#define __MENU_SCREEN_PDA_VIDEO_DISKS_HPP__

/*
================================================
idMenuScreen_PDA_UserData
================================================
*/
class idMenuScreen_PDA_VideoDisks : public idMenuScreen
{
public:
	idMenuScreen_PDA_VideoDisks() :
		activeVideo( nullptr )
	{
	}
	
	virtual ~idMenuScreen_PDA_VideoDisks()
	{
	}
	
	virtual void				Initialize( idMenuHandler* data );
	virtual void				Update();
	virtual void				ShowScreen( const mainMenuTransition_t transitionType );
	virtual void				HideScreen( const mainMenuTransition_t transitionType );
	virtual bool				HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	
	void						ToggleVideoDiskPlay();
	void						UpdateVideoDetails();
	void						SelectedVideoToPlay( int index );
	void						ClearActiveVideo()
	{
		activeVideo = nullptr;
	}
	const idDeclVideo* 			GetActiveVideo()
	{
		return activeVideo;
	}
private:
	idMenuWidget_ScrollBar		scrollbar;
	idMenuWidget_DynamicList 		pdaVideoList;
	idMenuWidget_PDA_VideoInfo 	videoDetails;
	idList< idList< idStr, TAG_IDLIB_LIST_MENU>, TAG_IDLIB_LIST_MENU >		videoItems;
	const idDeclVideo* 				activeVideo;
};

#endif //!__MENU_SCREEN_PDA_VIDEO_DISKS_HPP__