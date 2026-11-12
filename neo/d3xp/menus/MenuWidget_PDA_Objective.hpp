
#ifndef __MENU_WIDGET_OBJECTIVE_HPP__
#define __MENU_WIDGET_OBJECTIVE_HPP__

/*
================================================
idMenuWidget_PDA_Objective
================================================
*/
class idMenuWidget_PDA_Objective: public idMenuWidget
{
public:
	idMenuWidget_PDA_Objective() :
		pdaIndex( 0 )
	{
	}
	virtual void	Update();
	virtual void	ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
private:
	int		pdaIndex;
};

#endif //!__MENU_WIDGET_OBJECTIVE_HPP__