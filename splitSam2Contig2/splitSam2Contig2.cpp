/*
	File:	splitSam2Contig2.cpp
	Copyright(C) 2012-2013 RIKEN, Japan.
*/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <string>

#ifndef NAN
#include <limits>
#define NAN	std::numeric_limits<double>::quiet_NaN()
#endif

#if _TARGET_RICC == 1
#define ISNAN	isnan
#define ISFINITE	isfinite
#else
#define ISNAN	std::isnan
#define ISFINITE	std::isfinite
#endif

#include "my_types.h"
#include "textin.h"

#define OUT_BUFFER_SIZE	0x200000L

struct _ContigInfo {
	std::string	scChr;
	__Xint32	scStart;
	__Xint32	scEnd;
};
typedef struct _ContigInfo ContigInfo;
typedef	std::map<std::string, ContigInfo>	ContigInfoMap;

class CSeqContig {
private:
	static std::string	s_nochr;
	ContigInfoMap	m_infoMap;

public:
	bool		Load(	const char*	inSeqContigFile );

	const ContigInfo*
				GetInfo( const std::string&	inContigName ) const {
					ContigInfoMap::const_iterator	tFind = m_infoMap.find( inContigName );
					if ( tFind != m_infoMap.end() ) {
						return &tFind->second;
					}
					return NULL;
				};
};

std::string
CSeqContig::s_nochr = "";

bool
CSeqContig::Load(
		const char*	inSeqContigFile )
{
	bool			tDone = false;
	CTextIn			tIn;
	int				tDelimiterChar = CTextIn::TestDelimiterChar( inSeqContigFile );
	__string_vector	tWords;
	ContigInfo		tInfo;

	if ( tIn.Open( inSeqContigFile ) ) {
		while ( tIn.GetLine( tDelimiterChar, tWords ) >= 6 ) {
			if ( StringTo( tWords[2], tInfo.scStart ) &&
				 StringTo( tWords[3], tInfo.scEnd ) ) {
				tInfo.scChr = tWords[1];
				m_infoMap.insert( ContigInfoMap::value_type( tWords[5], tInfo ) );
			}
		}
		tIn.Close();
		tDone = m_infoMap.size() > 0;
	}
	return tDone;
}

class CPairedRead {
private:
	__string_vector	m_read[2];
	unsigned long	m_count[2];
	int				m_mapScore;
	std::string		m_contig[2];
	int				m_sameChr;
	const
	ContigInfo*		m_contigInfo[2];
	int				m_flag[2];
	int				m_mapStat;

public:
	bool	Load(	const CSeqContig&	inSC,
					CTextIn&			inIn,
					int					inDelimiterChar );

	const char*
			GetContigName(
					int	inIdx ) const {
				return m_contig[inIdx].c_str();
			};
	const char*
			GetMappedContig() const;
	double	GetBQAverage(
					int	inIdx ) const;
	int		GetFragmentLength() const;
	int		GetMQ(	int	inIdx ) const;
	int		GetPos(	int	inIdx ) const;
	int		GetMappedValue() const;
	int		GetMappedValue();
	const char*
			GetTagValue(
					int			inIdx,
					const char*	inTag ) const;
	void	Append(	int			inIdx,
					const char*	inAttr );
	const char*
			GetLine(
					int				inIdx,
					std::string&	outBuf ) const;
	void	PutLine(
					int				inIdx,
					std::ostream&	inOut ) const;
};

bool
CPairedRead::Load(
		const CSeqContig&	inSC,
		CTextIn&			inIn,
		int					inDelimiterChar ) {
	__string_vector	tWords;

	while ( ( ( m_count[0] = inIn.GetLine( inDelimiterChar, m_read[0] ) ) > 0 ) &&
			( m_read[0][0] == "@SQ" ) ) {
	}
	if ( m_count[0] >= 11 &&
		 ( m_count[1] = inIn.GetLine( inDelimiterChar, m_read[1] ) ) >= 11 ) {
		if ( m_read[0][0] != m_read[1][0] ) {
			std::cerr << "[ERROR] Bad records found at " << m_read[0][0] << " and " << m_read[1][0] << std::endl;
			return false;
		}
		m_mapScore = 0;
		for ( int i = 0; i < 2; ++i ) {
			if ( ( StringTo( m_read[i][1], m_flag[i] ) && m_flag[i] & 4 ) ||
				 m_read[i][2] == "*" ) {
				++m_mapScore;
			}
			m_contig[i] = m_read[i][2];
			if ( m_contig[i] != "*" ) {
				std::string::size_type	tBegin, tLast;
				if ( ( tBegin = m_contig[i].find( "ref|", 0 ) ) != std::string::npos &&
					 ( tLast = m_contig[i].find( "|", tBegin + 4 ) ) != std::string::npos ) {
					m_contig[i] = m_contig[i].substr( tBegin + 4, tLast - tBegin - 4 );
				}
				m_contigInfo[i] = inSC.GetInfo( m_contig[i] );
				if ( m_contigInfo[i] == NULL ) {
					std::cerr << "[ERROR] Bad records found at " << m_read[0][0] << " and " << m_read[1][0] << std::endl;
					return false;
				}
			} else {
				m_contigInfo[i] = NULL;
			}
		}
		if ( m_mapScore == 0 &&
			 ( m_flag[0] & 8 || m_flag[1] & 8 ) ) {
			m_mapScore = 1;
		}

		if ( m_contig[0] == m_contig[1] ) {
			m_sameChr = 1;
		} else if ( m_contigInfo[0]->scChr == m_contigInfo[1]->scChr ) {
			m_sameChr = 2;
		} else {
			m_sameChr = 0;
		}
		m_mapStat = -1;

		return true;
	}
	return false;
}

const char*
CPairedRead::GetMappedContig() const
{
	int	tMapped = GetMappedValue();
	switch ( tMapped ) {
	case 1:
		if ( m_sameChr == 1 || GetPos( 0 ) < GetPos( 1 ) ) {
			return m_contig[0].c_str();
		} else {
			return m_contig[1].c_str();
		}
		break;
	case 2: case 3: case 4:
		return "invert";
	case 5:
		return "interchromo";
	case 6:
		return "single";
	case 7:
		return "unmap";
	default:
		std::cerr << "Can not parse mapping position!!" << std::endl;
		exit( -1 );
	}
}

double
CPairedRead::GetBQAverage(
		int	inIdx ) const
{
	std::string::const_iterator	tIter = m_read[inIdx][10].begin();
	int	tSum = 0;
	while ( tIter != m_read[inIdx][10].end() ) {
		tSum += ( (int)*tIter - 33 );
		++tIter;
	}
	return (double)tSum / (double)m_read[inIdx][10].size();
}

static int
SumCIGAR( const std::string&	inCIGAR )
{
	std::stringstream	tSS;
	int		tSum = 0;
	int		i;
	char	c;
	tSS.str( inCIGAR );
	tSS.clear();
	while ( ! tSS.fail() && ! tSS.eof() ) {
		tSS >> i;
		if ( ! tSS.fail() && ! tSS.eof() ) {
			tSS >> c;
			if ( c != 'S' ) {
				tSum += i;
			}
		}
	}
	return tSum;
}

int
CPairedRead::GetFragmentLength() const
{
	int	tVal[2];

	if ( m_sameChr == 1 ) {
		if ( StringTo( m_read[0][8], tVal[0] ) ) {
			return std::abs( tVal[0] );
		} else {
			return -1;
		}
	}

	if ( m_sameChr == 0 || m_mapScore > 0 ) {
		return -1;//NA
	}

	if ( StringTo( m_read[0][3], tVal[0] ) &&
		 StringTo( m_read[1][3], tVal[1] ) ) {
		__Xint32	tPos[4], tMax, tMin;
		tPos[0] = m_contigInfo[0]->scStart + tVal[0] - 1;
		tPos[1] = m_contigInfo[1]->scStart + tVal[1] - 1;
		tPos[2] = tPos[0] + SumCIGAR( m_read[0][5] ) - 1;
		tPos[3] = tPos[1] + SumCIGAR( m_read[1][5] ) - 1;

		tMax = tMin = tPos[0];
		for ( int i = 1; i < 4; ++i ) {
			if ( tPos[i] > tMax ) {
				tMax = tPos[i];
			} else if ( tPos[i] < tMin ) {
				tMin = tPos[i];
			}
		}
		return tMax - tMin + 1;
	}
	return 0;
}

int
CPairedRead::GetMQ(
		int	inIdx ) const
{
	int	tVal;

	if ( StringTo( m_read[inIdx][4], tVal ) ) {
		return tVal;
	} else {
		return 0;
	}
}

int
CPairedRead::GetPos(
		int	inIdx ) const
{
	int	tVal;

	if ( StringTo( m_read[inIdx][3], tVal ) ) {
		return m_contigInfo[0]->scStart + tVal - 1;
	}
	return 0;
}

int
CPairedRead::GetMappedValue() const
{
	if ( m_mapStat >= 0 ) {
		return m_mapStat;
	}

	int	tStat;

	if ( m_mapScore == 2 ) {
		tStat = 7; //UM
	} else if ( m_mapScore == 1 ) {
		tStat = 6; //MU
	} else if ( m_flag[0] & 2 ) {
		tStat = 1; //FR
	} else if ( m_sameChr == 0 ) {
		tStat = 5; //nterchromo
	} else {
		int		tPos1 = GetPos( 0 );
		int		tPos2 = GetPos( 1 );
		char	tFt, tBt;
		if ( tPos1 <= tPos2 ) {
			if ( m_flag[0] & 0x10 ) {
				tFt = 'R';
			} else {
				tFt = 'F';
			}
			if ( m_flag[1] & 0x10 ) {
				tBt = 'R';
			} else {
				tBt = 'F';
			}
		} else {
			if ( m_flag[1] & 0x10 ) {
				tFt = 'R';
			} else {
				tFt = 'F';
			}
			if ( m_flag[0] & 0x10 ) {
				tBt = 'R';
			} else {
				tBt = 'F';
			}
		}
		if ( tFt == 'F' && tBt == 'R' ) {
			tStat = 1;
		} else if ( tFt == 'R' && tBt == 'F' ) {
			tStat = 2;
		} else if ( tFt == 'F' && tBt == 'F' ) {
			tStat = 3;
		} else if ( tFt == 'R' && tBt == 'R' ) {
			tStat = 4;
		} else {
			tStat = 0; //unknown
		}
	}
	return tStat;
}

int
CPairedRead::GetMappedValue()
{
	if ( m_mapStat >= 0 ) {
		return m_mapStat;
	}

	m_mapStat = ((const CPairedRead*)this)->GetMappedValue();
	return m_mapStat;
}

const char*
CPairedRead::GetTagValue(
		int			inIdx,
		const char*	inTag ) const
{
	int	i = 11;
	while ( i < m_read[inIdx].size() ) {
		if ( m_read[inIdx][i].find( inTag, 0 ) == 0 ) {
			return m_read[inIdx][i].c_str();
		}
		++i;
	}
	return NULL;
}

void
CPairedRead::Append(
		int			inIdx,
		const char*	inAttr )
{
	if ( m_count[inIdx] < m_read[inIdx].size() ) {
		m_read[inIdx][m_count[inIdx]] = inAttr;
	} else {
		m_read[inIdx].push_back( inAttr );
	}
	++m_count[inIdx];
}

const char*
CPairedRead::GetLine(
		int				inIdx,
		std::string&	outBuf ) const
{
	std::stringstream	tSS;
	__string_vector::const_iterator
						tIter = m_read[inIdx].begin();
	unsigned long		tCount = 1;

	tSS << *tIter++;
	while ( tCount < m_count[inIdx] &&
			tIter != m_read[inIdx].end() ) {
		tSS << '\t' << *tIter++;
		++tCount;
	}
	outBuf = tSS.str();
	return outBuf.c_str();
}

void
CPairedRead::PutLine(
		int				inIdx,
		std::ostream&	inOut ) const
{
	__string_vector::const_iterator
						tIter = m_read[inIdx].begin();
	unsigned long		tCount = 1;

	inOut << *tIter++;
	while ( tCount < m_count[inIdx] &&
			tIter != m_read[inIdx].end() ) {
		inOut << '\t' << *tIter++;
		++tCount;
	}
	inOut << '\n'; //std::endl;
}

class COutFileInfo {
public:
	std::ofstream	fiOut;
	int				fiCount;

private:
	__int8_array	mBuf;

public:
	COutFileInfo() {
		fiCount = 0;
	};

	~COutFileInfo() {
		Close();
	}

	bool	Open(	const char*	inOutFilename ) {
				fiOut.open( inOutFilename );
				mBuf.alloc( OUT_BUFFER_SIZE );
				fiOut.rdbuf()->pubsetbuf( (char*)mBuf.ptr, OUT_BUFFER_SIZE );
				return fiOut.is_open();
			}

	void	Close() {
				if ( fiOut.is_open() ) {
					fiOut.close();
				}
			}
};
typedef std::map<std::string, COutFileInfo*>	FileInfoMap;

int
main(	int		inArgc,
		char**	inArgv )
{
	FileInfoMap	tFMap;
	std::string	tBuf;

	if ( inArgc >= 4 ) {
		std::string	tOutDir = inArgv[3];
		if ( tOutDir.find_last_of( '/' ) != tOutDir.length() - 1 ) {
			tOutDir += '/';
		}

		CSeqContig	tSC;
		if ( tSC.Load( inArgv[1] ) ) {
			CTextIn	tIn;
			int		tDelimiterChar = CTextIn::TestDelimiterChar( inArgv[2] );
			if ( tIn.Open( inArgv[2] ) ) {
				std::stringstream	tSS;
				std::vector<CPairedRead>	tReads( 10000 );
				int	tLoaded = tReads.size();
				int	i = 0;
				while ( tLoaded == tReads.size() ) {
					tLoaded = 0;
					while ( tLoaded < tReads.size() &&
							tReads[tLoaded].Load( tSC, tIn, tDelimiterChar ) ) {
						++tLoaded;
					}
					for ( int tLoad = 0; tLoad < tLoaded; ++tLoad ) {
						const
						char*	tContig;
						double	tBQ[2] = { tReads[tLoad].GetBQAverage( 0 ),
											tReads[tLoad].GetBQAverage( 1 ) };
						int		tMapped = tReads[tLoad].GetMappedValue();
						int		tFL = tReads[tLoad].GetFragmentLength();

						for ( int j = 0; j < 2; ++j ) {
							tSS.str( "" ); tSS << "XB:f:" << tBQ[j & 1];
							tReads[tLoad].Append( j, tSS.str().c_str() );
							tSS.str( "" ); tSS << "XC:f:" << tBQ[(j+1) & 1];
							tReads[tLoad].Append( j, tSS.str().c_str() );

							tSS.str( "" ); tSS << "MQ:i:" << tReads[tLoad].GetMQ( j );
							tReads[tLoad].Append( j, tSS.str().c_str() );

							tSS.str( "" ); tSS << "XS:i:" << tMapped;
							tReads[tLoad].Append( j, tSS.str().c_str() );

							if ( tFL >= 0 ) {
								tSS.str( "" ); tSS << "Xl:i:" << tFL;
								tReads[tLoad].Append( j, tSS.str().c_str() );
							}

							if ( tMapped <= 5 ) {
								if ( ( tBuf = tReads[tLoad].GetTagValue( (j+1) & 1, "XT" ) ).length() > 0 ) {
									tBuf[1] = 't';
									tSS.str( "" ); tSS << tBuf;
									tReads[tLoad].Append( j, tSS.str().c_str() );
								}
								tContig = tReads[tLoad].GetContigName( j );
							} else {
								tContig = tReads[tLoad].GetMappedContig();
							}
							FileInfoMap::iterator	tFind = tFMap.find( tContig );
							if ( tFind == tFMap.end() ) {
								COutFileInfo*	tInfo = new COutFileInfo();
								tSS.str( "" );
								tSS << tOutDir << tContig << ".sam";
								if ( ! tInfo->Open( tSS.str().c_str() ) ) {
									std::cerr << "create output file failed!!: " << tSS.str() << std::endl;
									return -1;
								}
								tInfo->fiCount = 0;
								tFind = tFMap.insert( FileInfoMap::value_type( tContig, tInfo ) ).first;
							}
							tReads[tLoad].PutLine( j, tFind->second->fiOut );
							++tFind->second->fiCount;
						}
						++i;
					}
				}
				tIn.Close();
				FileInfoMap::const_iterator	tIter = tFMap.begin();
				while ( tIter != tFMap.end() ) {
					tIter->second->Close();
					std::cout << tIter->first << '\t' << tIter->second->fiCount << '\n'; //std::endl;
					delete tIter->second;
					++tIter;
				}
			}
		}
	}

	return 0;
}
