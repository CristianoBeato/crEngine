
#ifndef __GWEN_DEBUG__
#define __GWEN_DEBUG__

#include "Gwen/Exports.h"

namespace Gwen
{
	namespace Debug
	{
		void GWEN_EXPORT Msg( const wchar_t* str, ... );
		void GWEN_EXPORT Msg( const char* str, ... );
		void GWEN_EXPORT AssertCheck( bool b, const char* strMsg );
	}	
}

#endif //__GWEN_DEBUG__