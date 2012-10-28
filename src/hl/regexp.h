#pragma once

#define _GNU_SOURCE
extern "C"{
#include <sys/types.h>
#include "regex.h"
}
#include <stdio.h>
#include <string.h>

class regexp{
	std::string		_str;
	std::string		_pattern;
	regex_t			preg;
	regmatch_t		matches[10];
	int				errcode;
public:
	regexp(const std::string& pattern):_pattern(pattern){
		regcomp(&preg, _pattern.c_str(), REG_EXTENDED );	
	}
	~regexp(){
		regfree(&preg);
	}

	int  match(const std::string& str){
		_str = str;
		return (errcode = regexec(&preg, _str.c_str(), 10,  matches, 0)) == 0;
	}
	std::string operator[](int i){
		if( matches[i].rm_so < 0) return "";
		return _str.substr(matches[i].rm_so,matches[i].rm_eo-matches[i].rm_so);
	}
	std::string error(){		
		char buffer[1024];
		regerror(errcode,  &preg, buffer, sizeof(buffer) );
		return std::string(buffer) + ":" + _pattern;
	}
};