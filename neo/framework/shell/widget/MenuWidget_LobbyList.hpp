
#ifndef __MENU_WIDGET_LOBBY_LIST_HPP__
#define __MENU_WIDGET_LOBBY_LIST_HPP__

/*
================================================
	idMenuWidget_DynamicList
================================================
*/
class idMenuWidget_LobbyList : public idMenuWidget_List
{
public:
	idMenuWidget_LobbyList() :
		numEntries( 0 )
	{
	}
	
	virtual void				Update();
	virtual bool				PrepareListElement( idMenuWidget& widget, const int childIndex );
	virtual int					GetTotalNumberOfOptions() const
	{
		return numEntries;
	}
	void						SetEntryData( int index, idStr name, voiceStateDisplay_t voiceState );
	void						SetHeadingInfo( idList< idStr >& list );
	void						SetNumEntries( int num )
	{
		numEntries = num;
	}
	int							GetNumEntries()
	{
		return numEntries;
	}
	void						SetRefreshFunction( const char* func );
private:
	idList< idStr, TAG_IDLIB_LIST_MENU >	headings;
	int							numEntries;
};

#endif //!__MENU_WIDGET_LOBBY_LIST_HPP__