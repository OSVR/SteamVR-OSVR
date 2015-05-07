/** @file
    @brief Backport of std::make_unique to C++11.

    @date 2015

    @author
    Stephan T. Lavavej <stl@microsoft.com>

*/

#ifndef INCLUDED_make_unique_h_GUID_C7526AAA_3549_41DF_AF95_5323788FBADE
#define INCLUDED_make_unique_h_GUID_C7526AAA_3549_41DF_AF95_5323788FBADE

// Internal Includes
#include "osvr_compiler_tests.h"

#ifdef OSVR_HAS_STD_MAKE_UNIQUE

#include <memory>

#else // OSVR_HAS_STD_MAKE_UNIQUE

// If std::make_unique is not available, then we'll define it ourselves.

// Standard includes
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace std {

template<class T> struct _Unique_if
{
    typedef unique_ptr<T> _Single_object;
};

template<class T> struct _Unique_if<T[]>
{
    typedef unique_ptr<T[]> _Unknown_bound;
};

template<class T, size_t N> struct _Unique_if<T[N]>
{
    typedef void _Known_bound;
};

template<class T, class... Args>
typename _Unique_if<T>::_Single_object
make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T>
typename _Unique_if<T>::_Unknown_bound
make_unique(size_t n)
{
    typedef typename remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]());
}

template<class T, class... Args>
typename _Unique_if<T>::_Known_bound
make_unique(Args&&...) = delete;

} // end namespace std

#endif // OSVR_HAS_STD_MAKE_UNIQUE

#endif // INCLUDED_make_unique_h_GUID_C7526AAA_3549_41DF_AF95_5323788FBADE

