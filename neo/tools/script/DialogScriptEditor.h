/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#ifndef __DIALOGSCRIPTEDITOR_H__
#define __DIALOGSCRIPTEDITOR_H__

// DialogScriptEditor dialog

#include "Gwen/Controls.h"
class DialogScriptEditor : public Gwen::Controls::Canvas 
{
	DECLARE_DYNAMIC(DialogScriptEditor)

public:
						DialogScriptEditor( CWnd* pParent = nullptr );   // standard constructor
	virtual				~DialogScriptEditor( void );

	void				OpenFile( const char *fileName );

	virtual bool		OnInitDialog( void );
	virtual void		DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	virtual bool		PreTranslateMessage( MSG* pMsg );

protected:
	
	bool		OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult );
	void		OnSetFocus( CWnd *pOldWnd );
	void		OnDestroy();
	void		OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );
	void		OnMove( int x, int y );
	void		OnSize( UINT nType, int cx, int cy );
	void		OnSizing( UINT nSide, LPRECT lpRect );
	void		OnEditGoToLine();
	void		OnEditFind();
	void		OnEditFindNext();
	void		OnEditReplace();
    LRESULT		OnFindDialogMessage( WPARAM wParam, LPARAM lParam );
	void		OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult );
	void		OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult );
	void		OnBnClickedOk( void );
	void		OnBnClickedCancel( void );
	
private:

	enum				{ IDD = IDD_DIALOG_SCRIPTEDITOR };
	CStatusBarCtrl			statusBar;
	CSyntaxRichEditCtrl		scriptEdit;
	Gwen::Controls::Button*	okButton;
	Gwen::Controls::Button*	cancelButton;
	//}}AFX_DATA

	static toolTip_t	toolTips[];

	HACCEL				m_hAccel;
	CRect				initialRect;
	CFindReplaceDialog *findDlg;
	Gwen::String		findStr;
	Gwen::String		replaceStr;
	bool				matchCase;
	bool				matchWholeWords;
	bool				searchForward;
	idStr				fileName;
	int					firstLine;

private:
	void				InitScriptEvents( void );
	void				UpdateStatusBar( void );
};

#endif /* !__DIALOGSCRIPTEDITOR_H__ */
