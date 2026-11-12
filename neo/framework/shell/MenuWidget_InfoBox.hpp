
#ifndef __MENU_WIDGET_INFO_BOX_HPP__
#define __MENU_WIDGET_INFO_BOX_HPP__

/*
================================================
idMenuWidget_InfoBox
================================================
*/
class idMenuWidget_InfoBox: public idMenuWidget
{
public:
	idMenuWidget_InfoBox() :
		scrollbar( nullptr )
	{
	}
	
	virtual void	Initialize( idMenuHandler* data );
	virtual void	Update();
	virtual bool	HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual void	ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
	void			SetHeading( idStr val )
	{
		heading = val;
	}
	void			SetBody( idStr val )
	{
		info = val;
	}
	void			ResetInfoScroll();
	void			Scroll( int d );
	int				GetScroll();
	int				GetMaxScroll();
	void			SetScroll( int scroll );
	void			SetScrollbar( idMenuWidget_ScrollBar* bar );
private:
	idMenuWidget_ScrollBar* 	scrollbar;
	idStr						heading;
	idStr						info;
};

#endif //!__MENU_WIDGET_INFO_BOX_HPP__