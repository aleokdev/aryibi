#ifndef ARYIBI_ARYIBI_ASSERT_HPP
#define ARYIBI_ARYIBI_ASSERT_HPP

#include <cassert>
#define ARYIBI_ASSERT(x, msg) (!!(x)) ? (void)(0) : __assert_fail("[aryibi] " msg, __FILE__, __LINE__, __ASSERT_FUNCTION);

#endif // ARYIBI_ARYIBI_ASSERT_HPP
