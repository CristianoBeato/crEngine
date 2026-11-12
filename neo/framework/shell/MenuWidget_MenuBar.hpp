
#ifndef __MENU_WIDGET_MENU_BAR_HPP__
#define __MENU_WIDGET_MENU_BAR_HPP__

/*
================================================
idMenuWidget_NavBar

The nav bar is set up with the main option being at the safe frame line.
================================================
*/
class idMenuWidget_MenuBar : public idMenuWidget_DynamicList
{
public:

	idMenuWidget_MenuBar() :
		totalWidth( 0.0f ),
		buttonPos( 0.0f ),
		rightSpacer( 0.0f )
	{
	}
	
	virtual void				Update();
	virtual void				Initialize( idMenuHandler* data );
	virtual void				SetButtonSpacing( float rSpace )
	{
		rightSpacer = rSpace;
	}
	virtual bool				PrepareListElement( idMenuWidget& widget, const int navIndex );
	virtual void				SetListHeadings( idList< idStr >& list );
	virtual int					GetTotalNumberOfOptions() const;
	
private:

	idList< idStr, TAG_IDLIB_LIST_MENU >				headings;
	float						totalWidth;
	float						buttonPos;
	float						rightSpacer;
	
};

#endif //!__MENU_WIDGET_MENU_BAR_HPP__