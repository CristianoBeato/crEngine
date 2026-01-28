/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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
#ifndef __THREAD_H__
#define __THREAD_H__

// BEATO Begin:
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_atomic.h>
// BEATO End

/*
================================================
idSysMutex provides a C++ wrapper to the low level system mutex functions.  A mutex is an
object that can only be locked by one thread at a time.  It's used to prevent two threads
from accessing the same piece of data simultaneously.
================================================
*/
// BEATO Begin: USE portaable SDL_mutex structure
class idSysMutex
{
public:
	/// @brief Create the mutex object structure
	idSysMutex( void ) : m_handle( nullptr )
	{
		m_handle = SDL_CreateMutex();
		assert( !m_handle );
	}

	/// @brief free the mutex objec structure
	~idSysMutex( void )
	{
		if ( m_handle != nullptr )
		{
			SDL_DestroyMutex( m_handle );
			m_handle = nullptr;
		}
	}
	
	/// @brief Lock the mutex ( or just try ). 
	/// @param blocking if true, the caller will block until the mutex is available.
	/// If false will try to lock a mutex without blocking, if the mutex is not available,
	/// this function returns false immediately 
	/// @return on non blocking, true on success, false if the mutex would block.
	bool	Lock( const bool blocking = true )
	{
		if ( blocking )
		{
			SDL_LockMutex( m_handle );
			return true;
		}
		
		return SDL_TryLockMutex( m_handle );
	}

	/// @brief Unlock the mutex. 
	/// 
	void 	Unlock( void )
	{
		SDL_UnlockMutex( m_handle );
	}
	
private:
	SDL_Mutex*	m_handle;
	
	idSysMutex( const idSysMutex& s ) {}
	void			operator=( const idSysMutex& s ) {}
};

/*
================================================
idScopedCriticalSection is a helper class that automagically locks a mutex when it's created
and unlocks it when it goes out of scope.
================================================
*/
class idScopedCriticalSection
{
public:
	idScopedCriticalSection( idSysMutex& m ) : mutex( &m )
	{
		mutex->Lock();
	}
	~idScopedCriticalSection()
	{
		mutex->Unlock();
	}
	
private:
	idSysMutex* 	mutex;	// NOTE: making this a reference causes a TypeInfo crash
};

/*
================================================
idSysSignal is a C++ wrapper for the low level system signal functions.  A signal is an object
that a thread can wait on for it to be raised.  It's used to indicate data is available or that
a thread has reached a specific point.
================================================
*/
class idSysSignal
{
public:
	static const int32_t	WAIT_INFINITE = -1;
	
	idSysSignal( const bool manualReset = false );

	~idSysSignal( void );
	
	/// @brief Raise a waiting thread, or put a signal to next one
	/// that reach wait, that we already have a singal  
	void	Raise( void );

	/// @brief Reset the signal state
	void	Clear( void );
	
	/// @brief Wait for a specific signal, or consume a previous 
	/// Wait also clears the signalled
	/// state when the signalled state is reached within the time out period.
	/// @param timeout
	/// @return true if the object is in a signalled state and
	/// returns false if the wait timed out. 
	bool	Wait( const int32_t timeout = WAIT_INFINITE );	
private:
		// DG: all this stuff is needed to emulate Window's Event API
	//     (CreateEvent(), SetEvent(), WaitForSingleObject(), ...)
	bool				manualReset;
	volatile bool		signaled; // is it signaled right now?
	volatile int		waiting; // number of threads waiting for a signal
	SDL_Condition*		cond;
	SDL_Mutex*			mutex;

	idSysSignal( const idSysSignal& s ) {}
	void				operator=( const idSysSignal& s ) {}
};

/*
================================================
idSysInterlockedInteger is a C++ wrapper for the low level system interlocked integer
routines to atomically increment or decrement an integer.
================================================
*/
class idSysInterlockedInteger
{
public:
	idSysInterlockedInteger( void ) 
	{
		value.value = 0;
	}
	
	/// @brief atomically increments the integer and returns the new value
	int Increment( void )
	{
		return SDL_AddAtomicInt( &value, 1 ) + 1;
	}
	
	/// @brief atomically decrements the integer and returns the new value
	int Decrement( void )
	{
		return SDL_AddAtomicInt( &value, -1 ) - 1;
	}
	
	/// @brief atomically adds a value to the integer and returns the new value
	int Add( const int v )
	{
		return SDL_AddAtomicInt( &value, v ) + v;
	}
	
	/// @brief atomically subtracts a value from the integer and returns the new value
	int Sub( const int v )
	{
		return SDL_AddAtomicInt( &value, - v ) - v;
	}
	
	/// @brief returns the current value of the integer
	int GetValue( void ) const
	{
		return SDL_GetAtomicInt( const_cast<SDL_AtomicInt*>( &value ) );
	}

	/// @brief Compares two integers for equality and, if they are equal,
	/// replaces the first value, as an atomic operation.
	int CompareExchange( const int oldValue, const int newValue )
	{
		return SDL_CompareAndSwapAtomicInt( &value, oldValue, newValue );
	}
	
	/// @brief sets a new value
	void SetValue( const int v )
	{
		SDL_SetAtomicInt( &value, v );
	}
	
private:
	SDL_AtomicInt	value;
};

/*
================================================
idSysInterlockedPointer is a C++ wrapper around the low level system interlocked pointer
routine to atomically set a pointer while retrieving the previous value of the pointer.
================================================
*/
template< typename T >
class idSysInterlockedPointer
{
public:
	idSysInterlockedPointer() : ptr( nullptr ) {}
	
	// atomically sets the pointer and returns the previous pointer value
	T* 		Set( T* newPtr )
	{
		return static_cast<T*>( SDL_SetAtomicPointer( &ptr, newPtr ) );
	}
	
	// atomically sets the pointer to 'newPtr' only if the previous pointer is equal to 'comparePtr'
	// ptr = ( ptr == comparePtr ) ? newPtr : ptr
	T* 		CompareExchange( T* comparePtr, T* newPtr )
	{
		if( SDL_CompareAndSwapAtomicPointer( &ptr, comparePtr, newPtr ) )
			return comparePtr; // comapre are the old pointer
		else
			return ptr; // pointer has no change
		//return ( T* ) Sys_InterlockedCompareExchangePointer( ( void*& ) ptr, comparePtr, newPtr );
	}
	
	// returns the current value of the pointer
	T* Get( void ) const
	{
		return static_cast<T*>( SDL_GetAtomicPointer( &ptr ) );
	}
	
private:
	T* 		ptr;
};

/*
================================================
idSysThread is an abstract base class, to be extended by classes implementing the
idSysThread::Run() method.

	class idMyThread : public idSysThread {
	public:
		virtual int Run() {
			// run thread code here
			return 0;
		}
		// specify thread data here
	};

	idMyThread thread;
	thread.Start( "myThread" );

A worker thread is a thread that waits in place (without consuming CPU)
until work is available. A worker thread is implemented as normal, except that, instead of
calling the Start() method, the StartWorker() method is called to start the thread.
Note that the Sys_CreateThread function does not support the concept of worker threads.

	class idMyWorkerThread : public idSysThread {
	public:
		virtual int Run() {
			// run thread code here
			return 0;
		}
		// specify thread data here
	};

	idMyWorkerThread thread;
	thread.StartThread( "myWorkerThread" );

	// main thread loop
	for ( ; ; ) {
		// setup work for the thread here (by modifying class data on the thread)
		thread.SignalWork();           // kick in the worker thread
		// run other code in the main thread here (in parallel with the worker thread)
		thread.WaitForThread();        // wait for the worker thread to finish
		// use results from worker thread here
	}

In the above example, the thread does not continuously run in parallel with the main Thread,
but only for a certain period of time in a very controlled manner. Work is set up for the
Thread and then the thread is signalled to process that work while the main thread continues.
After doing other work, the main thread can wait for the worker thread to finish, if it has not
finished already. When the worker thread is done, the main thread can safely use the results
from the worker thread.
================================================
*/
class idSysThread
{
public:
	idSysThread( void );
	virtual			~idSysThread( void );
	
	const char* 	GetName( void ) const
	{
		return name.c_str();
	}

	uintptr_t		GetThreadHandle( void ) const
	{
		return reinterpret_cast<uintptr_t>( threadHandle );
	}

	bool			IsRunning( void ) const
	{
		return isRunning;
	}

	bool			IsTerminating( void ) const
	{
		return isTerminating;
	}
	
	//------------------------
	// Thread Start/Stop/Wait
	//------------------------
	
	bool			StartThread( const char* name, core_t core,
								 xthreadPriority priority = THREAD_NORMAL,
								 int stackSize = DEFAULT_THREAD_STACK_SIZE );
								 
	bool			StartWorkerThread( const char* name, core_t core,
									   xthreadPriority priority = THREAD_NORMAL,
									   int stackSize = DEFAULT_THREAD_STACK_SIZE );
									   
	void			StopThread( const bool wait = true );
	
	// This can be called from multiple other threads. However, in the case
	// of a worker thread, the work being "done" has little meaning if other
	// threads are continuously signalling more work.
	void			WaitForThread( void );
	
	//------------------------
	// Worker Thread
	//------------------------
	
	// Signals the thread to notify work is available.
	// This can be called from multiple other threads.
	void			SignalWork( void );
	
	// Returns true if the work is done without waiting.
	// This can be called from multiple other threads. However, the work
	// being "done" has little meaning if other threads are continuously
	// signalling more work.
	bool			IsWorkDone( void );
	
protected:
	// The routine that performs the work.
	virtual int		Run( void );
	
	bool			forceStop;

private:
	bool				isWorker;
	bool				isRunning;
	volatile bool		isTerminating;
	volatile bool		moreWorkToDo;
	idSysSignal			signalWorkerDone;
	idSysSignal			signalMoreWorkToDo;
	idSysMutex			signalMutex;
	idStr				name;
	SDL_ThreadPriority	priority;
	SDL_Thread*			threadHandle;
	
	static int		ThreadProc( idSysThread* thread );
	
	idSysThread( const idSysThread& s ) {}
	void			operator=( const idSysThread& s ) {}
};

/*
================================================
idSysWorkerThreadGroup implements a group of worker threads that
typically crunch through a collection of similar tasks.

	class idMyWorkerThread : public idSysThread {
	public:
		virtual int Run() {
			// run thread code here
			return 0;
		}
		// specify thread data here
	};

	idSysWorkerThreadGroup<idMyWorkerThread> workers( "myWorkers", 4 );
	for ( ; ; ) {
		for ( int i = 0; i < workers.GetNumThreads(); i++ ) {
			// workers.GetThread( i )-> // setup work for this thread
		}
		workers.SignalWorkAndWait();
		// use results from the worker threads here
	}

The concept of worker thread Groups is probably most useful for tools and compilers.
For instance, the AAS Compiler is using a worker thread group. Although worker threads
will work well on the PC, Mac and the 360, they do not directly map to the PS3,
in that the worker threads won't automatically run on the SPUs.
================================================
*/
template<class threadType>
class idSysWorkerThreadGroup
{
public:
	idSysWorkerThreadGroup( const char* name, int numThreads,
							xthreadPriority priority = THREAD_NORMAL,
							int stackSize = DEFAULT_THREAD_STACK_SIZE );
							
	virtual			~idSysWorkerThreadGroup();
	
	int				GetNumThreads() const
	{
		return threadList.Num();
	}
	threadType& 	GetThread( int i )
	{
		return *threadList[i];
	}
	
	void			SignalWorkAndWait();
	
private:
	idList<threadType*, TAG_THREAD>	threadList;
	bool					runOneThreadInline;	// use the signalling thread as one of the threads
	bool					singleThreaded;		// set to true for debugging
};

/*
========================
idSysWorkerThreadGroup<threadType>::idSysWorkerThreadGroup
========================
*/
template<class threadType>
ID_INLINE idSysWorkerThreadGroup<threadType>::idSysWorkerThreadGroup( const char* name,
		int numThreads, xthreadPriority priority, int stackSize )
{
	runOneThreadInline = ( numThreads < 0 );
	singleThreaded = false;
	numThreads = abs( numThreads );
	for( int i = 0; i < numThreads; i++ )
	{
		threadType* thread = new( TAG_THREAD ) threadType;
		thread->StartWorkerThread( va( "%s_worker%i", name, i ), ( core_t ) i, priority, stackSize );
		threadList.Append( thread );
	}
}

/*
========================
idSysWorkerThreadGroup<threadType>::~idSysWorkerThreadGroup
========================
*/
template<class threadType>
ID_INLINE idSysWorkerThreadGroup<threadType>::~idSysWorkerThreadGroup()
{
	threadList.DeleteContents();
}

/*
========================
idSysWorkerThreadGroup<threadType>::SignalWorkAndWait
========================
*/
template<class threadType>
ID_INLINE void idSysWorkerThreadGroup<threadType>::SignalWorkAndWait()
{
	if( singleThreaded )
	{
		for( int i = 0; i < threadList.Num(); i++ )
		{
			threadList[ i ]->Run();
		}
		return;
	}
	for( int i = 0; i < threadList.Num() - runOneThreadInline; i++ )
	{
		threadList[ i ]->SignalWork();
	}
	if( runOneThreadInline )
	{
		threadList[ threadList.Num() - 1 ]->Run();
	}
	for( int i = 0; i < threadList.Num() - runOneThreadInline; i++ )
	{
		threadList[ i ]->WaitForThread();
	}
}

/*
================================================
idSysThreadSynchronizer, allows a group of threads to
synchronize with each other half-way through execution.

	idSysThreadSynchronizer sync;

	class idMyWorkerThread : public idSysThread {
	public:
		virtual int Run() {
			// perform first part of the work here
			sync.Synchronize( threadNum );	// synchronize all threads
			// perform second part of the work here
			return 0;
		}
		// specify thread data here
		unsigned int threadNum;
	};

	idSysWorkerThreadGroup<idMyWorkerThread> workers( "myWorkers", 4 );
	for ( int i = 0; i < workers.GetNumThreads(); i++ ) {
		workers.GetThread( i )->threadNum = i;
	}

	for ( ; ; ) {
		for ( int i = 0; i < workers.GetNumThreads(); i++ ) {
			// workers.GetThread( i )-> // setup work for this thread
		}
		workers.SignalWorkAndWait();
		// use results from the worker threads here
	}

================================================
*/
class idSysThreadSynchronizer
{
public:
	static const int	WAIT_INFINITE = -1;
	
	ID_INLINE	void			SetNumThreads( const uint32_t num );
	ID_INLINE	void			Signal( const uint32_t threadNum );
	ID_INLINE	bool			Synchronize( const uint32_t threadNum, int32_t timeout = WAIT_INFINITE );
	
private:
	idList< idSysSignal*, TAG_THREAD >		signals;
	idSysInterlockedInteger		busyCount;
};

/*
========================
idSysThreadSynchronizer::SetNumThreads
========================
*/
ID_INLINE void idSysThreadSynchronizer::SetNumThreads( const uint32_t num )
{
	assert( busyCount.GetValue() == signals.Num() );
	if( num != signals.Num() )
	{
		signals.DeleteContents();
		signals.SetNum( ( int )num );
		for( uint32_t i = 0; i < num; i++ )
		{
			signals[i] = new( TAG_THREAD ) idSysSignal();
		}
		busyCount.SetValue( num );
		SYS_MEMORYBARRIER;
	}
}

/*
========================
idSysThreadSynchronizer::Signal
========================
*/
ID_INLINE void idSysThreadSynchronizer::Signal( const uint32_t threadNum )
{
	if( busyCount.Decrement() == 0 )
	{
		busyCount.SetValue( signals.Num() );
		SYS_MEMORYBARRIER;
		for( uint32_t i = 0; i < signals.Num(); i++ )
		{
			signals[i]->Raise();
		}
	}
}

/*
========================
idSysThreadSynchronizer::Synchronize
========================
*/
ID_INLINE bool idSysThreadSynchronizer::Synchronize( const uint32_t threadNum, const int32_t timeout )
{
	return signals[threadNum]->Wait( timeout );
}

#endif // !__THREAD_H__
