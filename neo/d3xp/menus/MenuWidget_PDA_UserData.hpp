
#ifndef __MENU_WIDGET_PDA_USER_DATA_HPP__
#define __MENU_WIDGET_PDA_USER_DATA_HPP__

/*
================================================
idMenuWidget_PDA_UserData
================================================
*/
class idMenuWidget_PDA_UserData : public idMenuWidget
{
public:
	idMenuWidget_PDA_UserData() :
		pdaIndex( 0 )
	{
	}
	virtual ~idMenuWidget_PDA_UserData() {}
	virtual void	Update();
	virtual void	ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
	
private:
	int		pdaIndex;
};

#endif //!__MENU_WIDGET_PDA_USER_DATA_HPP__