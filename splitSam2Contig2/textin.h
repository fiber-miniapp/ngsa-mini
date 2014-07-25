/*
	File:	textin.h
	Copyright(C) 2009-2012 RIKEN, Japan.
*/
#ifndef __textin_H__
#define __textin_H__

#include <cstdio>
#include <sstream>
#include <string>

#include "my_types.h"

template <class T>
static bool
StringTo(
		const std::string&	in,
		T&					out )
{
	std::stringstream	tSS;
	std::string			tTmp;

	tSS.str( in );
	tSS.clear();
	tSS >> out;
	if ( tSS.eof() ) {
		return true;
	} else if ( tSS.fail() ) {
		return false;
	} else {
		tSS >> tTmp;
		return ( tSS.eof() && ( tTmp.size() == 0 ) );
	}
}

class CTextIn {
private:
	FILE*	m_in;
	int		m_lastChar;

public:
				CTextIn();
				CTextIn(
						const char*	inTextFilename );
	virtual		~CTextIn();

	bool		IsValid() const {
					return m_in != NULL;
				};

	bool		Open(	const char*	inTextFilename );
	void		Close();

	__Xint64	GetSize() const;
	__Xint64	GetPosition() const {
					if ( m_in != NULL ) {
						return (__Xint64)ftello( m_in );
					} else {
						return -1;
					}
				};
	bool		SetPosition( __Xint64	inPos ) {
					if ( m_in != NULL ) {
						if ( fseeko( m_in, (off_t)inPos, 0/*SEEK_SET*/ ) == 0 ) {
							return true;
						}
					}
					return false;
				};

	const char*	GetLine(
						std::string&	outLine );

	unsigned long
				GetLine(
						int					inDelimiterChar,
						__string_vector&	outWords );

	unsigned long
				GetLine(
						int						inDelimiterChar,
						const __uint32_vector&	inGetList,
						__string_vector&		outWords );

	static int	WordParse(
						const char*			inLine,
						int					inDelimiterChar,
						__string_vector&	outWords );
	static int	TestDelimiterChar(
						const char*	inFilename );
	static int	TestDelimiterCharFromText(
						const char*	inText );
	static std::string
				Trim(	const std::string&	inStr,
						const char*			inTrimChars = " " ) {
					std::string::size_type	tF = inStr.find_first_not_of( inTrimChars );
					if ( tF != std::string::npos ) {
						std::string::size_type	tL = inStr.find_last_not_of( inTrimChars );
						return inStr.substr( tF, tL - tF + 1 );
					}
					return std::string();
				};
	static std::string
				Trim(	const char*	inStr,
						const char*	inTrimChars = " " ) {
					std::string	tStr = inStr;
					std::string::size_type	tF = tStr.find_first_not_of( inTrimChars );
					if ( tF != std::string::npos ) {
						std::string::size_type	tL = tStr.find_last_not_of( inTrimChars );
						return tStr.substr( tF, tL - tF + 1 );
					}
					return std::string();
				};
};

#endif	//__textin_H__
