/*-------------------------------------------------------------------------------
Copyright (c)2001. All rights reserved.

  Authors:
  Artyom V. Borodkin
-------------------------------------------------------------------------------*/

#include "hl/thread_object.h"

namespace	concurrency{
	
	// Begin the execution of a thread.
	int thread_object::start (){
		
		if( !hThread )hThread = CreateThread(NULL,0,ThreadProc, this ,NULL,&ThreadID);	
		
		if( hThread ) return 1;
		
		return 0;
	}
	bool thread_object::is_current(){
		return ThreadID == GetCurrentThreadId();
	}
	
	void thread_object::wait_for_exit(unsigned int timeout){
		WaitForSingleObject(hThread,(DWORD)timeout);
	}

	// Finish the execution of a thread.
	int thread_object::stop (unsigned int timeout){
		BOOL result = 0;
		if( is_current() ) ExitThread(0);
		
		if( hThread ){			
			if(WaitForSingleObject(hThread,(DWORD)timeout) != WAIT_OBJECT_0){
				result = TerminateThread(hThread, 0);
			}						
			CloseHandle(hThread);
			hThread = NULL;			
		}
		return result;
	}
	
	// Continue the execution of a previously suspended thread.
	int thread_object::resume (){
		return false;		
	}
	
	// Suspend the execution of a thread.
	int thread_object::suspend (){
		return false;		
	}
}// namespace	concurrency