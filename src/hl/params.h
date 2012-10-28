#pragma once

/*
	params all_params( GetCommandLine() );

	std::string tcp_log_addr;
	int			tcp_port=0;

	if( all_params.read("-tcp_log:([^ :]+):([0-9]+)", tcp_log_addr, tcp_port) ){
		std::cout << "Tcp log on (address= "<< tcp_log_addr << ":" << tcp_port<< ")\n";	
	}

	int steps=0;
	if( all_params.read("-steps:([^ ]*)", steps) ){
		std::cout << "Number of steps= "<< steps << "\n";
	}

*/
#include "hl/regexp.h"
#include <string>
class params{
	std::string _cmd_line;
public:
	params(const std::string& cmd_line):_cmd_line(cmd_line){}
	
	int check(const std::string& _regex, bool required = true){
		regexp	rg(_regex);
		if( rg.match(_cmd_line) ) return 1;
		if(required)
			throw std::exception( (std::string("Param missed: ")+_regex).c_str() );
		return false;
	}
	int read(const std::string& _regex, int& param1, int& param2, bool required = true){
		regexp	rg(_regex);
		if( rg.match(_cmd_line) ){
			param1 = atoi(rg[1].c_str());
			param2 = atoi(rg[2].c_str());
			return 1;
		}
		if(required)
			throw std::exception( (std::string("Param missed: ")+_regex).c_str() );
		return false;
	}
	int read(const std::string& _regex, int& param, bool required = true){
		regexp	rg(_regex);
		if( rg.match(_cmd_line) ){
			param = atoi(rg[1].c_str());
			return 1;
		}
		if(required)
			throw std::exception( (std::string("Param missed: ")+_regex).c_str() );
		return false;
	}
	int read(const std::string& _regex, std::string& param1, std::string& param2, bool required = true){
		regexp	rg(_regex);
		if( rg.match(_cmd_line) ){
			param1 = rg[1];
			param2 = rg[2];
			return 1;
		}
		if(required)
			throw std::exception( (std::string("Param missed: ")+_regex).c_str() );
		return false;
	}
	int read(const std::string& _regex, std::string& param, bool required = true){
		regexp	rg(_regex);
		if( rg.match(_cmd_line) ){
			param = rg[1];
			return 1;
		}
		if(required)
			throw std::exception( (std::string("Param missed: ")+_regex).c_str() );
		return false;
	}
	int read(const std::string& _regex, std::string& param1, int& param2, bool required = true){
		regexp	rg(_regex);
		if( rg.match(_cmd_line) ){
			param1 = rg[1];
			param2 = atoi(rg[2].c_str());
			return 1;
		}
		if(required)
			throw std::exception( (std::string("Param missed: ")+_regex).c_str() );
		return false;
	}
};
