
#ifndef __MENU_WIDGET_PDA_AUDIO_FILES_HPP__
#define __MENU_WIDGET_PDA_AUDIO_FILES_HPP__

/*
================================================
idMenuWidget_PDA_AudioFiles
================================================
*/
class idMenuWidget_PDA_AudioFiles: public idMenuWidget
{
public:
	idMenuWidget_PDA_AudioFiles() :
		pdaIndex( 0 )
	{
	}
	virtual ~idMenuWidget_PDA_AudioFiles();
	virtual void	Update();
	virtual void	Initialize( idMenuHandler* data );
	virtual void	ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
private:
	int								pdaIndex;
	idList< idList< idStr, TAG_IDLIB_LIST_MENU >, TAG_IDLIB_LIST_MENU >		audioFileNames;
};

#endif //!__MENU_WIDGET_PDA_AUDIO_FILES_HPP__