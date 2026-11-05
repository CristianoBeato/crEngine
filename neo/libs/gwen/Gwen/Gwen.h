/*
	GWEN

	Copyright (c) 2010 Facepunch Studios

	MIT License

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#ifndef GWEN_GWEN_H
#define GWEN_GWEN_H

#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <list>

#include "Gwen/Macros.h"
#include "Gwen/Config.h"
#include "Gwen/Exports.h"
#include "Gwen/Structures.h"
#include "Gwen/Utility.h"
#include "Gwen/TextObject.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "Gwen/FrameBuffer.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/Canvas.h"
#include "Gwen/Align.h"

// Enable the hook system (se Hook.h)
#define GWEN_HOOKSYSTEM

namespace Gwen
{
	

	extern GWEN_EXPORT Controls::Base*	HoveredControl;
	extern GWEN_EXPORT Controls::Base*	KeyboardFocus;
	extern GWEN_EXPORT Controls::Base*	MouseFocus;

	namespace Input
	{
		inline void Blur()
		{
			if ( KeyboardFocus )
			{ KeyboardFocus->Blur(); }
		}
	}

} //namespace Gwen

#include "Controls.h"

#endif
