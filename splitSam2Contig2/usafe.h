/*
	File:	usafe.h
	Copyright(C) 2008 RIKEN, Japan.
*/
#ifndef __usafe_H__
#define __usafe_H__

#include <cstdlib>
#include <iostream>

template <class T>
class Tsafearray {
public:
	T*	ptr;

public:
			Tsafearray() {
				ptr = NULL;
			};
			Tsafearray(
					long	inSize ) {
				try {
					ptr = new T [inSize];
					if ( ptr == NULL ) {
						std::cerr << "Tsafearray: allocation failed11:" << sizeof(T) << ',' << inSize <<  std::endl;
					}
				} catch (...) {
					ptr = NULL;
					std::cerr << "Tsafearray: allocation failed1:" << sizeof(T) << ',' << inSize <<  std::endl;
				}
			};
			~Tsafearray() {
				if ( ptr != NULL ) {
					delete [] ptr;
				}
			};

	T*		alloc(	long	inSize ) {
				try {
					if ( ptr != NULL ) {
						delete [] ptr;
						ptr = NULL;
					}
					if ( inSize > 0 ) {
						ptr = new T [inSize];
						if ( ptr == NULL ) {
							std::cerr << "Tsafearray: allocation failed21:" << sizeof(T) << ',' << inSize <<  std::endl;
						}
					}
				} catch (...) {
					ptr = NULL;
					std::cerr << "Tsafearray: allocation failed2:" << sizeof(T) << ',' << inSize <<  std::endl;
				}
				return ptr;
			};
	void	release() {
				try {
					if ( ptr != NULL ) {
						delete [] ptr;
					}
				} catch (...) {
					std::cerr << "Tsafearray: allocation failed3:" << sizeof(T) <<  std::endl;
				}
				ptr = NULL;
			};
};

#endif	//__usafe_H__
