#pragma once

std::string print(const char *format, ...)
{
	std::vector<char> d;
	d.resize(1024);
	va_list marker;
	va_start( marker, format );
	vsprintf_s(&d[0], d.size(), format, marker ); 
//	vsprintf(&d[0], format, marker ); 
	va_end( marker );
	d.resize(strlen(&d[0]));
	return std::string( d.begin(), d.end() );
}
