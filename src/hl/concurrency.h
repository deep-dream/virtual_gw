/*=============================================================================
    concurrency.h:	(created:	14:05:2000   12:01)

        Copyright (c) 2000.  All Rights Reserved.

    Authors:
        Artyom V. Borodkin         (http://www.deep-dream.com)
=============================================================================*/
#ifndef _concurrency_h_
#define _concurrency_h_
#define WIN32_LEAN_AND_MEAN
//#define WINVER 0x0500
//#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <string>
/*-----------------------------------------------------------------------------
	Synchronization tools
-----------------------------------------------------------------------------*/
namespace	concurrency{

	class	cs_object{
		CRITICAL_SECTION CriticalSection;
	public:
		cs_object(){InitializeCriticalSection(&CriticalSection);}
		~cs_object(){DeleteCriticalSection(&CriticalSection);}
//		bool	try_enter(){ return TryEnterCriticalSection(&CriticalSection); }
		void	enter(){EnterCriticalSection(&CriticalSection);}
		void	leave(){LeaveCriticalSection(&CriticalSection);}
	};
	
	template<class T = cs_object>
		class	lock{
		T* ptr;
	public:
		lock(T* p):ptr(p){ ptr->enter(); }
		~lock(){ ptr->leave(); }
	};
	
	template<class T, class L = cs_object>
		class	locking_ptr: private lock<L>{
		T*	ptr;
		locking_ptr& operator = (const locking_ptr&);
	public:
		locking_ptr(T* p,cs_object* cs_o):lock<L>(cs_o),ptr(p){}
		T*	operator->(){return ptr;}
	};	

	class	event{
		std::string name;
		HANDLE handle;
	public:
		event(bool initial_state = false, bool manual_reset = true, std::string _name = ""):
		  name(_name){
			const char* c_str = NULL;
			if( name.length() ) c_str = name.c_str();
			handle = CreateEvent(NULL,manual_reset,initial_state,c_str);
		}
		unsigned wait_for_event( DWORD Milliseconds = INFINITE ){
			 return WaitForSingleObject( handle, Milliseconds );
		}
		bool isSignaled( DWORD Milliseconds = 0 ){
			return wait_for_event( Milliseconds ) == WAIT_OBJECT_0;
		}
		void Set(){ SetEvent(handle); }
		void Reset(){ ResetEvent(handle); }
		void Pulse(){ PulseEvent(handle); }
		~event(){ CloseHandle( handle ); }
	};
}
#endif// _concurrency_h_