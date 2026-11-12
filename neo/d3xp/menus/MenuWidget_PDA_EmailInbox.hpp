
#ifndef __MENU_WIDGET_PDA_EMAILINBOX_HPP__
#define __MENU_WIDGET_PDA_EMAILINBOX_HPP__

/*
================================================
idMenuWidget_PDA_AudioFiles
================================================
*/
class idMenuWidget_PDA_EmailInbox: public idMenuWidget
{
public:
	idMenuWidget_PDA_EmailInbox() :
		emailList( nullptr ),
		scrollbar( nullptr ),
		pdaIndex( 0 )
	{
	}
	virtual void	Update();
	virtual void	Initialize( idMenuHandler* data );
	virtual void	ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
	
	idMenuWidget_DynamicList* 	GetEmailList()
	{
		return emailList;
	}
	idMenuWidget_ScrollBar* 	GetScrollbar()
	{
		return scrollbar;
	}
private:
	idMenuWidget_DynamicList* 		emailList;
	idMenuWidget_ScrollBar* 		scrollbar;
	int								pdaIndex;
	idList< idList< idStr, TAG_IDLIB_LIST_MENU >, TAG_IDLIB_LIST_MENU >		emailInfo;
};

#endif //!__MENU_WIDGET_PDA_EMAILINBOX_HPP__