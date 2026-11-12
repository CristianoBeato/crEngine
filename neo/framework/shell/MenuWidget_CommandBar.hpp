
#ifndef __MENU_WIDGET_COMMAND_BAR_HPP__
#define __MENU_WIDGET_COMMAND_BAR_HPP__

/*
================================================
idMenuWidget_CommandBar
================================================
*/
class idMenuWidget_CommandBar : public idMenuWidget
{
public:
	enum button_t
	{
		BUTTON_JOY1,
		BUTTON_JOY2,
		BUTTON_JOY3,
		BUTTON_JOY4,
		BUTTON_JOY10,
		BUTTON_TAB,
		MAX_BUTTONS
	};
	
	enum alignment_t
	{
		LEFT,
		RIGHT
	};
	
	struct buttonInfo_t
	{
		idStr			label;			// empty labels are treated as hidden buttons
		idWidgetAction	action;
	};
	
	idMenuWidget_CommandBar() :
		alignment( LEFT )
	{
	
		buttons.SetNum( MAX_BUTTONS );
	}
	
	virtual void		Update();
	virtual bool		ExecuteEvent( const idWidgetEvent& event );
	
	buttonInfo_t* 		GetButton( const button_t button )
	{
		return &buttons[ button ];
	}
	void				ClearAllButtons();
	
	alignment_t			GetAlignment() const
	{
		return alignment;
	}
	void				SetAlignment( const alignment_t alignment_ )
	{
		alignment = alignment_;
	}
	
private:
	idStaticList< buttonInfo_t, MAX_BUTTONS >	buttons;
	alignment_t									alignment;
};

#endif //!__MENU_WIDGET_COMMAND_BAR_HPP__