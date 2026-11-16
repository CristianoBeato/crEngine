
#ifndef __MENU_WIDGET_LIST_HPP__
#define __MENU_WIDGET_LIST_HPP__

/*
================================================
idMenuWidget_List

Stores a list of widgets but displays only a segment of them at a time.
================================================
*/
class idMenuWidget_List : public idMenuWidget
{
public:
	idMenuWidget_List() :
		numVisibleOptions( 0 ),
		viewOffset( 0 ),
		viewIndex( 0 ),
		allowWrapping( false )
	{
	
	}
	
	virtual void				Update();
	virtual bool				HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual void				ObserveEvent( const idMenuWidget& widget, const idWidgetEvent& event );
	virtual void				Scroll( const int scrollIndexAmount, const bool wrapAround = false );
	virtual void				ScrollOffset( const int scrollIndexAmount );
	virtual int					GetTotalNumberOfOptions() const
	{
		return GetChildren().Num();
	}
	virtual bool				PrepareListElement( idMenuWidget& widget, const int childIndex )
	{
		return true;
	}
	
	bool						IsWrappingAllowed() const
	{
		return allowWrapping;
	}
	void						SetWrappingAllowed( const bool allow )
	{
		allowWrapping = allow;
	}
	
	void						SetNumVisibleOptions( const int numVisibleOptions_ )
	{
		numVisibleOptions = numVisibleOptions_;
	}
	int							GetNumVisibleOptions() const
	{
		return numVisibleOptions;
	}
	
	int							GetViewOffset() const
	{
		return viewOffset;



	}
	void						SetViewOffset( const int offset )
	{
		viewOffset = offset;
	}
	
	int							GetViewIndex() const
	{
		return viewIndex;
	}
	void						SetViewIndex( const int index )
	{
		viewIndex = index;
	}
	
	void						CalculatePositionFromIndexDelta( int& outIndex, int& outOffset, const int currentIndex, const int currentOffset, const int windowSize, const int maxSize, const int indexDelta, const bool allowWrapping, const bool wrapAround = false ) const;
	void						CalculatePositionFromOffsetDelta( int& outIndex, int& outOffset, const int currentIndex, const int currentOffset, const int windowSize, const int maxSize, const int offsetDelta ) const;
	
private:
	int							numVisibleOptions;
	int							viewOffset;
	int							viewIndex;
	bool						allowWrapping;
};


class idBrowserEntry_t
{
public:
	idStr serverName;
	int	index;
	int players;
	int maxPlayers;
	bool joinable;
	bool validMap;
	idStrId mapName;
	idStr modeName;
};


/*
================================================
idMenuWidget_GameBrowserList
================================================
*/
class idMenuWidget_GameBrowserList : public idMenuWidget_List
{
public:
	virtual void				Update();
	virtual bool				PrepareListElement( idMenuWidget& widget, const int childIndex );
	virtual int					GetTotalNumberOfOptions() const;
	void						ClearGames();
	void						AddGame( idStr name_, idStrId mapName_, idStr modeName_, int index_ = 0, int players_ = 0, int maxPlayers_ = 0, bool joinable_ = false, bool validMap_ = false );
	int							GetServerIndex();
private:
	idList< idBrowserEntry_t >	games;
};

class idControlEntry_t
{
public:
	int	fieldIndex;
	idStr label;
	menuOption_t type;
	int repeat;
	idMenuDataSource* dataSource;
};


/*
================================================
idMenuWidget_ControlList
================================================
*/
class idMenuWidget_ControlList : public idMenuWidget_List
{
public:
	virtual void				Update();
	virtual bool				PrepareListElement(idMenuWidget& widget, const int childIndex);
	virtual int					GetTotalNumberOfOptions() const;
	void						ClearControls();
	void						AddButton(idStr label_, idMenuDataSource* dataSource_, int fieldIndex_, int repeat_);
	void						AddButtonInfo(idStr label_, idMenuDataSource* dataSource_, int fieldIndex_, int repeat_);
	void						AddFullTextSlider(idStr label_, idMenuDataSource* dataSource_, int fieldIndex_, int repeat_);
	void						AddTextSlider(idStr label_, idMenuDataSource* dataSource_, int fieldIndex_, int repeat_);
	void						AddSliderBar(idStr label_, idMenuDataSource* dataSource_, int fieldIndex_, int repeat_);
	void						AddSliderToggle(idStr label_, idMenuDataSource* dataSource_, int fieldIndex_, int repeat_);
	//int							GetServerIndex();
private:
	idList< idControlEntry_t >	controls;
};

#endif //!__MENU_WIDGET_LIST_HPP__