
#ifndef __MENU_WIDGET_HELP_HPP__
#define __MENU_WIDGET_HELP_HPP__

/*
================================================
idMenuWidget_Help

Knows how to display help tooltips by observing events from other widgets
================================================
*/
class idMenuWidget_Help : public idMenuWidget
{
public:
	virtual void Update();
	virtual void ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
	
private:
	idStr		lastFocusedMessage;		// message from last widget that had focus
	idStr		lastHoveredMessage;		// message from last widget that was hovered over
	bool		hideMessage;
};

#endif //!__MENU_WIDGET_HELP_HPP__