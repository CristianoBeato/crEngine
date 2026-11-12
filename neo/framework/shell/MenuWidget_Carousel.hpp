#ifndef __MENU_WIDGET_CAROUSEL_HPP__
#define __MENU_WIDGET_CAROUSEL_HPP__

/*
================================================
idMenuWidget_Carousel
Displays a list of items in a looping carousel pattern
================================================
*/
class idMenuWidget_Carousel : public idMenuWidget
{
public:

	idMenuWidget_Carousel() :
		numVisibleOptions( 0 ),
		viewIndex( 0 ),
		moveToIndex( 0 ),
		moveDiff( 0 ),
		fastScroll( false ),
		scrollLeft( false )
	{
	}
	
	virtual void				Initialize( idMenuHandler* data );
	virtual void				Update();
	virtual bool				HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual int					GetTotalNumberOfOptions() const
	{
		return imgList.Num();
	}
	virtual bool				PrepareListElement( idMenuWidget& widget, const int childIndex )
	{
		return true;
	}
	
	void						SetNumVisibleOptions( const int numVisibleOptions_ )
	{
		numVisibleOptions = numVisibleOptions_;
	}
	int							GetNumVisibleOptions() const
	{
		return numVisibleOptions;
	}
	
	void						MoveToIndex( int index, bool instant = false );
	void						MoveToFirstItem( bool instant = true );
	void						MoveToLastItem( bool instant = true );
	int							GetMoveToIndex()
	{
		return moveToIndex;
	}
	void						SetMoveToIndex( int index )
	{
		moveToIndex = index;
	}
	void						SetViewIndex( int index )
	{
		viewIndex = index;
	}
	int							GetViewIndex() const
	{
		return viewIndex;
	}
	void						SetListImages( idList<const idMaterial*>& list );
	void						SetMoveDiff( int val )
	{
		moveDiff = val;
	}
	int							GetMoveDiff()
	{
		return moveDiff;
	}
	bool						GetScrollLeft()
	{
		return scrollLeft;
	}
	
private:

	int							numVisibleOptions;
	int							viewIndex;
	int							moveToIndex;
	int							moveDiff;
	bool						fastScroll;
	bool						scrollLeft;
	idList<const idMaterial*>	imgList;
	
};

#endif //!__MENU_WIDGET_CAROUSEL_HPP__