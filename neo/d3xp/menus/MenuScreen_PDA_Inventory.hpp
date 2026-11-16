
#ifndef __MENU_SCREEN_PDA_INVENTORY_HPP__
#define __MENU_SCREEN_PDA_INVENTORY_HPP__

/*
================================================
idMenuScreen_PDA_UserData
================================================
*/
class idMenuScreen_PDA_Inventory : public idMenuScreen
{
public:
	idMenuScreen_PDA_Inventory()
	{
	}
	virtual void				Initialize( idMenuHandler* data );
	virtual void				Update();
	virtual void				ShowScreen( const mainMenuTransition_t transitionType );
	virtual void				HideScreen( const mainMenuTransition_t transitionType );
	virtual bool				HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	
	void						EquipWeapon();
	const char* 				GetWeaponName( int index );
	bool						IsVisibleWeapon( int index );
	
private:
	idMenuWidget_Carousel 			itemList;
	idMenuWidget_InfoBox 			infoBox;
};

#endif //!__MENU_SCREEN_PDA_INVENTORY_HPP__