
#ifndef __MENU_HANDLER_CINEMATIC_HPP__
#define __MENU_HANDLER_CINEMATIC_HPP__

/*
================================================
idMenuHandler_Cinematic
================================================
*/
class idMenuHandler_Cinematic : public idMenuHandler
{
public:

	idMenuHandler_Cinematic() :
				hiding( false )
	{
	}
	
	
	virtual void			Update();
	virtual void			ActivateMenu( bool show );
	virtual void			Initialize( const char* swfFile, idSoundWorld* sw );
	virtual idMenuScreen* 	GetMenuScreen( int index );
	
	idMenuScreen_Cinematic* 		GetCinematic();

	
protected:

	bool	hiding;
	
};

#endif //!__MENU_HANDLER_CINEMATIC_HPP__