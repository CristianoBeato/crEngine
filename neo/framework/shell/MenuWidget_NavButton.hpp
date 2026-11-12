
#ifndef __MENU_WIDGET_NAV_BUTTON_HPP__
#define __MENU_WIDGET_NAV_BUTTON_HPP__

/*
================================================
idMenuWidget_NavButton
================================================
*/
class idMenuWidget_NavButton : public idMenuWidget_Button
{
public:

	enum navWidgetState_t
	{
		NAV_WIDGET_LEFT,		// option on left side
		NAV_WIDGET_RIGHT,		// option on right side
		NAV_WIDGET_SELECTED		// option is selected
	};
	
	idMenuWidget_NavButton() :
		navIndex( 0 ),
		xPos( 0 )
	{
	}
	
	virtual bool			ExecuteEvent( const idWidgetEvent& event );
	virtual void			Update();
	
	void					SetNavIndex( int i, const navWidgetState_t type )
	{
		navIndex = i;
		navState = type;
	}
	void					SetPosition( float pos )
	{
		xPos = pos;
	}
	
private:

	int						navIndex;
	float					xPos;
	navWidgetState_t		navState;
	
};

/*
================================================
idMenuWidget_NavButton
================================================
*/
class idMenuWidget_MenuButton : public idMenuWidget_Button
{
public:

	idMenuWidget_MenuButton() :
		xPos( 0 )
	{
	}
	
	virtual void			Update();
	void					SetPosition( float pos )
	{
		xPos = pos;
	}
	
private:
	float					xPos;	
};

#endif //!__MENU_WIDGET_NAV_BUTTON_HPP__