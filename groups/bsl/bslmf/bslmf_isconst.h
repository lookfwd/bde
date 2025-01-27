// bslmf_isconst.h                                                    -*-C++-*-
#ifndef INCLUDED_BSLMF_ISCONST
#define INCLUDED_BSLMF_ISCONST

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a compile-time check for 'const'-qualified types.
//
//@CLASSES:
//  bsl::is_const: meta-function for determining 'const'-qualified types
//  bsl::is_const_v: the result value of the 'bsl::is_const' meta-function
//
//@SEE_ALSO: bslmf_integralconstant
//
//@DESCRIPTION: This component defines a meta-function, 'bsl::is_const' and a
// template variable 'bsl::is_const_v', that represents the result value of the
// 'bsl::is_const' meta-function, that may be used to query whether a type is
// 'const'-qualified as defined in the C++ standard [basic.type.qualifier].
//
// 'bsl::is_const' meets the requirements of the 'is_const' template defined in
// the C++11 standard [meta.unary.prop].
//
// Note that the template variable 'is_const_v' is defined in the C++17
// standard as an inline variable.  If the current compiler supports the inline
// variable C++17 compiler feature, 'bsl::is_const_v' is defined as an
// 'inline constexpr bool' variable.  Otherwise, if the compiler supports the
// variable templates C++14 compiler feature, 'bsl::is_const_v' is defined
// as a non-inline 'constexpr bool' variable.  See
// 'BSLS_COMPILERFEATURES_SUPPORT_INLINE_VARIABLES' and
// 'BSLS_COMPILERFEATURES_SUPPORT_VARIABLE_TEMPLATES' macros in
// bsls_compilerfeatures component for details.
//
///Usage
///-----
// In this section we show intended use of this component.
//
///Example 1: Verify 'const' Types
///- - - - - - - - - - - - - - - -
// Suppose that we want to assert whether a particular type is
// 'const'-qualified.
//
// First, we create two 'typedef's -- a 'const'-qualified type and an
// unqualified type:
//..
//  typedef int        MyType;
//  typedef const int  MyConstType;
//..
// Now, we instantiate the 'bsl::is_const' template for each of the 'typedef's
// and assert the 'value' static data member of each instantiation:
//..
//  assert(false == bsl::is_const<MyType>::value);
//  assert(true  == bsl::is_const<MyConstType>::value);
//..
// Note that if the current compiler supports the variable templates C++14
// feature then we can re-write the snippet of code above using the
// 'bsl::is_const_v' variable as follows:
//..
//#ifdef BSLS_COMPILERFEATURES_SUPPORT_VARIABLE_TEMPLATES
//  assert(false == bsl::is_const_v<MyType>);
//  assert(true  == bsl::is_const_v<MyConstType>);
//#endif
//..

#include <bslscm_version.h>

#include <bslmf_integralconstant.h>
#include <bslmf_issame.h>

#include <bsls_compilerfeatures.h>
#include <bsls_keyword.h>
#include <bsls_platform.h>

#include <stddef.h>

#if (defined(BSLS_PLATFORM_CMP_MSVC) && BSLS_PLATFORM_CMP_VERSION < 1910)     \
 ||  defined(BSLS_PLATFORM_CMP_IBM)
// The Microsoft compiler does not recognize array-types as cv-qualified (when
// the element type is cv-qualified) when performing matching for partial
// template specialization, but does get the correct result when performing
// overload resolution for functions (taking arrays by reference).  Given the
// function dispatch behavior being correct, we choose to work around this
// compiler bug, rather than try to report compiler behavior, as the compiler
// itself is inconsistent depending on how the trait might be used.  This also
// corresponds to how Microsft itself implements the trait in VC2010 and later.
// Last tested against VC 2015 (Release Candidate).
# define BSLMF_ISCONST_COMPILER_DOES_NOT_DETECT_CV_QUALIFIED_ARRAY_ELEMENT 1
#endif

namespace bsl {

                         // ===============
                         // struct is_const
                         // ===============

template <class TYPE>
struct is_const : false_type {
    // This 'struct' template implements the 'is_const' meta-function defined
    // in the C++11 standard [meta.unary.cat] to determine if the (template
    // parameter) 'TYPE' is 'const'-qualified.  This 'struct' derives from
    // 'bsl::true_type' if the 'TYPE' is 'const'-qualified, and
    // 'bsl::false_type' otherwise.  Note that this generic default template
    // derives from 'bsl::false_type'.  A template specialization is provided
    // (below) that derives from 'bsl::true_type'.
};

                         // ===========================
                         // struct is_const<TYPE const>
                         // ===========================

#if defined(BSLS_PLATFORM_CMP_SUN)
template <class TYPE>
struct is_const<const TYPE>
    : integral_constant<bool, !is_same<TYPE, const TYPE>::value> {
     // This partial specialization of 'is_const', for when the (template
     // parameter) 'TYPE' is 'const'-qualified, derives from 'bsl::true_type'.
     // Note that the Solaris CC compiler misdiagnoses cv-qualified
     // "abominable" function types as being cv-qualified themselves.  The
     // correct result is obtained by delegating the result to a call through
     // 'is_same'.
};
#else
template <class TYPE>
struct is_const<const TYPE> : true_type {
     // This partial specialization of 'is_const', for when the (template
     // parameter) 'TYPE' is 'const'-qualified, derives from 'bsl::true_type'.
};
#endif


#if defined(BSLMF_ISCONST_COMPILER_DOES_NOT_DETECT_CV_QUALIFIED_ARRAY_ELEMENT)
// The Microsoft compiler does not recognize array-types as cv-qualified when
// the element type is cv-qualified when performing matching for partial
// template specialization, but does get the correct result when performing
// overload resolution for functions (taking arrays by reference).  Given the
// function dispatch behavior being correct, we choose to work around this
// compiler bug, rather than try to report compiler behavior, as the compiler
// itself is inconsistent depeoning on how the trait might be used.  This also
// corresponds to how Microsft itself implements the trait in VC2010 and later.
// Last tested against VC 2015 (Release Candidate).

template <class TYPE>
struct is_const<const TYPE[]> : true_type {
     // This partial specialization of 'is_const', for when the (template
     // parameter) 'TYPE' is 'const'-qualified, derives from 'bsl::true_type'.
     // Note that this single specialization is sufficient to work around the
     // MSVC issue, even for multidimensional arrays.
};

template <class TYPE, size_t LENGTH>
struct is_const<const TYPE[LENGTH]> : true_type {
     // This partial specialization of 'is_const', for when the (template
     // parameter) 'TYPE' is 'const'-qualified, derives from 'bsl::true_type'.
     // Note that this single specialization is sufficient to work around the
     // MSVC issue, even for multidimensional arrays.
};
#endif

#ifdef BSLS_COMPILERFEATURES_SUPPORT_VARIABLE_TEMPLATES
template <class TYPE>
BSLS_KEYWORD_INLINE_VARIABLE
constexpr bool is_const_v = is_const<TYPE>::value;
    // This template variable represents the result value of the
    // 'bsl::is_const' meta-function.
#endif

}  // close namespace bsl

#endif

// ----------------------------------------------------------------------------
// Copyright 2013 Bloomberg Finance L.P.
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
