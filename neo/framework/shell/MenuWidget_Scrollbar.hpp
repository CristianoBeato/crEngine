
#ifndef __MENU_WIDGET_SCROLLBAR_HPP__
#define __MENU_WIDGET_SCROLLBAR_HPP__

/*
================================================
idMenuWidget_DynamicList
================================================
*/
class idMenuWidget_ScrollBar : public idMenuWidget
{
public:
	idMenuWidget_ScrollBar() :
		yTop( 0.0f ),
		yBot( 0.0f ),
		dragging( false )
	{
	}
	
	virtual void	Initialize( idMenuHandler* data );
	virtual void	Update();
	virtual bool	HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual void	ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
	
	void			CalcTopAndBottom();
	void			CalculatePosition( float x, float y );
	
	float			yTop;
	float			yBot;
	bool			dragging;
};

#endif //!__MENU_WIDGET_SCROLLBAR_HPP__