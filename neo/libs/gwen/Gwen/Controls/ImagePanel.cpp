/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#include "Gwen/Precompiled.h"
#include "Gwen/Controls/ImagePanel.h"

GWEN_CONTROL_CONSTRUCTOR( ImagePanel )
{
	SetUV( 0, 0, 1, 1 );
	SetMouseInputEnabled( false );
	m_DrawColor = Colors::White;
	SetStretch( true );
}

Gwen::Controls::ImagePanel::~ImagePanel( void )
{
	m_Texture.Release( GetSkin()->GetRender() );
}

void Gwen::Controls::ImagePanel::SetUV( float u1, float v1, float u2, float v2 )
{
	m_uv[0] = u1;
	m_uv[1] = v1;
	m_uv[2] = u2;
	m_uv[3] = v2;
}

void Gwen::Controls::ImagePanel::SetImage( const TextObject & imageName )
{
	m_Texture.Load( imageName, GetSkin()->GetRender() );
}
