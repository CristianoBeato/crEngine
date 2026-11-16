
#ifndef __MENU_WIDGET_BUTTON_HPP__
#define __MENU_WIDGET_BUTTON_HPP__

/*
================================================
idMenuWidget_Button

This might be better named "ComboButton", as it contains behavior for several controls along
with standard button behavior.
================================================
*/
class idMenuWidget_Button : public idMenuWidget
{
public:

	enum animState_t
	{
		ANIM_STATE_UP,			// standard
		ANIM_STATE_DOWN,		// pressed down
		ANIM_STATE_OVER,		// hovered over this
		ANIM_STATE_MAX
	};
	
	idMenuWidget_Button() :
		animState( ANIM_STATE_UP ),
		img( nullptr ),
		ignoreColor( false )
	{
	}
	
	virtual ~idMenuWidget_Button() {}
	
	virtual bool			ExecuteEvent( const idWidgetEvent& event );
	virtual void			Update();
	
	//---------------
	// Model
	//---------------
	void					SetLabel( const idStr& label )
	{
		btnLabel = label;
	}
	const idStr& 			GetLabel() const
	{
		return btnLabel;
	}
	void					SetValues( idList< idStr >& list );
	const idStr& 			GetValue( int index ) const;
	void					SetImg( const idMaterial* val )
	{
		img = val;
	}
	const idMaterial* 		GetImg()
	{
		return img;
	}
	void					SetDescription( const char* desc_ )
	{
		description = desc_;
	}
	const idStr& 			GetDescription() const
	{
		return description;
	}
	
	void					SetIgnoreColor( const bool b )
	{
		ignoreColor = b;
	}
	
	animState_t				GetAnimState() const
	{
		return animState;
	}
	void					SetAnimState( const animState_t state )
	{
		animState = state;
	}
	void					SetOnPressFunction( idSWFScriptFunction* func )
	{
		scriptFunction = func;
	}
	
protected:
	void					SetupTransitionInfo( widgetTransition_t& trans, const widgetState_t buttonState, const animState_t sourceAnimState, const animState_t destAnimState ) const;
	void					AnimateToState( const animState_t targetState, const bool force = false );
	
	idList< idStr, TAG_IDLIB_LIST_MENU >			values;
	idStr					btnLabel;
	idStr					description;
	animState_t				animState;
	const idMaterial* 		img;
	idSWFScriptFunction* 	scriptFunction;
	bool					ignoreColor;
};

/*
================================================
idMenuWidget_LobbyButton
================================================
*/
class idMenuWidget_LobbyButton : public idMenuWidget_Button
{
public:
	idMenuWidget_LobbyButton() :
		voiceState( VOICECHAT_DISPLAY_NONE )
	{
	}
	
	virtual void			Update();
	void					SetButtonInfo( idStr name_, voiceStateDisplay_t voiceState_ );
	bool					IsValid()
	{
		return !name.IsEmpty();
	}
	
protected:
	idStr					name;
	voiceStateDisplay_t		voiceState;
};

/*
================================================
idMenuWidget_ScoreboardButton
================================================
*/
class idMenuWidget_ScoreboardButton : public idMenuWidget_Button
{
public:
	idMenuWidget_ScoreboardButton() :
		voiceState( VOICECHAT_DISPLAY_NONE ),
		index( -1 )
	{
	}
	
	virtual void			Update();
	void					SetButtonInfo( int index_, idList< idStr >& list, voiceStateDisplay_t voiceState_ );
	
protected:
	voiceStateDisplay_t		voiceState;
	int						index;
};

/*
================================================
idMenuWidget_ControlButton
================================================
*/
class idMenuWidget_ControlButton : public idMenuWidget_Button
{
public:
	idMenuWidget_ControlButton() :
		optionType( OPTION_BUTTON_TEXT ),
		disabled( false )
	{
	}
	
	virtual void			Update();
	void					SetOptionType( const menuOption_t type )
	{
		optionType = type;
	}
	menuOption_t			GetOptionType() const
	{
		return optionType;
	}
	void					SetupEvents( int delay, int index );
	void					SetDisabled( bool disable )
	{
		disabled = disable;
	}
	
protected:
	menuOption_t			optionType;
	bool					disabled;
};

/*
================================================
idMenuWidget_ServerButton
================================================
*/
class idMenuWidget_ServerButton : public idMenuWidget_Button
{
public:

	idMenuWidget_ServerButton() :
		index( 0 ),
		players( 0 ),
		maxPlayers( 0 ),
		joinable( false ),
		validMap( false )
	{
	}
	
	virtual void			Update();
	void					SetButtonInfo( idStr name_, idStrId mapName_, idStr modeName_, int index_ = 0, int players_ = 0, int maxPlayers_ = 0, bool joinable_ = false, bool validMap_ = false );
	bool					IsValid()
	{
		return !serverName.IsEmpty();
	}
	bool					CanJoin()
	{
		return ( joinable && validMap );
	}
	
protected:
	idStr					serverName;
	int						index;
	int						players;
	int						maxPlayers;
	bool					joinable;
	bool					validMap;
	idStrId					mapName;
	idStr					modeName;
};

#endif //!__MENU_WIDGET_BUTTON_HPP__