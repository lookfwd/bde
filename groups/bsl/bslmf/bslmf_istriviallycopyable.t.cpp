// bslmf_istriviallycopyable.t.cpp                                    -*-C++-*-

#include <bslmf_istriviallycopyable.h>

#include <bslmf_addconst.h>
#include <bslmf_addcv.h>
#include <bslmf_addlvaluereference.h>
#include <bslmf_addpointer.h>
#include <bslmf_addvolatile.h>
#include <bslmf_nestedtraitdeclaration.h>

#include <bsls_bsltestutil.h>
#include <bsls_timeinterval.h>

#include <stdio.h>   // 'printf'
#include <stdlib.h>  // 'atoi'

using namespace BloombergLP;

//=============================================================================
//                                TEST PLAN
//-----------------------------------------------------------------------------
//                                Overview
//                                --------
// The component under test defines a meta-function,
// 'bsl::is_trivially_copyable' and a template variable
// 'bsl::is_trivially_copyable_v', that determine whether a template parameter
// type is trivially copyable.  By default, the meta-function supports a
// restricted set of type categories and can be extended to support other types
// through either template specialization or use of the
// 'BSLMF_NESTED_TRAIT_DECLARATION' macro.
//
// Thus, we need to ensure that the natively supported types are correctly
// identified by the meta-function by testing the meta-function with each of
// the supported type categories.  We also need to verify that the
// meta-function can be correctly extended to support other types through
// either of the two supported mechanisms.  Finally, we need to test correct
// support for cv-qualified and array types, where the underlying type may be
// trivially copyable.
//
// ----------------------------------------------------------------------------
// PUBLIC CLASS DATA
// [ 1] bsl::is_trivially_copyable::value
// [ 1] bsl::is_trivially_copyable_v
//
// ----------------------------------------------------------------------------
// [ 4] USAGE EXAMPLE
// [ 3] TESTING: 'bsl::is_trivially_copyable<bsls::TimeInterval>'
// [ 2] EXTENDING bsl::is_trivially_copyable

// ============================================================================
//                     STANDARD BSL ASSERT TEST FUNCTION
// ----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(bool condition, const char *message, int line)
{
    if (condition) {
        printf("Error " __FILE__ "(%d): %s    (failed)\n", line, message);

        if (0 <= testStatus && testStatus <= 100) {
            ++testStatus;
        }
    }
}

}  // close unnamed namespace

// ============================================================================
//               STANDARD BSL TEST DRIVER MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT       BSLS_BSLTESTUTIL_ASSERT
#define ASSERTV      BSLS_BSLTESTUTIL_ASSERTV

#define LOOP_ASSERT  BSLS_BSLTESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BSLS_BSLTESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BSLS_BSLTESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BSLS_BSLTESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BSLS_BSLTESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BSLS_BSLTESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BSLS_BSLTESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BSLS_BSLTESTUTIL_LOOP6_ASSERT

#define Q            BSLS_BSLTESTUTIL_Q   // Quote identifier literally.
#define P            BSLS_BSLTESTUTIL_P   // Print identifier and value.
#define P_           BSLS_BSLTESTUTIL_P_  // P(X) without '\n'.
#define T_           BSLS_BSLTESTUTIL_T_  // Print a tab (w/o newline).
#define L_           BSLS_BSLTESTUTIL_L_  // current Line number

//=============================================================================
//                  COMPONENT SPECIFIC MACROS FOR TESTING
//-----------------------------------------------------------------------------

// Each of the macros below will test the 'bsl::is_trivially_copyable' trait
// with a set of variations on a type.  There are several layers of macros, as
// object types support the full range of variation, but function types cannot
// form an array, nor be cv-qualified.  Similarly, 'void' may be cv-qualified
// but still cannot form an array.  As macros are strictly text-substitution
// we must use the appropriate 'add_decoration' traits to transform types in a
// manner that is guaranteed to be syntactically valid.  Note that these are
// not type-dependent contexts, so there is no need to use 'typename' when
// fetching the result from any of the queried traits.

#ifdef BSLS_COMPILERFEATURES_SUPPORT_VARIABLE_TEMPLATES
# define ASSERT_IS_TRIVIALLY_COPYABLE_TYPE(TYPE, RESULT)                      \
    ASSERT( bsl::is_trivially_copyable  <TYPE>::value == RESULT);             \
    ASSERT( bsl::is_trivially_copyable  <TYPE>::value ==                      \
            bsl::is_trivially_copyable_v<TYPE>);                              \
    ASSERT( bsl::is_trivially_copyable<bsl::add_pointer<TYPE>::type>::value); \
    ASSERT(!bsl::is_trivially_copyable<                                       \
                                bsl::add_lvalue_reference<TYPE>::type>::value)
    // Test the specified 'TYPE', a pointer and a reference to that type and
    // confirm that the result value of 'bsl::is_trivially_copyable'
    // instantiated with 'TYPE' and the expected 'RESULT' value are the same.
    // Also confirm that 'bsl::is_trivially_copyable_v' has the same value as
    // the result value.  Also confirm that result value of
    // 'bsl::is_trivially_copyable' instantiated with a pointer to 'TYPE' is
    // 'true', and instantiated with a reference to 'TYPE' is 'false'.

# define ASSERT_IS_TRIVIALLY_COPYABLE(TYPE, RESULT)                           \
    ASSERT( bsl::is_trivially_copyable<  TYPE>::value == RESULT);             \
    ASSERT( bsl::is_trivially_copyable  <TYPE>::value ==                      \
            bsl::is_trivially_copyable_v<TYPE>)
    // Confirm that the result value of 'bsl::is_trivially_copyable'
    // instantiated with the specified 'TYPE' and the expected 'RESULT' value
    // are the same.  Also confirm that 'bsl::is_trivially_copyable_v' has the
    // same value as the result value.
#else
# define ASSERT_IS_TRIVIALLY_COPYABLE_TYPE(TYPE, RESULT)                      \
    ASSERT( bsl::is_trivially_copyable<TYPE>::value == RESULT);               \
    ASSERT( bsl::is_trivially_copyable<bsl::add_pointer<TYPE>::type>::value); \
    ASSERT(!bsl::is_trivially_copyable<                                       \
                                bsl::add_lvalue_reference<TYPE>::type>::value)
    // Test the specified 'TYPE', a pointer and a reference to that type and
    // confirm that the result value of 'bsl::is_trivially_copyable'
    // instantiated with 'TYPE' and the expected 'RESULT' value are the same.
    // Also confirm that result value of 'bsl::is_trivially_copyable'
    // instantiated with a pointer to 'TYPE' is 'true', and instantiated with a
    // reference to 'TYPE' is 'false'.
# define ASSERT_IS_TRIVIALLY_COPYABLE(TYPE, RESULT)                           \
    ASSERT( bsl::is_trivially_copyable<  TYPE>::value == RESULT)
    // Confirm that the result value of 'bsl::is_trivially_copyable'
    // instantiated with the specified 'TYPE' and the expected 'RESULT' value
    // are the same.
#endif

#define ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE, RESULT)                    \
    ASSERT_IS_TRIVIALLY_COPYABLE_TYPE(TYPE, RESULT);                          \
    ASSERT_IS_TRIVIALLY_COPYABLE_TYPE(bsl::add_const<TYPE>::type, RESULT);    \
    ASSERT_IS_TRIVIALLY_COPYABLE_TYPE(bsl::add_volatile<TYPE>::type, RESULT); \
    ASSERT_IS_TRIVIALLY_COPYABLE_TYPE(bsl::add_cv<TYPE>::type, RESULT);

#if defined(BSLS_PLATFORM_CMP_IBM)
// Last checked with the xlC 12.1 compiler.  The IBM xlC compiler has problems
// correctly handling arrays of unknown bound as template parameters.
# define ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(TYPE, RESULT)               \
    ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE, RESULT)                        \
    ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE[128], RESULT)                   \
    ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE[12][8], RESULT)
#else
# define ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(TYPE, RESULT)               \
    ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE, RESULT)                        \
    ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE[128], RESULT)                   \
    ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE[12][8], RESULT)                 \
    ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE[], RESULT)                      \
    ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(TYPE[][8], RESULT)
#endif

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

namespace {

class MyTriviallyCopyableType {
};

struct MyNonTriviallyCopyableType {
    MyNonTriviallyCopyableType() {}
    MyNonTriviallyCopyableType(const MyNonTriviallyCopyableType&) {}
    //...
};

}  // close unnamed namespace

namespace bsl {

template <>
struct is_trivially_copyable<MyTriviallyCopyableType> : bsl::true_type {
    // This template specialization for 'is_trivially_copyable' indicates that
    // 'MyTriviallyCopyableType' is a trivially copyable.
};

}  // close namespace bsl


namespace {

struct UserDefinedTcTestType {
    // This user-defined type, which is marked to be trivially copyable using
    // template specialization (below), is used for testing.

    UserDefinedTcTestType() {}
    UserDefinedTcTestType(const UserDefinedTcTestType&) {}
        // Explicitly supply constructors that do nothing, to ensure that this
        // class has no trivial traits detected with a conforming C++11 library
        // implementation.
};

struct UserDefinedTcTestType2 {
    // This user-defined type, which is marked to be trivially copyable using
    // the 'BSLMF_NESTED_TRAIT_DECLARATION' macro, is used for testing.

    BSLMF_NESTED_TRAIT_DECLARATION(UserDefinedTcTestType2,
                                   bsl::is_trivially_copyable);

    UserDefinedTcTestType2() {}
    UserDefinedTcTestType2(const UserDefinedTcTestType2&) {}
        // Explicitly supply constructors that do nothing, to ensure that this
        // class has no trivial traits detected with a conforming C++11 library
        // implementation.
};

struct UserDefinedNonTcTestType {
    // This user-defined type, which is not marked to be trivially copyable, is
    // used for testing.
    UserDefinedNonTcTestType() {}
    UserDefinedNonTcTestType(const UserDefinedNonTcTestType&) {}
};

enum EnumTestType {
    // This 'enum' type is used for testing.
};

typedef int (UserDefinedNonTcTestType::*MethodPtrTestType) ();
    // This pointer to non-static function member type is used for testing.

}  // close unnamed namespace


namespace bsl {

template <>
struct is_trivially_copyable<UserDefinedTcTestType> : bsl::true_type {
};

}  // close namespace bsl


//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int                 test = argc > 1 ? atoi(argv[1]) : 0;
    bool             verbose = argc > 2;
    bool         veryVerbose = argc > 3;
    bool     veryVeryVerbose = argc > 4;
    bool veryVeryVeryVerbose = argc > 5;

    (void) veryVerbose;          // eliminate unused variable warning
    (void) veryVeryVerbose;      // eliminate unused variable warning
    (void) veryVeryVeryVerbose;  // eliminate unused variable warning

    setbuf(stdout, NULL);       // Use unbuffered output

    printf("TEST " __FILE__ " CASE %d\n", test);

    switch (test) { case 0:
      case 5: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //
        // Concerns:
        //: 1 The usage example provided in the component header file compiles,
        //:   links, and runs as shown.
        //
        // Plan:
        //: 1 Incorporate usage example from header into test driver, remove
        //:   leading comment characters, and replace 'assert' with 'ASSERT'.
        //:   (C-1)
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) printf("\nUSAGE EXAMPLE"
                            "\n=============\n");

///Usage
///-----
// In this section we show intended use of this component.
//
///Example 1: Verify Whether Types are Trivially Copyable
/// - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose that we want to assert whether a type is trivially copyable.
//
// First, we define a set of types to evaluate:
//..
        typedef int  MyFundamentalType;
        typedef int& MyFundamentalTypeReference;
//
//  class MyTriviallyCopyableType {
//  };
//
//  struct MyNonTriviallyCopyableType {
//      //...
//  };
//..
// Then, since user-defined types cannot be automatically evaluated by
// 'is_trivially_copyable', we define a template specialization to specify that
// 'MyTriviallyCopyableType' is trivially copyable:
//..
//  namespace bsl {
//
//  template <>
//  struct is_trivially_copyable<MyTriviallyCopyableType> : bsl::true_type {
//      // This template specialization for 'is_trivially_copyable' indicates
//      // that 'MyTriviallyCopyableType' is a trivially copyable type.
//  };
//
//  }  // close namespace bsl
//..
// Now, we verify whether each type is trivially copyable using
// 'bsl::is_trivially_copyable':
//..
        ASSERT(true  == bsl::is_trivially_copyable<MyFundamentalType>::value);
        ASSERT(false == bsl::is_trivially_copyable<
                                           MyFundamentalTypeReference>::value);
        ASSERT(true  == bsl::is_trivially_copyable<
                                              MyTriviallyCopyableType>::value);
        ASSERT(false == bsl::is_trivially_copyable<
                                           MyNonTriviallyCopyableType>::value);
//..
// Note that if the current compiler supports the variable templates C++14
// feature, then we can re-write the snippet of code above as follows:
#ifdef BSLS_COMPILERFEATURES_SUPPORT_VARIABLE_TEMPLATES
        ASSERT( bsl::is_trivially_copyable_v<MyFundamentalType>);
        ASSERT(!bsl::is_trivially_copyable_v<MyFundamentalTypeReference>);
        ASSERT( bsl::is_trivially_copyable_v<MyTriviallyCopyableType>);
        ASSERT(!bsl::is_trivially_copyable_v<MyNonTriviallyCopyableType>);
#endif
//..

      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING: 'bsl::is_trivially_copyable<bslmf::Nil>'
        //   Ensure that 'bsl::is_trivially_copyable' meta-function is
        //   specialized correctly for 'bsls::TimeInterval'.
        //
        // NOTE: This is not tested in 'bsls' for dependency reasons.
        //
        // Concerns:
        //: 1 'bsl::is_trivially_copyable<bslmf::Nil>' is 'true'
        //
        // Plan:
        //: 1 Verify 'bsl::is_trivially_copyable<bslmf::Nil>' is 'true'
        //
        // Testing:
        //
        // --------------------------------------------------------------------

        if (verbose)
            printf(
              "\nTESTING: 'bsl::is_trivially_copyable<bsls::TimeInterval>'\n"
              "\n=========================================================\n");

        // C-1
        ASSERT(bsl::is_trivially_copyable<bslmf::Nil>::value);


      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING: 'bsl::is_trivially_copyable<bsls::TimeInterval>'
        //   Ensure that 'bsl::is_trivially_copyable' meta-function is
        //   specialized correctly for 'bsls::TimeInterval'.
        //
        // NOTE: This is not tested in 'bsls' for dependency reasons.
        //
        // Concerns:
        //: 1 'bsl::is_trivially_copyable<bsls::TimeInterval>' is 'true'
        //
        // Plan:
        //: 1 Verify 'bsl::is_trivially_copyable<bsls::TimeInterval>' is 'true'
        //
        // Testing:
        //
        // --------------------------------------------------------------------

        if (verbose)
            printf(
              "\nTESTING: 'bsl::is_trivially_copyable<bsls::TimeInterval>'\n"
              "\n=========================================================\n");

        // C-1
        ASSERT(bsl::is_trivially_copyable<bsls::TimeInterval>::value);


      } break;
      case 2: {
        // --------------------------------------------------------------------
        // EXTENDING 'bsl::is_trivially_copyable'
        //   Ensure the 'bsl::is_trivially_copyable' meta-function returns the
        //   correct value for types explicitly specified to be trivially
        //   copyable.
        //
        // Concerns:
        //: 1 The meta-function returns 'false' for normal user-defined types.
        //:
        //: 2 The meta-function returns 'true' for a user-defined type, if a
        //:   specialization for 'bsl::is_trivially_copyable' on that type is
        //:   defined to inherit from 'bsl::true_type'.
        //:
        //: 3 The meta-function returns 'true' for a user-defined type that
        //:   specifies it has the trait using the
        //:   'BSLMF_NESTED_TRAIT_DECLARATION' macro.
        //:
        //: 4 For cv-qualified types, the meta-function returns 'true' if the
        //:   corresponding cv-unqualified type is trivially copyable, and
        //:   'false' otherwise.
        //:
        //: 5 For array types, the meta-function returns 'true' if the array
        //:   element is trivially copyable, and 'false' otherwise.
        //
        // Plan:
        //:  1 Create a set of macros that will generate an 'ASSERT' test for
        //:    all variants of a type:  (C4,5)
        //:    o  reference and pointer types
        //:    o  all cv-qualified combinations
        //:    o  arrays, of fixed and runtime bounds, and multiple dimensions
        //:
        //:  2 For each category of type in concerns 1-3, use the appropriate
        //:    test macro for confirm the correct result for a representative
        //:    sample of types.
        //
        // Testing:
        //   EXTENDING 'bsl::is_trivially_copyable'
        // --------------------------------------------------------------------

        if (verbose)
            printf("\nEXTENDING 'bsl::is_trivially_copyable'"
                   "\n======================================\n");

        // C-1
        ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(UserDefinedNonTcTestType,
                                                 false);

        // C-2
        ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(UserDefinedTcTestType, true);

        // C-3
        ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(UserDefinedTcTestType2, true);
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // 'bsl::is_trivially_copyable::value'
        //   Ensure the 'bsl::is_trivially_copyable' meta-function
        //   returns the correct value for intrinsically supported types.
        //
        // Concerns:
        //:  1 The meta-function returns 'true' for fundamental types.
        //:
        //:  2 The meta-function returns 'true' for enumerated types.
        //:
        //:  3 The meta-function returns 'true' for pointer to member types.
        //:
        //:  4 The meta-function returns 'false' for cv-qualified 'void' types.
        //:
        //:  5 The meta-function returns 'false' for function types.
        //
        //:  6 The meta-function returns 'true' for pointer types.
        //:
        //:  7 The meta-function returns 'false' for reference types.
        //:
        //:  8 The meta-function returns the same result for array types as it
        //:    would for the array's element type.
        //:
        //:  9 The meta-function returns the same result for cv-qualified
        //:    types that it would return 'true' for the corresponding
        //:    cv-unqualified type.
        //:
        //: 10 That 'is_trivially_copyable<T>::value' has the same value as
        //:    'is_trivially_copyable_v<T>' for a variety of template parameter
        //:    types.
        //
        // Plan:
        //:  1 Create a set of macros that will generate an 'ASSERT' test for
        //:    all variants of a type:  (C6-9)
        //:    o  reference and pointer types
        //:    o  all cv-qualified combinations
        //:    o  arrays, of fixed and runtime bounds, and multiple dimensions
        //:
        //:  2 For each category of type in concerns 1-5, use the appropriate
        //:    test macro for confirm the correct result for a representative
        //:    sample of types.
        //
        // Testing:
        //   bsl::is_trivially_copyable::value
        // --------------------------------------------------------------------

        if (verbose)
            printf("\n'bsl::is_trivially_copyable::value'"
                   "\n===================================\n");

        // C-1, (partial 6, 7, 8, 9)
        ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(int, true);
        ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(char, true);
        ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(long double, true);

        // C-2 (partial 6, 7, 8, 9)
        ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(EnumTestType, true);

        // C-3 (complete 6, 7, 8, 9)
        ASSERT_IS_TRIVIALLY_COPYABLE_OBJECT_TYPE(MethodPtrTestType, true);

        // C-4 : 'void' is not an object type, but can be cv-qualified.
        ASSERT_IS_TRIVIALLY_COPYABLE_CV_TYPE(void, false);

        // C-5 : Function types are not object types, nor cv-qualifiable.
        ASSERT_IS_TRIVIALLY_COPYABLE(void(*)(), true);
        ASSERT_IS_TRIVIALLY_COPYABLE(int(*)(float, double...), true);
        ASSERT_IS_TRIVIALLY_COPYABLE(void(), false);
        ASSERT_IS_TRIVIALLY_COPYABLE(int(float, double...), false);
        ASSERT_IS_TRIVIALLY_COPYABLE(void(&)(), false);
        ASSERT_IS_TRIVIALLY_COPYABLE(int(&)(float, double...), false);
      } break;
      default: {
          fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
          testStatus = -1;
      }
    }

    if (testStatus > 0) {
        fprintf(stderr, "Error, non-zero test status = %d.\n", testStatus);
    }
    return testStatus;
}

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
