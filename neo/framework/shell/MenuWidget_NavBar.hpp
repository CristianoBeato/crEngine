
#ifndef __MENU_WIDGET_NAVBAR_HPP__
#define __MENU_WIDGET_NAVBAR_HPP__

/*
================================================
idMenuWidget_NavBar

The nav bar is set up with the main option being at the safe frame line.
================================================
*/
class idMenuWidget_NavBar : public idMenuWidget_DynamicList
{
public:

	idMenuWidget_NavBar() :
		initialPos( 0.0f ),
		buttonPos( 0.0f ),
		leftSpacer( 0.0f ),
		rightSpacer( 0.0f ),
		selectedSpacer( 0.0f )
	{
	}
	
	virtual void				Update();
	virtual void				Initialize( idMenuHandler* data );
	virtual void				SetInitialXPos( float pos )
	{
		initialPos = pos;
	}
	virtual void				SetButtonSpacing( float lSpace, float rSpace, float sSpace )
	{
		leftSpacer = lSpace;
		rightSpacer = rSpace;
		selectedSpacer = sSpace;
	}
	virtual bool				PrepareListElement( idMenuWidget& widget, const int navIndex );
	virtual void				SetListHeadings( idList< idStr >& list );
	virtual int					GetTotalNumberOfOptions() const;
	
private:

	idList< idStr, TAG_IDLIB_LIST_MENU >		headings;
	float				initialPos;
	float				buttonPos;
	float				leftSpacer;
	float				rightSpacer;
	float				selectedSpacer;	
};

#endif //!__MENU_WIDGET_NAVBAR_HPP__