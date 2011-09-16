// StrManip.h

#ifndef STR_MANIP_H
#define STR_MANIP_H

#include <cstdlib>
#include <string>
#include "../Common.h"

inline static void strToNumber(const char* str, int* pNumber) {*pNumber = std::atoi(str);}
inline static void strToNumber(const char* str, uint* pNumber) {*pNumber = std::atoi(str);}
inline static void strToNumber(const char* str, float* pNumber) {*pNumber = std::atof(str);}
inline static void strToNumber(const char* str, double* pNumber) {*pNumber = (double)(std::atof(str));}

// ---------------------------------------------------------------------
template <class T>
inline void getNumbersArray(const char* str, T* numbersArray, unsigned* pNbNumbers = NULL)
{
	if(!str)
	{
		*pNbNumbers=0;
		return;
	}

	// string correcsponding to the current number
	char strCurrentNumber[32] = "";

	for(unsigned i=0, j=0, k=0 ; str[i] != '\0' ; i++)
	{
		// if the char is the last of str :
		if(str[i+1] == '\0')
		{
			// the string strCurrentNumber is terminated
			strCurrentNumber[j] = str[i];
			j++;
			strCurrentNumber[j] = '\0';

			// the number is added to the array
			strToNumber(strCurrentNumber, &numbersArray[k]);
			k++;

			if(pNbNumbers != NULL)
				*pNbNumbers = k;

			break;
		}
		else if(str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n')
		{
			strCurrentNumber[j] = '\0';

			strToNumber(strCurrentNumber, &numbersArray[k]);
			k++;

			while(str[i+1] == ' ' || str[i+1] == '\t'|| str[i+1] == '\r' || str[i+1] == '\n')
				i++;

			j = 0;
			strCurrentNumber[0] = '\0';
		}
		else
		{
			strCurrentNumber[j] = str[i];
			j++;
		}
	}
}

// ---------------------------------------------------------------------
inline std::string safeString(const char* s)
{
	if(s == NULL)
		return std::string();
	else
		return std::string(s);
}

// ---------------------------------------------------------------------
inline std::string getBaseDirectory(const char* filename)
{
	std::string s = filename;
	return s.substr(0, s.find_last_of("/\\"));
}

#endif // STR_MANIP_H
