/*
	File:	textin.cpp
	Copyright(C) 2009-2010 RIKEN, Japan.
*/

#include <sys/stat.h>
#include "textin.h"

CTextIn::CTextIn()
{
	m_in = NULL;
}

CTextIn::CTextIn(
		const char* inTextFilename )
{
	m_in = NULL;
	Open( inTextFilename );
}

CTextIn::~CTextIn()
{
	Close();
}

bool
CTextIn::Open(
		const char*	inTextFilename )
{
	if ( m_in != NULL ) {
		Close();
	}

	m_in = fopen( inTextFilename, "r" );
	m_lastChar = 0;

	return m_in != NULL;
}

void
CTextIn::Close()
{
	if ( m_in != NULL ) {
		fclose( m_in );
		m_in = NULL;
	}
}

__Xint64
CTextIn::GetSize() const
{
	if ( m_in != NULL ) {
		struct stat	tStat;
		if ( fstat( fileno( m_in ), &tStat ) == 0 ) {
			return (__Xint64)tStat.st_size;
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

const char*
CTextIn::GetLine(
		std::string&	outLine )
{
	int	c;
	int	tCount;

	outLine.clear();
	tCount = 0;

	if ( IsValid() && !feof( m_in ) ) {
		while ( ( c = fgetc( m_in ) ) != EOF ) {
			if ( c & 0xf0 ) {
				outLine += c;
				++tCount;
			} else if ( ( m_lastChar == 0x0d ) && ( c == 0x0a ) ) {
				//skip
			} else if ( ( c == 0x0a ) || ( c == 0x0d ) ) {
				++tCount;
				break;
			} else {
				outLine += c;
				++tCount;
			}
		}
		m_lastChar = c;
	}

	if ( tCount > 0 ) {
		return outLine.c_str();
	} else {
		return NULL;
	}
}

unsigned long
CTextIn::GetLine(
		int					inDelimiterChar,
		__string_vector&	outWords )
{
	register int	c;
	__string_vector::iterator	tWordPtr;
	std::string::size_type		tAlloc = 100;
	std::string::size_type		tUsed = 0;

	if ( IsValid() && !feof( m_in ) ) {
		if ( outWords.size() < tAlloc ) {
			outWords.resize( tAlloc, "" );
		} else {
			tAlloc = outWords.size();
		}

		tWordPtr = outWords.begin();
		(*tWordPtr).clear();
		while ( ( c = fgetc( m_in ) ) != EOF ) {
			if ( c == inDelimiterChar ) {
				if ( ++tUsed >= tAlloc ) {
					outWords.resize( ( tAlloc += 100 ), "" );
					tWordPtr = outWords.begin() + tUsed;
				} else {
					++tWordPtr;
				}
				(*tWordPtr).clear();
			} else if ( c & 0xf0 ) {
				(*tWordPtr) += c;
			} else if ( ( m_lastChar == 0x0d ) && ( c == 0x0a ) ) {
				//skip
			} else if ( ( c == 0x0a ) || ( c == 0x0d ) ) {
				break;
			} else {
				(*tWordPtr) += c;
			}
		}
		m_lastChar = c;
		++tUsed;
	} else {
		tUsed = 0;
	}

	return tUsed;
}

unsigned long
CTextIn::GetLine(
		int						inDelimiterChar,
		const __uint32_vector&	inGetList,
		__string_vector&		outWords )
{
	register int	c;
	__string_vector::iterator		tWordPtr;
	__uint32_vector::const_iterator	tGetIter;
	std::string::size_type		tGetCount = inGetList.size();
	std::string::size_type		i = 0;

	if ( IsValid() && !feof( m_in ) ) {
		if ( outWords.size() < tGetCount ) {
			outWords.resize( tGetCount, "" );
		}

		tWordPtr = outWords.begin();
		tGetIter = inGetList.begin();
		(*tWordPtr).clear();
		i = 0;
		while ( ( c = fgetc( m_in ) ) != EOF ) {
			if ( c == inDelimiterChar ) {
				if ( i++ == (*tGetIter) ) {
					if ( ++tGetIter == inGetList.end() ) {
						break;
					}
				}
				if ( i == (*tGetIter) ) {
					++tWordPtr;
					(*tWordPtr).clear();
				}
			} else if ( c & 0xf0 ) {
				if ( i == (*tGetIter) ) {
					(*tWordPtr) += c;
				}
			} else if ( ( m_lastChar == 0x0d ) && ( c == 0x0a ) ) {
				//skip
			} else if ( ( c == 0x0a ) || ( c == 0x0d ) ) {
				break;
			} else {
				if ( i == (*tGetIter) ) {
					(*tWordPtr) += c;
				}
			}
		}
		if ( c == inDelimiterChar ) {
			while ( ( c = fgetc( m_in ) ) != EOF ) {
				if ( c == inDelimiterChar ) {
					++i;
				} else if ( ( c == 0x0a ) || ( c == 0x0d ) ) {
					break;
				}
			}
		}
		m_lastChar = c;
		++i;
	} else {
		i = 0;
	}

	return i;
}

int
CTextIn::WordParse(
		const char*			inLine,
		int					inDelimiterChar,
		__string_vector&	outWords )
{
	register int	c;
	__string_vector::iterator	tWordPtr;
	std::string::size_type		tAlloc = 100;
	std::string::size_type		tUsed = 0;

	if ( outWords.size() < tAlloc ) {
		outWords.resize( tAlloc, "" );
	} else {
		tAlloc = outWords.size();
	}

	tWordPtr = outWords.begin();
	(*tWordPtr).clear();
	while ( ( c = *inLine++ ) != 0 ) {
		if ( c == inDelimiterChar ) {
			if ( ++tUsed >= tAlloc ) {
				outWords.resize( ( tAlloc += 100 ), "" );
				tWordPtr = outWords.begin() + tUsed;
			} else {
				++tWordPtr;
			}
			(*tWordPtr).clear();
		} else {
			(*tWordPtr) += c;
		}
	}
	++tUsed;
	if ( tUsed < tAlloc ) {
		outWords.resize( tUsed, "" );
	}

	return outWords.size();
}

int
CTextIn::TestDelimiterChar(
		const char*	inFilename )
{
	CTextIn			tIn;
	std::string		tBuf;

	if ( tIn.Open( inFilename ) ) {
		if ( tIn.GetLine( tBuf ) != NULL ) {
			return TestDelimiterCharFromText( tBuf.c_str() );
		}
	}
	return 0;
}

int
CTextIn::TestDelimiterCharFromText(
		const char*	inText )
{
	__string_vector	tWords;

	if ( CTextIn::WordParse( inText, '\t', tWords ) > 1 ) {
		return '\t';
	} else if ( CTextIn::WordParse( inText, ',', tWords ) > 1 ) {
		return ',';
	}
	return 0;
}
