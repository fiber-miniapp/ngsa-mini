/*
	File:	my_types.h
	Copyright(C) 2009-2012 RIKEN, Japan.
*/
#ifndef __my_types_H__
#define __my_types_H__

#include <vector>
#include <string>
#include <stdint.h>
#include "usafe.h"

typedef int8_t			__Xint8;
typedef uint8_t			__Xuint8;
typedef int16_t			__Xint16;
typedef uint16_t		__Xuint16;
typedef int32_t			__Xint32;
typedef uint32_t		__Xuint32;
typedef int64_t			__Xint64;
typedef uint64_t		__Xuint64;

typedef std::vector<std::string>	__string_vector;
typedef std::vector<int>			__int_vector;
typedef std::vector<__Xint8>		__int8_vector;
typedef std::vector<__Xuint8>		__uint8_vector;
typedef std::vector<__Xint32>		__int32_vector;
typedef std::vector<__Xuint32>		__uint32_vector;
typedef std::vector<__Xint64>		__int64_vector;
typedef std::vector<__Xuint64>		__uint64_vector;
typedef std::vector<double>			__double_vector;

typedef std::vector<__int_vector>	__int_vector2;

typedef Tsafearray<std::string>		__string_array;
typedef Tsafearray<int>				__int_array;
typedef Tsafearray<__Xint8>			__int8_array;
typedef Tsafearray<__Xuint8>		__uint8_array;
typedef Tsafearray<__Xint32>		__int32_array;
typedef Tsafearray<__Xuint32>		__uint32_array;
typedef Tsafearray<__Xint64>		__int64_array;
typedef Tsafearray<__Xuint64>		__uint64_array;
typedef Tsafearray<double>			__double_array;

#define __UINT32_MAX	0xFFFFFFFFUL

#endif	//__my_types_H__
