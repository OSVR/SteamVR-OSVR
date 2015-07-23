/** @file
    @brief Backport of std::make_unique to C++11.

    @date 2015

    @author
    LLVM Team
    University of Illinois at Urbana-Champaign
    <http://llvm.org>

    Extracted from libc++ which is dual-licensed under the MIT and the
    University of Illinois "BSD-Like" licenses.
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
#include <memory>       // for std::unique_ptr
#include <type_traits>  // for std::remove_extent
#include <utility>      // for std::forward

namespace std {

template<class _Tp>
struct __unique_if
{
    typedef unique_ptr<_Tp> __unique_single;
};

template<class _Tp>
struct __unique_if<_Tp[]>
{
    typedef unique_ptr<_Tp[]> __unique_array_unknown_bound;
};

template<class _Tp, size_t _Np>
struct __unique_if<_Tp[_Np]>
{
    typedef void __unique_array_known_bound;
};

template<class _Tp, class... _Args>
inline typename __unique_if<_Tp>::__unique_single
make_unique(_Args&&... __args)
{
    return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}

template<class _Tp>
inline typename __unique_if<_Tp>::__unique_array_unknown_bound
make_unique(size_t __n)
{
    typedef typename remove_extent<_Tp>::type _Up;
    return unique_ptr<_Tp>(new _Up[__n]());
}

template<class _Tp, class... _Args>
typename __unique_if<_Tp>::__unique_array_known_bound
make_unique(_Args&&...) = delete;

} // end namespace std

#endif // OSVR_HAS_STD_MAKE_UNIQUE

#endif // INCLUDED_make_unique_h_GUID_C7526AAA_3549_41DF_AF95_5323788FBADE

