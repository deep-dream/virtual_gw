/*-------------------------------------------------------------------------------
Copyright (c)2001. All rights reserved.

  Authors:
  Artyom V. Borodkin
-------------------------------------------------------------------------------*/
#ifndef _THREAD_OBJECT_H_
#define _THREAD_OBJECT_H_
#include<windows.h>
#include "concurrency.h"

namespace	concurrency{
	
	class thread_object{
		DWORD		ThreadID;
		HANDLE		hThread;
	protected:
		virtual unsigned long main (){ return 0; };
	public:
		virtual ~thread_object(){ stop(0); }
		thread_object():hThread(NULL),ThreadID(0){}
		// Begin the execution of a thread.
		virtual int start ();
		
		// Finish the execution of a thread.
		virtual int stop (unsigned int timeout = 0 );
		
		// Continue the execution of a previously suspended thread.
		virtual int resume ();
		
		// Suspend the execution of a thread.
		virtual int suspend ();

		bool is_current();
		
		virtual void wait_for_exit(unsigned int timeout = INFINITE);
		
	private:
		static    unsigned long __stdcall ThreadProc(void* lpParameter){
			if(lpParameter) return ((thread_object*)lpParameter)->main();
			return 0;
		}
	};
} //namespace	concurrency
#endif // THREAD_OBJECT_H_
