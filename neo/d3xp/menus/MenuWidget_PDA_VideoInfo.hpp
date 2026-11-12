
#ifndef __MENU_WIDGET_PDA_VIDEO_INFO_HPP__
#define __MENU_WIDGET_PDA_VIDEO_INFO_HPP__

/*
================================================
idMenuWidget_PDA_AudioFiles
================================================
*/
class idMenuWidget_PDA_VideoInfo: public idMenuWidget
{
public:
virtual void	Update();
	virtual void	ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
private:
	int				videoIndex;
};

#endif //!__MENU_WIDGET_PDA_VIDEO_INFO_HPP__