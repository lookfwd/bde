// bsl_functional.h                                                   -*-C++-*-
#ifndef INCLUDED_BSL_FUNCTIONAL
#define INCLUDED_BSL_FUNCTIONAL

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide functionality of the corresponding C++ Standard header.
//
//@SEE_ALSO: package bos+stdhdrs in the bos package group
//
//@DESCRIPTION: Provide types, in the 'bsl' namespace, equivalent to those
// defined in the corresponding C++ standard header.  Include the native
// compiler-provided standard header, and also directly include Bloomberg's
// implementation of the C++ standard type (if one exists).  Finally, place the
// included symbols from the 'std' namespace (if any) into the 'bsl' namespace.

#include <bsls_libraryfeatures.h>
#include <bsls_nativestd.h>
#include <bsls_platform.h>

#include <functional>

namespace bsl {

    // Import selected symbols into bsl namespace
    using native_std::binary_negate;
    using native_std::divides;
    using native_std::greater;
    using native_std::greater_equal;
    using native_std::less;
    using native_std::less_equal;
    using native_std::logical_and;
    using native_std::logical_not;
    using native_std::logical_or;
    using native_std::minus;
    using native_std::modulus;
    using native_std::multiplies;
    using native_std::negate;
    using native_std::not1;
    using native_std::not2;
    using native_std::not_equal_to;
    using native_std::plus;
    using native_std::unary_negate;

#if ! defined BSLS_LIBRARYFEATURES_HAS_CPP17_DEPRECATED_REMOVED
    // These names are removed by C++17
    using native_std::binary_function;
    using native_std::bind1st;
    using native_std::bind2nd;
    using native_std::binder1st;
    using native_std::binder2nd;
    using native_std::const_mem_fun1_ref_t;
    using native_std::const_mem_fun1_t;
    using native_std::const_mem_fun_ref_t;
    using native_std::const_mem_fun_t;
    using native_std::mem_fun1_ref_t;
    using native_std::mem_fun1_t;
    using native_std::mem_fun;
    using native_std::mem_fun_ref;
    using native_std::mem_fun_ref_t;
    using native_std::mem_fun_t;
    using native_std::pointer_to_binary_function;
    using native_std::pointer_to_unary_function;
    using native_std::ptr_fun;
    using native_std::unary_function;
#endif

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY
    namespace placeholders = native_std::placeholders;

    using native_std::bind;
    using native_std::bit_and;
    using native_std::bit_or;
    using native_std::bit_xor;
    using native_std::is_bind_expression;
    using native_std::is_placeholder;
    using native_std::mem_fn;
#endif  // BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP14_BASELINE_LIBRARY
    using native_std::bit_not;
#endif  // BSLS_LIBRARYFEATURES_HAS_CPP14_BASELINE_LIBRARY

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_BASELINE_LIBRARY
    using native_std::is_bind_expression_v;
    using native_std::is_placeholder_v;
#elif defined BSLS_LIBRARYFEATURES_HAS_CPP14_BASELINE_LIBRARY
    template <class TYPE>
    constexpr bool is_bind_expression_v =
                                   native_std::is_bind_expression<TYPE>::value;
        // This template variable represents the result value of the
        // 'native_std::is_bind_expression' meta-function.

    template <class TYPE>
    constexpr bool is_placeholder_v = native_std::is_placeholder<TYPE>::value;
        // This template variable represents the result value of the
        // 'native_std::is_placeholder' meta-function.

#endif

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP17_SEARCH_ALGORITHM
    using native_std::default_searcher;
    using native_std::boyer_moore_searcher;
    using native_std::boyer_moore_horspool_searcher;
#endif

}  // close package namespace

// Include Bloomberg's implementation, unless compilation is configured to
// override native types in the 'std' namespace with Bloomberg's
// implementation, in which case the implementation file will be included by
// the Bloomberg supplied standard header file.

#ifndef BSL_OVERRIDES_STD

#ifndef BDE_OPENSOURCE_PUBLICATION // STP
#include <bslstp_exfunctional.h>
#endif  // BDE_OPENSOURCE_PUBLICATION -- STP
#ifndef BSLS_LIBRARYFEATURES_HAS_CPP17_SEARCH_ALGORITHM
#include <bslstl_defaultsearcher.h>
#include <bslstl_boyermoorehorspoolsearcher.h>
#endif  // BSLS_LIBRARYFEATURES_HAS_CPP17_SEARCH_ALGORITHM
#include <bslstl_equalto.h>
#include <bslstl_function.h>
#include <bslstl_hash.h>
#include <bslstl_referencewrapper.h>

#endif

#endif // INCLUDED_BSL_FUNCTIONAL

// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
