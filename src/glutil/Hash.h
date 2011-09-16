// Hash.h
/* Usage:
void test(Hash h) {
	printf("h.val == %d\n", h.val);
}

test("Hello World !");
*/

#ifndef HASH_H
#define HASH_H

#include <string>
#include "../log/Log.h"

#define ADD_HASH(i) (val = val * 65599 + s[i])

struct Hash
{
	int val;

	enum Marker
	{
		AT_RUNTIME
	};

	// Non optimized constructor:
	Hash(const char* s, Marker m)
	: val(0)
	{
		for(unsigned int i=0 ; s[i] != '\0' ; i++)
			ADD_HASH(i);
	}

	// Non optimized constructor, using std::string
	Hash(const std::string& s, Marker m)
	: val(0)
	{
		size_t len = s.length();
		for(unsigned int i=0 ; i < len ; i++)
			ADD_HASH(i);
	}

	// Optimized constructors (resolved at compilation, when optimizations
	// are turned on, at least by GCC):
//	Hash(const char (&s)[1])
//	: val(0)
//	{
//		// Only 1 byte in the array => must be '\0'
//	}

	Hash(const char (&s)[2])
	: val(0)
	{
		ADD_HASH(0 );
	}

	Hash(const char (&s)[3])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 );
	}

	Hash(const char (&s)[4])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 );
	}

	Hash(const char (&s)[5])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 );
	}

	Hash(const char (&s)[6])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
	}

	Hash(const char (&s)[7])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 );
	}

	Hash(const char (&s)[8])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 );
	}

	Hash(const char (&s)[9])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 );
	}

	Hash(const char (&s)[10])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 );
	}

	Hash(const char (&s)[11])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
	}

	Hash(const char (&s)[12])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10);
	}

	Hash(const char (&s)[13])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11);
	}

	Hash(const char (&s)[14])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11); ADD_HASH(12);
	}

	Hash(const char (&s)[15])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11); ADD_HASH(12); ADD_HASH(13);
	}

	Hash(const char (&s)[16])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11); ADD_HASH(12); ADD_HASH(13); ADD_HASH(14);
	}

	Hash(const char (&s)[17])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11); ADD_HASH(12); ADD_HASH(13); ADD_HASH(14);
		ADD_HASH(15);
	}

	Hash(const char (&s)[18])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11); ADD_HASH(12); ADD_HASH(13); ADD_HASH(14);
		ADD_HASH(15); ADD_HASH(16);
	}

	Hash(const char (&s)[19])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11); ADD_HASH(12); ADD_HASH(13); ADD_HASH(14);
		ADD_HASH(15); ADD_HASH(16); ADD_HASH(17);
	}

	Hash(const char (&s)[20])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11); ADD_HASH(12); ADD_HASH(13); ADD_HASH(14);
		ADD_HASH(15); ADD_HASH(16); ADD_HASH(17); ADD_HASH(18);
	}

	// Up to 20 characters (the last byte is always '\0' and never considered)
	Hash(const char (&s)[21])
	: val(0)
	{
		ADD_HASH(0 ); ADD_HASH(1 ); ADD_HASH(2 ); ADD_HASH(3 ); ADD_HASH(4 );
		ADD_HASH(5 ); ADD_HASH(6 ); ADD_HASH(7 ); ADD_HASH(8 ); ADD_HASH(9 );
		ADD_HASH(10); ADD_HASH(11); ADD_HASH(12); ADD_HASH(13); ADD_HASH(14);
		ADD_HASH(15); ADD_HASH(16); ADD_HASH(17); ADD_HASH(18); ADD_HASH(19);
	}
};

#endif // HASH_H
