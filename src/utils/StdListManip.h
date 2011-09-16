// StdListManip.h

#ifndef STD_LIST_MANIP_H
#define STD_LIST_MANIP_H

#include <list>
#include <vector>

// ---------------------------------------------------------------------
// Convert a list to a native C++ array
template <class T>
T* listToArray(const std::list<T>& l)
{
	T* result = new T[l.size()];

	uint i=0;
	typename std::list<T>::const_iterator it_end = l.end();
	for(typename std::list<T>::const_iterator it = l.begin() ;
		it != it_end ;
		it++, i++)
	{
		result[i] = *it;
	}

	return result;
}

// ---------------------------------------------------------------------

// listCat(&a, &b) operates a = a + b
template <class T>
void listCat(std::list<T>* a, const std::list<T>* b)
{
	typename std::list<T>::const_iterator it_b_end = b->end();
	for(typename std::list<T>::const_iterator it_b = b->begin() ;
		it_b != b->end() ;
		it_b++)
	{
		a->push_back(*it_b);
	}
}

// vectCat(&a, &b) operates a = a + b
template <class T>
void vectCat(std::vector<T>* a, const std::vector<T>* b)
{
	typename std::vector<T>::const_iterator it_b_end = b->end();
	for(typename std::vector<T>::const_iterator it_b = b->begin() ;
		it_b != b->end() ;
		it_b++)
	{
		a->push_back(*it_b);
	}
}

#endif // STD_LIST_MANIP_H
