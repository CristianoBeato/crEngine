
#ifndef __MENU_SCREEN_PDA_USER_EMAILS_HPP__
#define __MENU_SCREEN_PDA_USER_EMAILS_HPP__

/*
================================================
idMenuScreen_PDA_UserEmails
================================================
*/
class idMenuScreen_PDA_UserEmails : public idMenuScreen
{
public:
	idMenuScreen_PDA_UserEmails() :
		readingEmails( false ),
		scrollEmailInfo( false )
	{
	}
	
	virtual ~idMenuScreen_PDA_UserEmails()
	{
	}
	
	virtual void					Update();
	virtual void					Initialize( idMenuHandler* data );
	virtual void					ShowScreen( const mainMenuTransition_t transitionType );
	virtual void					HideScreen( const mainMenuTransition_t transitionType );
	virtual bool					HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual void					ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
	idMenuWidget_PDA_EmailInbox& 	GetInbox()
	{
		return pdaInbox;
	}
	
	bool							ScrollCorrectList( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget );
	void							ShowEmail( bool show );
	void							UpdateEmail();
private:
	idMenuWidget_PDA_EmailInbox 	pdaInbox;
	idMenuWidget_InfoBox 			emailInfo;
	idMenuWidget_ScrollBar 			emailScrollbar;
	bool							readingEmails;
	bool							scrollEmailInfo;
};

#endif //__MENU_SCREEN_PDA_USER_EMAILS_HPP__