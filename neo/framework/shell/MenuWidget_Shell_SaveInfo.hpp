
#ifndef __MENU_WIDGET_SHELL_SAVEINFO_HPP__
#define __MENU_WIDGET_SHELL_SAVEINFO_HPP__


/*
================================================
idMenuWidget_Shell_SaveInfo
================================================
*/
class idMenuWidget_Shell_SaveInfo: public idMenuWidget
{
public:
	idMenuWidget_Shell_SaveInfo();
	virtual void	Initialize( idMenuHandler* data );
	virtual void	Update();
	virtual void   UpdateSavePreviews();
	virtual void	ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
	void			SetForSaveScreen( bool val )
	{
		forSaveScreen = val;
	}
private:
	const idMaterial* saveThumbDefault;
	const idMaterial* saveGameThumbnails[MAX_SAVEGAMES];
	int		loadIndex;
	bool	forSaveScreen;
};


#endif //!__MENU_WIDGET_SHELL_SAVEINFO_HPP__