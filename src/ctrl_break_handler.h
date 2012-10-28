#pragma once
#include <windows.h>
#include <iostream>

class Ctrl_break_handler{
	static BOOL CtrlHandler( DWORD fdwCtrlType )
	{ 
		switch( fdwCtrlType ) 
		{ 
			case CTRL_C_EVENT:
			case CTRL_CLOSE_EVENT: 
			case CTRL_BREAK_EVENT: 
			case CTRL_LOGOFF_EVENT: 
			case CTRL_SHUTDOWN_EVENT: 
				printf("Break event\n");
				ctrl_break_pressed = true;
				return TRUE; 
		} 
		printf("Unknown event!!!\n");
		return FALSE; 
	} 
public:
	static int ctrl_break_pressed;

	Ctrl_break_handler(){
		ctrl_break_pressed = false;

		if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE) Ctrl_break_handler::CtrlHandler, TRUE ) ) {
			std::cout << "ERROR: Could not set control handler\n";
			throw std::exception();
		}
		std::cout << "Control Handler (Ctrl-Break interception) is installed.\n";
	}
	~Ctrl_break_handler(){
		SetConsoleCtrlHandler( (PHANDLER_ROUTINE) Ctrl_break_handler::CtrlHandler, FALSE );
	}
};

int Ctrl_break_handler::ctrl_break_pressed=0;