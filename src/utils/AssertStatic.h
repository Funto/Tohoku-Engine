// AssertStatic.h
// Compile-time assertion macro.

#ifndef ASSERT_STATIC_H
#define ASSERT_STATIC_H

namespace StaticAssert
{
	template<bool> struct FAILED;
	template<> struct FAILED<true> {};
}

#define ASSERT_STATIC(expression) (StaticAssert::FAILED< (expression) != 0 >())

#endif // ASSERT_STATIC_H
