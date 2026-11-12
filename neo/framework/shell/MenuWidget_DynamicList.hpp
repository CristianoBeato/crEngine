
#ifndef __MENU_WIDGET_DYNAMIC_LIST_HPP__
#define __MENU_WIDGET_DYNAMIC_LIST_HPP__

/*
================================================
idMenuWidget_DynamicList
================================================
*/
class idMenuWidget_DynamicList : public idMenuWidget_List
{
public:

	idMenuWidget_DynamicList() :
		controlList( false ),
		ignoreColor( false )
	{
	}
	
	virtual void				Update();
	virtual void				Initialize( idMenuHandler* data );
	virtual int					GetTotalNumberOfOptions() const;
	virtual bool				PrepareListElement( idMenuWidget& widget, const int childIndex );
	
	virtual void				Recalculate();
	virtual void				SetListData( idList< idList< idStr, TAG_IDLIB_LIST_MENU >, TAG_IDLIB_LIST_MENU >& list );
	
	void						SetControlList( bool val )
	{
		controlList = val;
	}
	void						SetIgnoreColor( bool val )
	{
		ignoreColor = val;
	}
	
protected:
	idList< idList< idStr, TAG_IDLIB_LIST_MENU >, TAG_IDLIB_LIST_MENU >	listItemInfo;
	bool						controlList;
	bool						ignoreColor;
};

/*
================================================
idMenuWidget_ScoreboardList
================================================
*/
class idMenuWidget_ScoreboardList : public idMenuWidget_DynamicList
{
public:
	virtual void				Update();
	virtual int				GetTotalNumberOfOptions() const;
};

#endif 