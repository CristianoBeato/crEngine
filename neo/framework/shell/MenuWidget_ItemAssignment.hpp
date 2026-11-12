
#ifndef __MENU_WIDGET_ITEM_ASSIGNAMENT_HPP__
#define __MENU_WIDGET_ITEM_ASSIGNAMENT_HPP__

/*
================================================
	idMenuWidget_ItemAssignment
================================================
*/
class idMenuWidget_ItemAssignment: public idMenuWidget
{
public:
	idMenuWidget_ItemAssignment() :
		slotIndex( 0 )
	{
	}
	
	virtual void	Update();
	void			SetIcon( int index, const idMaterial* icon );
	void			FindFreeSpot();
	int				GetSlotIndex()
	{
		return slotIndex;
	}
	void			SetSlotIndex( int num )
	{
		slotIndex = num;
	}
private:
	const idMaterial* images[ NUM_QUICK_SLOTS ];
	int				slotIndex;
	
};

#endif //!__MENU_WIDGET_ITEM_ASSIGNAMENT_HPP__