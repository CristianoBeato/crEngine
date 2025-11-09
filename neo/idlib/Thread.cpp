/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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
#pragma hdrstop
#include "precompiled.h"

/*
================================================================================================
Contains the vartious ThreadingClass implementations.
================================================================================================
*/

/*
================================================================================================

	Signal

================================================================================================
*/

/*
========================
idSysSignal::idSysSignal
========================
*/
idSysSignal::idSysSignal( const bool in_manualReset ) :
	signaled( false ),
	manualReset( in_manualReset ),
	waiting( 0 ),
	cond( nullptr ),
	mutex( nullptr )
{
	manualReset = manualReset;
	// if this is true, the signal is only set to nonsignaled when Clear() is called,
	// else it's "auto-reset" and the state is set to !signaled after a single waiting
	// thread has been released
	
	// the inital state is always "not signaled"
	signaled = false;
	waiting = 0;

	cond = SDL_CreateCondition();
	if ( !cond )
		throw idException( SDL_GetError() );

	mutex = SDL_CreateMutex();
	if ( !mutex )
		throw idException( SDL_GetError() );
}

/*
========================
idSysSignal::~idSysSignal
========================
*/
idSysSignal::~idSysSignal(void)
{
	// CloseHandle( handle );
	signaled = false;
	waiting = 0;

	if ( mutex != nullptr )
	{
		SDL_DestroyMutex( mutex );
		mutex = nullptr;
	}
	
	if ( cond != nullptr )
	{
		SDL_DestroyCondition( cond );
		cond = nullptr;
	}	
}

/*
========================
idSysSignal::Raise
========================
*/
void idSysSignal::Raise( void )
{
	SDL_LockMutex( mutex );
	
	if( manualReset )
	{
		// signaled until reset
		signaled = true;
		// wake *all* threads waiting on this cond
		SDL_BroadcastCondition( cond );
	}
	else
	{
		// automode: signaled until first thread is released
		if( waiting > 0 )
		{
			// there are waiting threads => release one
			SDL_SignalCondition( cond );
		}
		else
		{
			// no waiting threads, save signal
			signaled = true;
			// while the MSDN documentation is a bit unspecific about what happens
			// when SetEvent() is called n times without a wait inbetween
			// (will only one wait be successful afterwards or n waits?)
			// it seems like the signaled state is a flag, not a counter.
			// http://stackoverflow.com/a/13703585 claims the same.
		}
	}
	
	SDL_UnlockMutex( mutex );
}

/*
========================
idSysSignal::Clear
========================
*/

void idSysSignal::Clear( void )
{
	// ResetEvent( handle );
	SDL_UnlockMutex( mutex );
	
	// TODO: probably signaled could be atomically changed?
	signaled = false;
	
	SDL_UnlockMutex( mutex );
}


/*
========================
idSysSignal::Wait
========================
*/
bool idSysSignal::Wait( const int32_t timeout )
{	
	int status;
	SDL_LockMutex( mutex );
	
	// there is a signal that hasn't been used yet
	if( signaled ) 
	{
		// for auto-mode only one thread may be released - this one.
		if( !manualReset ) 
			signaled = false;
			
		// success!
		status = 0; 
	}
	else // we'll have to wait for a signal
	{
		++waiting;
		if( timeout == idSysSignal::WAIT_INFINITE )
		{
			SDL_WaitCondition( cond, mutex );
			status = 0;
		}
		else
		{
			status = SDL_WaitConditionTimeout( cond, mutex, timeout ) ? 0 : status;
		}
		--waiting;
	}
	
	SDL_UnlockMutex( mutex );
	
	assert( status == 0 || ( timeout != idSysSignal::WAIT_INFINITE && status == ETIMEDOUT ) );
	
	return ( status == 0 );
	
}

/*
================================================================================================

	idSysThread

================================================================================================
*/

/*
========================
idSysThread::idSysThread
========================
*/
idSysThread::idSysThread() :
	threadHandle( 0 ),
	isWorker( false ),
	isRunning( false ),
	isTerminating( false ),
	moreWorkToDo( false ),
	signalWorkerDone( true ),
	forceStop( false )
{
}

/*
========================
idSysThread::~idSysThread
========================
*/
idSysThread::~idSysThread( void )
{
	StopThread( !forceStop );
	// if( threadHandle )
	// {
	// 	Sys_DestroyThread( threadHandle );
	// }
}

/*
========================
idSysThread::StartThread
========================
*/
bool idSysThread::StartThread( const char* name_, core_t core, xthreadPriority in_priority, int stackSize )
{
	if( isRunning )
		return false;
	
	name = name_;
	
	isTerminating = false;
	
	//if( threadHandle )
	//{
	//	Sys_DestroyThread( threadHandle );
	//}
	assert( threadHandle == nullptr );
	
	switch ( in_priority )
	{
		case THREAD_LOWEST:
			priority = SDL_THREAD_PRIORITY_LOW;
			break;
		
		// 
		case THREAD_BELOW_NORMAL:
		case THREAD_NORMAL:
		case THREAD_ABOVE_NORMAL:
			priority = SDL_THREAD_PRIORITY_NORMAL;
			break;
			
		case THREAD_HIGHEST:
			priority = SDL_THREAD_PRIORITY_HIGH;
			break;
	}

	SDL_PropertiesID threadProp = SDL_CreateProperties();
	SDL_SetPointerProperty( threadProp, SDL_PROP_THREAD_CREATE_ENTRY_FUNCTION_POINTER, (void*)ThreadProc );
	SDL_SetPointerProperty( threadProp, SDL_PROP_THREAD_CREATE_USERDATA_POINTER, this );
	SDL_SetStringProperty( threadProp, SDL_PROP_THREAD_CREATE_NAME_STRING, name );
	SDL_SetNumberProperty( threadProp, SDL_PROP_THREAD_CREATE_STACKSIZE_NUMBER, stackSize );

	threadHandle = SDL_CreateThreadWithProperties( threadProp );
	assert( threadHandle != nullptr );
	
	// detach worker threads 
	if ( isWorker )
		SDL_DetachThread( threadHandle );	

	isRunning = true;

	// 
	SDL_DestroyProperties( threadProp );

	return true;
}

/*
========================
idSysThread::StartWorkerThread
========================
*/
bool idSysThread::StartWorkerThread( const char* name_, core_t core, xthreadPriority priority, int stackSize )
{
	if( isRunning )
		return false;
	
	isWorker = true;
	
	bool result = StartThread( name_, core, priority, stackSize );
	
	signalWorkerDone.Wait( idSysSignal::WAIT_INFINITE );
	
	return result;
}

/*
========================
idSysThread::StopThread
========================
*/
void idSysThread::StopThread( bool wait )
{
	if( !isRunning )
		return;
	
	if( isWorker )
	{
		signalMutex.Lock();
		moreWorkToDo = true;
		signalWorkerDone.Clear();
		isTerminating = true;
		signalMoreWorkToDo.Raise();
		signalMutex.Unlock();
	}
	else
	{
		isTerminating = true;
	}

	if( wait )
		WaitForThread();
}

/*
========================
idSysThread::WaitForThread
========================
*/
void idSysThread::WaitForThread( void )
{
	if( isWorker )
	{
		signalWorkerDone.Wait( idSysSignal::WAIT_INFINITE );
	}
	else if( isRunning )
	{
		SDL_WaitThread( threadHandle, nullptr );
		threadHandle = 0;
	}
}

/*
========================
idSysThread::SignalWork
========================
*/
void idSysThread::SignalWork()
{
	if( isWorker )
	{
		signalMutex.Lock();
		moreWorkToDo = true;
		signalWorkerDone.Clear();
		signalMoreWorkToDo.Raise();
		signalMutex.Unlock();
	}
}

/*
========================
idSysThread::IsWorkDone
========================
*/
bool idSysThread::IsWorkDone()
{
	if( isWorker )
	{
		// a timeout of 0 will return immediately with true if signaled
		if( signalWorkerDone.Wait( 0 ) )
		{
			return true;
		}
	}
	return false;
}

/*
========================
idSysThread::ThreadProc
========================
*/
int idSysThread::ThreadProc( idSysThread* thread )
{
	int retVal = 0;
	SDL_SetCurrentThreadPriority( thread->priority );
	try
	{
		if( thread->isWorker )
		{
			while ( true )
			{
				thread->signalMutex.Lock();
				if( thread->moreWorkToDo )
				{
					thread->moreWorkToDo = false;
					thread->signalMoreWorkToDo.Clear();
					thread->signalMutex.Unlock();
				}
				else
				{
					thread->signalWorkerDone.Raise();
					thread->signalMutex.Unlock();
					thread->signalMoreWorkToDo.Wait( idSysSignal::WAIT_INFINITE );
					continue;
				}
				
				if( thread->isTerminating )
					break;
				
				retVal = thread->Run();
			}
			thread->signalWorkerDone.Raise();
		}
		else
		{
			retVal = thread->Run();
		}
	}
	catch( idException& ex )
	{
		idLib::Warning( "Fatal error in thread %s: %s", thread->GetName(), ex.GetError() );
		
		// We don't handle threads terminating unexpectedly very well, so just terminate the whole process
		exit( 0 );
	}
	
	thread->isRunning = false;
	
	return retVal;
}

/*
========================
idSysThread::Run
========================
*/
int idSysThread::Run()
{
	// The Run() is not pure virtual because on destruction of a derived class
	// the virtual function pointer will be set to nullptr before the idSysThread
	// destructor actually stops the thread.
	return 0;
}

/*
================================================================================================

	test

================================================================================================
*/

/*
================================================
idMyThread test class.
================================================
*/
class idMyThread : public idSysThread
{
public:
	virtual int Run()
	{
		// run threaded code here
		return 0;
	}
	// specify thread data here
};

/*
========================
TestThread
========================
*/
void TestThread()
{
	idMyThread thread;
	thread.StartThread( "myThread", CORE_ANY );
}

/*
========================
TestWorkers
========================
*/
void TestWorkers()
{
	idSysWorkerThreadGroup<idMyThread> workers( "myWorkers", 4 );
	for( ; ; )
	{
		for( int i = 0; i < workers.GetNumThreads(); i++ )
		{
			// workers.GetThread( i )-> // setup work for this thread
		}
		workers.SignalWorkAndWait();
	}
}
