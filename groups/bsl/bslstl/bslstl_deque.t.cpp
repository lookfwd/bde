// bslstl_deque.t.cpp                                                 -*-C++-*-
#include <bslstl_deque.h>

#include <bslstl_forwarditerator.h>
#include <bslstl_iterator.h>
#include <bslstl_randomaccessiterator.h>
#include <bslstl_string.h>
#include <bslstl_vector.h>

#include <bslma_allocator.h>
#include <bslma_constructionutil.h>
#include <bslma_default.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_destructorguard.h>
#include <bslma_newdeleteallocator.h>
#include <bslma_rawdeleterproctor.h>
#include <bslma_testallocator.h>
#include <bslma_testallocatorexception.h>
#include <bslma_testallocatormonitor.h>

#include <bslmf_ispointer.h>
#include <bslmf_issame.h>
#include <bslmf_nestedtraitdeclaration.h>

#include <bsls_alignmentutil.h>
#include <bsls_bsltestutil.h>
#include <bsls_compilerfeatures.h>
#include <bsls_libraryfeatures.h>
#include <bsls_nameof.h>
#include <bsls_objectbuffer.h>
#include <bsls_platform.h>
#include <bsls_stopwatch.h>
#include <bsls_types.h>
#include <bsls_util.h>

#include <bsltf_allocargumenttype.h>
#include <bsltf_allocemplacabletesttype.h>
#include <bsltf_argumenttype.h>
#include <bsltf_emplacabletesttype.h>
#include <bsltf_movablealloctesttype.h>
#include <bsltf_movabletesttype.h>
#include <bsltf_moveonlyalloctesttype.h>
#include <bsltf_movestate.h>
#include <bsltf_nondefaultconstructibletesttype.h>
#include <bsltf_nontypicaloverloadstesttype.h>
#include <bsltf_stdallocatoradaptor.h>
#include <bsltf_stdstatefulallocator.h>
#include <bsltf_stdtestallocator.h>
#include <bsltf_templatetestfacility.h>
#include <bsltf_testvaluesarray.h>

#include <iterator>
#include <new>         // placement 'new'
#include <stdexcept>

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
//                             TEST PLAN
// ----------------------------------------------------------------------------
// NOTICE: To reduce the compilation time, this test driver has been broken
// into 2 parts, 'bslstl_deque.t.cpp' (cases 1-11, plus the usage example), and
// 'bslstl_deque_test.cpp' (cases 12 and higher).
//
//                              Overview
//                              --------
// The class under test is a container whose interface and contract is dictated
// by the C++ standard.  In particular, the standard mandates strong exception
// safety (with full guarantee of rollback) in many cases, along with throwing
// 'std::length_error' on any request to increase the size of a deque to have
// more than 'max_size()' elements.  (Note: the value returned by 'max_size'
// depends on the (template parameter) 'VALUE_TYPE'.)  The general concerns are
// compliance, exception safety, and proper dispatching (for member function
// templates such as 'assign' and 'insert').  In addition, deque is a
// value-semantic type whose salient attributes are its size and the value of
// each element in its (ordered) sequence.  The deque container is implemented
// in the form of a class template, so its proper instantiation for several
// types is a concern.  Regarding the allocator template argument, we use
// mostly a 'bsl::allocator' together with a 'bslma::TestAllocator' mechanism,
// but we also verify the C++ standard allocator.
//
// Important states that must be covered by testing include filling "pages" (or
// "blocks") of the deque up to their page boundaries, both at the 'front'
// and the 'back', and behavior crossing those boundaries.  Therefore, the
// generator language provides a way to insert, independently, at the front and
// at the back, to fill up pages at either the front or the back.
//
// This test driver follows the standard approach for components implementing
// value-semantic containers.  We have chosen as *primary* *manipulators* the
// 'push_back' and 'push_front' methods taking rvalue references as well as the
// 'clear' method, with all three methods to be used by the generator function
// 'gg'.  Note that this choice is conditional on the implementation of
// 'clear', which clears all blocks and then sets 'd_start' to 'd_finish'.
// Additional helper functions are provided to facilitate perturbation of
// internal state (e.g., capacity and the start iterator within the start
// block).  Note that some manipulators must support aliasing, and those that
// perform memory allocation must be tested for exception neutrality via
// 'bslma::TestAllocator'.  After the mandatory sequence of cases (1-10) for
// value-semantic types (cases 5 and 10 are not implemented, as there is no
// output or BDEX streaming below 'bslstl'), we test each individual
// constructor, manipulator, and accessor in subsequent cases.  Move semantics
// (C++11) are tested within relevant test cases.
//
// Abbreviations
// -------------
// Throughout this test driver, we use:
//  T        VALUE_TYPE (template argument, no default)
//  A        ALLOCATOR (template argument, default is 'bsl::allocator<T>')
//  deque    bsl::deque<VALUE_TYPE, ALLOCATOR>
//  Args...  shorthand for a family of templates <A1>, <A1, A2>, etc.
// ----------------------------------------------------------------------------
// class bsl::deque<T, A>
// ============================================================================
// [11] TRAITS
//
// CREATORS
// [ 2] deque(const A& a = A());
// [12] deque(size_type n, const A& a = A());
// [12] deque(size_type n, const T& value, const A& a = A());
// [12] deque(ITER first, ITER last, const A& a = A());
// [ 7] deque(const deque& original, const A& = A());
// [24] deque(deque&& original);
// [24] deque(deque&& original, const A& basicAllocator);
// [30] deque(initializer_list<T>, const A& = A());
// [ 2] ~deque();
//
/// MANIPULATORS
// [ 9] deque& operator=(const deque& rhs);
// [25] deque& operator=(deque&& rhs);
// [30] deque& operator=(initializer_list<T>);
// [13] void assign(size_type n, const T& value);
// [13] void assign(ITER first, ITER last);
// [30] void assign(initializer_list<T>);
// [14] void reserve(size_type n);
// [14] void resize(size_type n);
// [14] void resize(size_type n, const T& value);
// [31] void shrink_to_fit();
// [17] void push_front(const T& value);
// [26] void push_front(T&& rvalue);
// [17] void push_back(const T& value);
// [26] void push_back(T&& rvalue);
// [28] void emplace_front(Args&&... args);
// [28] void emplace_back(Args&&... args);
// [29] iterator emplace(const_iterator pos, Args&&... args);
// [20] void pop_front();
// [20] void pop_back();
// [18] iterator insert(const_iterator pos, const T& value);
// [27] iterator insert(const_iterator pos, T&& rvalue);
// [18] iterator insert(const_iterator pos, size_type n, const T& value);
// [19] iterator insert(const_iterator pos, ITER first, ITER last);
// [30] iterator insert(const_iterator pos, initializer_list<T>);
// [20] iterator erase(const_iterator pos);
// [20] iterator erase(const_iterator first, const_iterator last);
// [21] void swap(deque& other);
// [ 2] void clear();
// [16] iterator begin();
// [16] iterator end();
// [16] reverse_iterator rbegin();
// [16] reverse_iterator rend();
// [15] reference operator[](size_type position);
// [15] reference at(size_type position);
// [15] reference front();
// [15] reference back();
//
// ACCESSORS
// [ 4] allocator_type get_allocator() const;
// [14] size_type max_size() const;
// [ 4] size_type size() const;
// [14] size_type capacity() const;
// [14] bool empty() const;
// [16] const_iterator begin() const;
// [16] const_iterator cbegin() const;
// [16] const_iterator end() const;
// [16] const_iterator cend() const;
// [16] const_reverse_iterator rbegin() const;
// [16] const_reverse_iterator crbegin() const;
// [16] const_reverse_iterator rend() const;
// [16] const_reverse_iterator crend() const;
// [ 4] const_reference operator[](size_type position) const;
// [ 4] const_reference at(size_type position) const;
// [15] const_reference front() const;
// [15] const_reference back() const;
//
// FREE OPERATORS
// [ 6] bool operator==(const deque& lhs, const deque& rhs);
// [ 6] bool operator!=(const deque& lhs, const deque& rhs);
// [22] bool operator< (const deque& lhs, const deque& rhs);
// [22] bool operator> (const deque& lhs, const deque& rhs);
// [22] bool operator<=(const deque& lhs, const deque& rhs);
// [22] bool operator>=(const deque& lhs, const deque& rhs);
//
// FREE FUNCTIONS
// [??] void swap(deque& a, deque& b);
// ----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [11] ALLOCATOR-RELATED CONCERNS
// [33] USAGE EXAMPLE 1
// [34] USAGE EXAMPLE 2
// [23] CONCERN: 'std::length_error' is used properly.
// [ *] CONCERN: In no case does memory come from the global allocator.
// [32] CONCERN: Methods qualified 'noexcept' in standard are so implemented.
//
// TEST APPARATUS: GENERATOR FUNCTIONS
// [ 3] int ggg(Obj *object, const char *spec, bool vF = true);
// [ 3] Obj& gg(Obj *object, const char *spec);
// [ 8] Obj   g(const char *spec);  // no longer implemented

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

#define RUN_EACH_TYPE BSLTF_TEMPLATETESTFACILITY_RUN_EACH_TYPE

// ============================================================================
//                  NEGATIVE-TEST MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

// ============================================================================
//                  PRINTF FORMAT MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ZU BSLS_BSLTESTUTIL_FORMAT_ZU

// ============================================================================
//                             SWAP TEST HELPERS
// ----------------------------------------------------------------------------

namespace incorrect {

template <class TYPE>
void swap(TYPE&, TYPE&)
    // Fail.  In a successful test, this 'swap' should never be called.  It is
    // set up to be called (and fail) in the case where ADL fails to choose the
    // right 'swap' in 'invokeAdlSwap' below.
{
    ASSERT(0 && "incorrect swap called");
}

}  // close namespace incorrect

template <class TYPE>
void invokeAdlSwap(TYPE *a, TYPE *b)
    // Exchange the values of the specified '*a' and '*b' objects using the
    // 'swap' method found by ADL (Argument Dependent Lookup).
{
    using incorrect::swap;

    // A correct ADL will key off the types of '*a' and '*b', which will be of
    // our 'bsl' container type, to find the right 'bsl::swap' and not
    // 'incorrect::swap'.

    swap(*a, *b);
}

template <class TYPE>
void invokePatternSwap(TYPE *a, TYPE *b)
    // Exchange the values of the specified '*a' and '*b' objects using the
    // 'swap' method found by the recommended pattern for calling 'swap'.
{
    // Invoke 'swap' using the recommended pattern for 'bsl' clients.

    using bsl::swap;

    swap(*a, *b);
}

// The following 'using' directives must come *after* the definition of
// 'invokeAdlSwap' and 'invokePatternSwap' (above).

using namespace BloombergLP;
using bsl::deque;
using bsl::Deque_BlockLengthCalcUtil;
using bsls::NameOf;

// ============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
// ----------------------------------------------------------------------------

// uses 'bslma' allocators
class TestTypeAlloc;
class BitwiseMoveableTestTypeAlloc;

typedef TestTypeAlloc                   TTA;
typedef BitwiseMoveableTestTypeAlloc    BMTTA;

// does not use 'bslma' allocators
class SmallTestTypeNoAlloc;
class MediumTestTypeNoAlloc;
class LargeTestTypeNoAlloc;
class BitwiseCopyableTestTypeNoAlloc;

typedef SmallTestTypeNoAlloc            SmlTT;
typedef MediumTestTypeNoAlloc           MedTT;
typedef LargeTestTypeNoAlloc            LrgTT;
typedef BitwiseCopyableTestTypeNoAlloc  BCTT;

typedef bsls::Types::IntPtr             IntPtr;
typedef bsls::Types::Int64              Int64;
typedef bsls::Types::Uint64             Uint64;

template <class T>
struct PageLength {
    // This class indicates, through an 'enum' 'k_VALUE', the number of objects
    // that can be inserted onto a page of a 'deque<T>'.  Note that the
    // effective capacity when inserting at the back is one fewer, as the last
    // slot must be occupied by a non-created object, which is the target of
    // the 'end' iterator.  This slot *is* occupied when inserting the last
    // element, but it requires allocating a fresh page with a new address for
    // the dummy one-past-the-end object.  This template must be specialized
    // with a hand-computed result for each type under test, in order to verify
    // that the internally computed page-size corresponds to the value that we
    // wish to test.
    //
    // Note that the explicit specializations of this class that defined its
    // value members must follow all the types that we wish to support in this
    // file, so that we can evaluate static assertions on the types' sizes.

    enum { k_VALUE = 16 };  // default; see specializations below
};

// CONSTANTS
const int MAX_ALIGN = bsls::AlignmentUtil::BSLS_MAX_ALIGNMENT;

const char UNINITIALIZED_VALUE = '_';
const char DEFAULT_VALUE       = 'z';
const char VA = 'A';
const char VB = 'B';
const char VC = 'C';
const char VD = 'D';
const char VE = 'E';
    // All test types have character value type.

const int NOMINAL_BLOCK_BYTES = bsl::Deque_BlockLengthCalcUtil<int>::
                                                            DEFAULT_BLOCK_SIZE;
    // Used 'int', but any type will do here, since DEFAULT_BLOCK_SIZE is
    // independent of the type.

const int MEDIUM_TEST_TYPE_SIZE = NOMINAL_BLOCK_BYTES / 2;
    // Size such that there should be only two objects in a deque block.

const int LARGE_TEST_TYPE_SIZE = NOMINAL_BLOCK_BYTES * 2;
    // Size such that there should be only a single object in a deque block.

const int  LARGE_SIZE_VALUE = 10;
    // Declare a large value for insertions into the deque.  Note this value
    // will cause multiple resizes during insertion into the deque.

const int NUM_ALLOCS[] = {
    // Number of allocations (blocks) to create a deque of the following size
    // by using 'push_back' repeatedly (without initial reserve):
    //
    // 0    1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17
    // --   --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --
       0,   1,  2,  3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  6
};

const int NUM_PADDING = 2;
    // BLOCK_ARRAY_PADDING as defined in 'bslalg_dequeimputil'.

const int NUM_INTERNAL_STATE_TEST = 10;
    // Number of different internal states to check.

const int DEFAULT_ALLOC_BLOCKS = 2;
    // Number of blocks allocated by a test allocator when default-constructing
    // a deque, or a moved-from state.

const int k_MINIMUM_PAGE_LENGTH = NOMINAL_BLOCK_BYTES;

// Define values used to initialize positional arguments for
// 'bsltf::EmplacableTestType' and 'bsltf::AllocEmplacableTestType'
// constructors.  Note, that you cannot change those values as they are used by
// 'TemplateTestFacility::getIdentifier' to map the constructed emplacable
// objects to their integer identifiers.
static const int V01 = 1;
static const int V02 = 20;
static const int V03 = 23;
static const int V04 = 44;
static const int V05 = 66;
static const int V06 = 176;
static const int V07 = 878;
static const int V08 = 8;
static const int V09 = 912;
static const int V10 = 102;

// ============================================================================
//                      GLOBAL HELPER FUNCTIONS FOR TESTING
// ----------------------------------------------------------------------------

typedef bsls::Types::Int64 Int64;

// Define DEFAULT DATA used in multiple test cases.

static const size_t DEFAULT_MAX_LENGTH = 32;

struct DefaultDataRow {
    int         d_line;     // source line number
    int         d_index;    // lexical order
    const char *d_spec;     // specification string, for input to 'gg' function
    const char *d_results;  // canonical equivalent spec
};

static
const DefaultDataRow DEFAULT_DATA[] = {
    //line idx  spec                 results
    //---- ---  --------             -------
    { L_,    0, "",                  "" },
    { L_,    1, "a",                 "A" },
    { L_,    2, "aa",                "AA" },
    { L_,    3, "aBCaBC",            "AABCBC" },
    { L_,    4, "aBCa",              "AABC" },
    { L_,    5, "aB",                "AB" },
    { L_,    6, "BCaB",              "ABCB" },
    { L_,    7, "BaC",               "ABC" },
    { L_,    9, "BCDa",              "ABCD" },
    { L_,   10, "BCaDE",             "ABCDE" },
    { L_,   11, "FEDCBa",            "AFEDCB" },
    { L_,   13, "BCDEFaGH",          "ABCDEFGH" },
    { L_,   14, "GCDFEabHI",         "BAGCDFEHI" },
    { L_,   15, "BFEDFGaMNOPHbIJKL", "BABFEDFGMNOPHIJKL" },
    { L_,   17, "BaD",               "ABD" },
    { L_,   19, "aC",                "AC" },
    { L_,   20, "B",                 "B" },
    { L_,   21, "BCDE",              "BCDE" },
    { L_,   22, "FEDCBc",            "CFEDCB" },
    { L_,   23, "cde",               "EDC" },
};
enum { DEFAULT_NUM_DATA = sizeof DEFAULT_DATA / sizeof *DEFAULT_DATA };

// ============================================================================
//                      GLOBAL HELPER FUNCTIONS FOR TESTING
// ----------------------------------------------------------------------------

namespace bsl {

template <class TYPE, class ALLOC>
void debugprint(const bsl::deque<TYPE,ALLOC>& v)
{
    using bsls::debugprint;

    if (v.empty()) {
        printf("<empty>");
    }
    else {
        putchar('"');
        for (size_t ii = 0; ii < v.size(); ++ii) {
            putchar(static_cast<char>(
                           bsltf::TemplateTestFacility::getIdentifier(v[ii])));
        }
        putchar('"');
    }
}

}  // close namespace bsl

template <class CONTAINER, class VALUES>
size_t verifyContainer(const CONTAINER& container,
                       const VALUES&    expectedValues,
                       size_t           expectedSize)
    // Verify the specified 'container' has the specified 'expectedSize' and
    // contains the same values as the array in the specified 'expectedValues'.
    // Return 0 if 'container' has the expected values, and a non-zero value
    // otherwise.
{
    ASSERTV(expectedSize, container.size(), expectedSize == container.size());

    if (expectedSize != container.size()) {
        return static_cast<size_t>(-1);                               // RETURN
    }

    typename CONTAINER::const_iterator it = container.cbegin();
    for (size_t i = 0; i < expectedSize; ++i) {
        ASSERTV(it != container.cend());
        ASSERTV(i, expectedValues[i], *it, expectedValues[i] == *it);

        if (bsltf::TemplateTestFacility::getIdentifier(expectedValues[i])
            != bsltf::TemplateTestFacility::getIdentifier(*it)) {
            return i + 1;                                             // RETURN
        }
        ++it;
    }
    return 0;
}

template <class CONTAINER>
size_t numMovedInto(const CONTAINER& X,
                    size_t           startIndex = 0,
                    size_t           endIndex   = 0)
{
    typedef bsltf::TemplateTestFacility TstFacility;
    typedef bsltf::MoveState            MoveState;

    int numMoved = 0;
    size_t upTo = endIndex > startIndex ? endIndex : X.size();
    for (size_t i = startIndex; i < upTo; ++i) {
        MoveState::Enum mState = TstFacility::getMovedIntoState(X[i]);
        if (MoveState::e_MOVED == mState || MoveState::e_UNKNOWN == mState) {
            ++numMoved;
        }
    }
    return numMoved;
}

template <class CONTAINER>
size_t numNotMovedInto(const CONTAINER& X,
                       size_t           startIndex = 0,
                       size_t           endIndex   = 0)
{
    typedef bsltf::TemplateTestFacility TstFacility;
    typedef bsltf::MoveState            MoveState;

    int numNotMoved = 0;
    size_t upTo = endIndex > startIndex ? endIndex : X.size();
    for (size_t i = startIndex; i < upTo; ++i) {
        MoveState::Enum mState = TstFacility::getMovedIntoState(X[i]);
        if (MoveState::e_NOT_MOVED == mState
         || MoveState::e_UNKNOWN   == mState) {
            ++numNotMoved;
        }
    }
    return numNotMoved;
}

template <class CONTAINER>
bool verifySpec(const CONTAINER& c, const char *spec)
{
    const size_t sz = c.size();
    if (strlen(spec) != sz) {
        return false;                                                 // RETURN
    }

    for (unsigned ii = 0; ii < sz; ++ii) {
        if (c[ii] != spec[ii]) {
            return false;                                             // RETURN
        }
    }

    return true;
}

// ============================================================================
//                       GLOBAL HELPER CLASSES FOR TESTING
// ----------------------------------------------------------------------------

// STATIC DATA
static bool             verbose;
static bool         veryVerbose;
static bool     veryVeryVerbose;
static bool veryVeryVeryVerbose;

static int numDefaultCtorCalls = 0;
static int numCharCtorCalls    = 0;
static int numCopyCtorCalls    = 0;
static int numAssignmentCalls  = 0;
static int numDestructorCalls  = 0;

                              // ===============
                              // Usage Example 1
                              // ===============

///Usage
///-----
// In this section we show intended usage of this component.
//
///Example 1: Using a 'deque' to Implement a Laundry Queue
///- - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose we want to define a class to maintain a process queue of names of
// customers who are dropping off their laundry at a drop-off laundry service.
// We can accomplish this by defining a new class characterizing a
// laundry-process queue that uses 'bsl::deque' in its implementation.
//
// The process queue provides two methods, 'push' and 'expeditedPush', for
// inserting names of customers onto the queue.  When calling the 'push'
// method, the customer's name will be inserted at the end of the queue -- his
// laundry will be done after the laundry of customers previously on the queue.
// The 'expeditedPush' method is reserved for customers who have bribed the
// merchant for expedited service.  When calling the 'expeditedPush' method,
// the customer's name will be inserted onto the front of the queue -- his
// laundry will be done before customers previously on the queue.
//
// When the workers are ready to do some laundry, they call the 'next' method
// of the queue, which returns the name of the customer whose laundry is to be
// done next.  For brevity of the usage example, we do not show how customers
// are track while or after their laundry is being done.
//
// In addition, the laundry queue also provides the 'find' method, which
// returns a 'bool' to indicate whether a given customer is still in the queue.
//
// First, we declare a class 'LaundryQueue' based on a deque, to store names of
// customers at a drop-off laundry:

class LaundryQueue {
    // This 'class' keeps track of customers enqueued to have their laundry
    // done by a laundromat.

    // DATA
    bsl::deque<bsl::string, bsl::allocator<bsl::string> > d_queue;

  public:
    // CREATORS
    explicit
    LaundryQueue(bslma::Allocator *basicAllocator = 0);
        // Create a 'LaundryQueue' object.  Optionally specify a
        // 'basicAllocator' used to supply memory.  If 'basicAllocator' is not
        // supplied, the default allocator is used.

    // MANIPULATORS
    void push(const bsl::string& customerName);
        // Add the specified 'customerName' to the back of the laundry queue.

    void expeditedPush(const bsl::string& customerName);
        // Add the specified 'customerName' to the laundry queue at the front.

    bsl::string next();
        // Return the name from the front of the queue, removing it from the
        // queue.  If the queue is empty, return "(* empty *)" which is not a
        // valid name for a customer.

    // ACCESSORS
    bool find(const bsl::string& customerName);
        // Return 'true' if the specified 'customerName' is in the queue, and
        // 'false' otherwise.
};

// Then, we define the implementation of the methods of 'LaundryQueue'

// CREATORS
LaundryQueue::LaundryQueue(bslma::Allocator *basicAllocator)
: d_queue(basicAllocator)
{
    // Note that the allocator is propagated to the underlying 'deque', which
    // will use the default allocator is '0 == basicAllocator'.
}

// MANIPULATORS
void LaundryQueue::push(const bsl::string& customerName)
{
    d_queue.push_back(customerName);     // note constant time
}

void LaundryQueue::expeditedPush(const bsl::string& customerName)
{
    d_queue.push_front(customerName);    // note constant time
}

bsl::string LaundryQueue::next()
{
    if (d_queue.empty()) {
        return "(* empty *)";                                         // RETURN
    }

    bsl::string ret = d_queue.front();   // note constant time

    d_queue.pop_front();                 // note constant time

    return ret;
}

// ACCESSORS
bool LaundryQueue::find(const bsl::string& customerName)
{
    // Note 'd_queue.empty() || d_queue[0] == d_queue.front()'

    for (size_t i = 0; i < d_queue.size(); ++i) {
        if (customerName == d_queue[i]) {    // note '[]' is constant time
            return true;                                              // RETURN
        }
    }

    return false;
}

                              // ===============
                              // Usage Example 2
                              // ===============

// It was decided to eliminate this second usage example from the .h file as it
// was too long, it is being kept as a test case, and perhaps here it may
// inform someone trying to familiarize themselves with the component.

///Usage
///-----
// In this section we show intended usage of this component.
//
///Example 2: Showing Properties of a Deque and its Iterators
/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// A 'deque' (pronounced 'deck') is a *D*ouble *E*nded *QUE*ue.  One can
// efficiently push or pop elements to the front or end of the queue.  It has
// iterators and reverse iterators which are quite symmetrical.
//
// First, we define a function 'checkInvariants' which will check properties
// that should always be true for any deque in a valid state.:
//..
template <class TYPE>
void checkInvariants(const deque<TYPE>& d)
{
    const size_t SZ = d.size();

    {
        // The following tests will be valid for any deque in a valid state.

        ASSERT((0 == SZ) == d.empty());

        ASSERT(d.end() - d.begin() == d.rend() - d.rbegin());
        ASSERT(d.end() - d.begin() >= 0);
        ASSERT(SZ == (size_t) (d.end() - d.begin()));

        for (size_t u = 0; u < SZ; ++u) {
            ASSERT(&*(d.begin()    + u) == &d[         u]);
            ASSERT(&*(d.end()  - 1 - u) == &d[SZ - 1 - u]);
            ASSERT(&*(d.rbegin()   + u) == &d[SZ - 1 - u]);
            ASSERT(&*(d.rend() - 1 - u) == &d[         u]);
        }
    }

    if (d.empty()) {
        return;                                                       // RETURN
    }

    {
        // The rest of the tests are only valid if the deque contains at least
        // one element, but are always valid in that case.

        ASSERT(&d[0]                   == &d.front());
        ASSERT(&d[SZ - 1]              == &d.back());

        ASSERT(&*d.begin()             == &d.front());
        ASSERT(&*d.rbegin()            == &d.back());

        ASSERT(&*(d.begin()  + SZ - 1) == &d.back());
        ASSERT(&*(d.rbegin() + SZ - 1) == &d.front());

        ASSERT(&*(d.end()         - 1) == &d.back());
        ASSERT(&*(d.rend()        - 1) == &d.front());

        ASSERT(&*(d.end()    - SZ)     == &d.front());
        ASSERT(&*(d.rend()   - SZ)     == &d.back());
    }
}
//..

                            // ==========================
                            // class StatefulStlAllocator
                            // ==========================

template <class VALUE>
class StatefulStlAllocator : public bsltf::StdTestAllocator<VALUE>
    // This class implements a standard compliant allocator that has an
    // attribute, 'id'.
{
    // DATA
    int d_id;  // identifier

  private:
    // TYPES
    typedef bsltf::StdTestAllocator<VALUE> StlAlloc;
        // Alias for the base class.

  public:
    template <class OTHER_TYPE>
    struct rebind {
        // This nested 'struct' template, parameterized by some 'OTHER_TYPE',
        // provides a namespace for an 'other' type alias, which is an
        // allocator type following the same template as this one but that
        // allocates elements of 'OTHER_TYPE'.  Note that this allocator type
        // is convertible to and from 'other' for any 'OTHER_TYPE' including
        // 'void'.

        typedef StatefulStlAllocator<OTHER_TYPE> other;
    };

    // CREATORS
    StatefulStlAllocator()
        // Create a 'StatefulStlAllocator' object.
    : StlAlloc()
    {
    }

    //! StatefulStlAllocator(const StatefulStlAllocator& original) = default;
        // Create a 'StatefulStlAllocator' object having the same id as the
        // specified 'original'.

    template <class OTHER_TYPE>
    StatefulStlAllocator(const StatefulStlAllocator<OTHER_TYPE>& original)
        // Create a 'StatefulStlAllocator' object having the same id as the
        // specified 'original' with a different template type.
    : StlAlloc(original)
    , d_id(original.id())
    {
    }

    // MANIPULATORS
    void setId(int value)
        // Set the 'id' attribute of this object to the specified 'value'.
    {
        d_id = value;
    }

    // ACCESSORS
    int id() const
        // Return the value of the 'id' attribute of this object.
    {
        return d_id;
    }
};

                            // ======================
                            // class ExceptionProctor
                            // ======================

template <class OBJECT, class ALLOCATOR>
class ExceptionProctor {
    // This class provides a mechanism to verify the strong exception guarantee
    // in exception-throwing code.  On construction, this class stores a copy
    // of an object of the (template parameter) type 'OBJECT' and the address
    // of that object.  On destruction, if 'release' was not invoked, it will
    // verify the value of the object is the same as the value of the copy
    // created on construction.  This class requires that the copy constructor
    // and 'operator ==' to be tested before use.

    // DATA
    int                         d_line;      // line number at construction
    bsls::ObjectBuffer<OBJECT>  d_control;   // copy of the object being tested
    const OBJECT               *d_object_p;  // address of the original object

  private:
    // NOT IMPLEMENTED
    ExceptionProctor(const ExceptionProctor&);
    ExceptionProctor& operator=(const ExceptionProctor&);

  public:
    // CREATORS
    ExceptionProctor(const OBJECT    *object,
                     int              line,
                     const ALLOCATOR& basicAllocator = ALLOCATOR())
    : d_line(line)
    , d_object_p(object)
        // Create an exception proctor for the specified 'object' at the
        // specified 'line' number that uses the specified 'basicAllocator' to
        // supply memory for the control object.
    {
        if (d_object_p) {
            new (d_control.buffer()) OBJECT(*object, basicAllocator);
        }
    }

    ExceptionProctor(const OBJECT     *object,
                     const OBJECT&     control,
                     int               line,
                     const ALLOCATOR&  basicAllocator = ALLOCATOR())
    : d_line(line)
    , d_object_p(object)
        // Create an exception proctor for the specified 'object' at
        // the specified 'line' number using the specified 'control'
        // object.  Use the specified 'basicAllocator' to supply
        // memory for the control object.
    {
        if (d_object_p) {
            new (d_control.buffer()) OBJECT(control, basicAllocator);
        }
    }

    ExceptionProctor(const OBJECT              *object,
                     bslmf::MovableRef<OBJECT>  control,
                     int                        line)
    : d_line(line)
    , d_object_p(object)
        // Create an exception proctor for the specified 'object' at the
        // specified 'line' number using the specified 'control' object.
    {
        if (d_object_p) {
            new (d_control.buffer()) OBJECT(
                                         bslmf::MovableRefUtil::move(control));
        }
    }

    ~ExceptionProctor()
        // Destroy the exception proctor.  If the proctor was not released,
        // verify that the state of the object supplied at construction has not
        // changed.
    {
        if (d_object_p) {
            const int LINE = d_line;
            ASSERTV(LINE, d_control.object(), *d_object_p,
                    d_control.object() == *d_object_p);
            d_control.object().~OBJECT();
        }
    }

    // MANIPULATORS
    void release()
        // Release the guard from verifying the state of the object.
    {
        if (d_object_p) {
            d_control.object().~OBJECT();
        }
        d_object_p = 0;
    }
};

                               // ===================
                               // class TestTypeAlloc
                               // ===================

class TestTypeAlloc {
    // This test type contains a 'char' in some allocated storage.  It counts
    // the number of default constructions, copy constructions, assignments,
    // and destructions.  It declares the 'bslma::UsesBslmaAllocator' trait.
    // It could also have the bitwise moveable trait, but we defer that to the
    // 'BitwiseMoveableTestTypeAlloc' type.

    // DATA
    char             *d_data_p;
    bslma::Allocator *d_allocator_p;

#if defined(BDE_USE_ADDRESSOF)
    // PRIVATE ACCESSORS
    void operator&() const;     // = delete;
        // Suppress the use of address-of operator on this type.
#endif

  public:
    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(TestTypeAlloc, bslma::UsesBslmaAllocator);

    // CREATORS
    explicit
    TestTypeAlloc(bslma::Allocator *ba = 0)
    : d_data_p(0)
    , d_allocator_p(bslma::Default::allocator(ba))
    {
        ++numDefaultCtorCalls;
        d_data_p  = (char *)d_allocator_p->allocate(sizeof(char));
        *d_data_p = DEFAULT_VALUE;
    }

    explicit
    TestTypeAlloc(int c, bslma::Allocator *ba = 0)
    : d_data_p(0)
    , d_allocator_p(bslma::Default::allocator(ba))
    {
        ++numCharCtorCalls;
        d_data_p  = (char *)d_allocator_p->allocate(sizeof(char));
        *d_data_p = static_cast<char>(c);
    }

    TestTypeAlloc(const TestTypeAlloc& original, bslma::Allocator *ba = 0)
    : d_data_p(0)
    , d_allocator_p(bslma::Default::allocator(ba))
    {
        ++numCopyCtorCalls;
        d_data_p  = (char *)d_allocator_p->allocate(sizeof(char));
        *d_data_p = *original.d_data_p;
    }

    ~TestTypeAlloc()
    {
        ++numDestructorCalls;
        *d_data_p = UNINITIALIZED_VALUE;
        d_allocator_p->deallocate(d_data_p);
        d_data_p = 0;
        d_allocator_p = 0;
    }

    // MANIPULATORS
    TestTypeAlloc& operator=(const TestTypeAlloc& rhs)
    {
        ++numAssignmentCalls;
        if (BSLS_UTIL_ADDRESSOF(rhs) != this) {
            char *newData = (char *)d_allocator_p->allocate(sizeof(char));
            *d_data_p = UNINITIALIZED_VALUE;
            d_allocator_p->deallocate(d_data_p);
            d_data_p  = newData;
            *d_data_p = *rhs.d_data_p;
        }
        return *this;
    }

    void setDatum(char c)
    {
        *d_data_p = c;
    }

    // ACCESSORS
    operator int() const
    {
        return static_cast<int>(*d_data_p);
    }

    char value() const
    {
        return *d_data_p;
    }

    bslma::Allocator *allocator() const
    {
        return d_allocator_p;
    }

    void print() const
    {
        if (d_data_p) {
            ASSERT(isalpha(*d_data_p));
            printf("%c (int: %d)\n", *d_data_p, (int)*d_data_p);
        } else {
            printf("VOID\n");
        }
    }
};

// FREE OPERATORS
bool operator==(const TestTypeAlloc& lhs, const TestTypeAlloc& rhs)
{
    ASSERT(isalpha(lhs.value()));
    ASSERT(isalpha(rhs.value()));

    return lhs.value() == rhs.value();
}

bool operator<(const TestTypeAlloc& lhs, const TestTypeAlloc& rhs)
{
    ASSERT(isalpha(lhs.value()));
    ASSERT(isalpha(rhs.value()));

    return lhs.value() < rhs.value();
}

// TestTypeAlloc-specific print function.
void debugprint(const TestTypeAlloc& rhs)
{
    printf("%c", rhs.value());
//    fflush(stdout);
}

                       // ==========================
                       // class SmallTestTypeNoAlloc
                       // ==========================

class SmallTestTypeNoAlloc {
    // This test type has a footprint and interface identical to
    // 'TestTypeAlloc', except that it does not use allocators.  It also counts
    // the number of default constructions, copy constructions, assignments,
    // and destructions.  It does not allocate, and thus could have the bitwise
    // copyable trait, but we defer that to the
    // 'BitwiseCopyableTestTypeNoAlloc' type.

    // DATA
    union {
        char d_char;
        char d_fill[sizeof(TestTypeAlloc)];
    } d_u;

  public:
    // CREATORS
    SmallTestTypeNoAlloc()
    {
        d_u.d_char = DEFAULT_VALUE;
        ++numDefaultCtorCalls;
    }

    explicit
    SmallTestTypeNoAlloc(int c)
    {
        d_u.d_char = static_cast<char>(c);
        ++numCharCtorCalls;
    }

    SmallTestTypeNoAlloc(const SmallTestTypeNoAlloc& original)
    {
        d_u.d_char = original.d_u.d_char;
        ++numCopyCtorCalls;
    }

    ~SmallTestTypeNoAlloc()
    {
        ++numDestructorCalls;
        d_u.d_char = '_';
    }

    // MANIPULATORS
    SmallTestTypeNoAlloc& operator=(const SmallTestTypeNoAlloc& rhs)
    {
        ++numAssignmentCalls;
        d_u.d_char = rhs.d_u.d_char;
        return *this;
    }

    // ACCESSORS
    operator int() const
    {
        return static_cast<int>(d_u.d_char);
    }

    char value() const
    {
        return d_u.d_char;
    }

    void print() const
    {
        ASSERT(isalpha(d_u.d_char));
        printf("%c (int: %d)\n", d_u.d_char, (int)d_u.d_char);
    }
};

// FREE OPERATORS
bool operator==(const SmallTestTypeNoAlloc& lhs,
                const SmallTestTypeNoAlloc& rhs)
{
    ASSERT(isalpha(lhs.value()));
    ASSERT(isalpha(rhs.value()));

    return lhs.value() == rhs.value();
}

// SmallTestTypeNoAlloc-specific print function.
void debugprint(const SmallTestTypeNoAlloc& rhs)
{
    printf("%c", rhs.value());
    fflush(stdout);
}

                       // ===========================
                       // class MediumTestTypeNoAlloc
                       // ===========================

class MediumTestTypeNoAlloc {
    // This test type has a larger footprint than 'TestTypeAlloc', so that
    // there are fewer objects in a deque's block.  It also counts the number
    // of default constructions, copy constructions, assignments, and
    // destructions.  It does not allocate, and thus could have the bitwise
    // copyable trait, but we defer that to the
    // 'BitwiseCopyableTestTypeNoAlloc' type.

    // DATA
    union {
        char d_char;
        char d_fill[MEDIUM_TEST_TYPE_SIZE];
    } d_u;

  public:
    // CREATORS
    MediumTestTypeNoAlloc()
    {
        d_u.d_char = DEFAULT_VALUE;
        ++numDefaultCtorCalls;
    }

    explicit
    MediumTestTypeNoAlloc(int c)
    {
        d_u.d_char = static_cast<char>(c);
        ++numCharCtorCalls;
    }

    MediumTestTypeNoAlloc(const MediumTestTypeNoAlloc&  original)
    {
        d_u.d_char = original.d_u.d_char;
        ++numCopyCtorCalls;
    }

    ~MediumTestTypeNoAlloc()
    {
        ++numDestructorCalls;
        d_u.d_char = '_';
    }

    // MANIPULATORS
    MediumTestTypeNoAlloc& operator=(const MediumTestTypeNoAlloc& rhs)
    {
        ++numAssignmentCalls;
        d_u.d_char = rhs.d_u.d_char;
        return *this;
    }

    // ACCESSORS
    operator int() const
    {
        return static_cast<int>(d_u.d_char);
    }

    char value() const
    {
        return d_u.d_char;
    }

    void print() const
    {
        ASSERT(isalpha(d_u.d_char));
        printf("%c (int: %d)\n", d_u.d_char, (int)d_u.d_char);
    }
};

// FREE OPERATORS
bool operator==(const MediumTestTypeNoAlloc& lhs,
                const MediumTestTypeNoAlloc& rhs)
{
    ASSERT(isalpha(lhs.value()));
    ASSERT(isalpha(rhs.value()));

    return lhs.value() == rhs.value();
}

// MediumTestTypeNoAlloc-specific print function.
void debugprint(const MediumTestTypeNoAlloc& rhs)
{
    printf("%c", rhs.value());
//    fflush(stdout);
}

                       // ==========================
                       // class LargeTestTypeNoAlloc
                       // ==========================

class LargeTestTypeNoAlloc {
    // This test type has a much larger footprint than 'TestTypeAlloc', so that
    // there are just 16 objects in a deque's block.  It also counts the number
    // of default constructions, copy constructions, assignments, and
    // destructions.  It does not allocate, and thus could have the bitwise
    // copyable trait, but we defer that to the
    // 'BitwiseCopyableTestTypeNoAlloc' type.

    // DATA
    union {
        char d_char;
        char d_fill[LARGE_TEST_TYPE_SIZE];
    } d_u;

  public:
    // CREATORS
    LargeTestTypeNoAlloc()
    {
        d_u.d_char = DEFAULT_VALUE;
        ++numDefaultCtorCalls;
    }

    explicit
    LargeTestTypeNoAlloc(int c)
    {
        d_u.d_char = static_cast<char>(c);
        ++numCharCtorCalls;
    }

    LargeTestTypeNoAlloc(const LargeTestTypeNoAlloc&  original)
    {
        d_u.d_char = original.d_u.d_char;
        ++numCopyCtorCalls;
    }

    ~LargeTestTypeNoAlloc()
    {
        ++numDestructorCalls;
        d_u.d_char = '_';
    }

    // MANIPULATORS
    LargeTestTypeNoAlloc& operator=(const LargeTestTypeNoAlloc& rhs)
    {
        ++numAssignmentCalls;
        d_u.d_char = rhs.d_u.d_char;
        return *this;
    }

    // ACCESSORS
    operator int() const
    {
        return static_cast<int>(d_u.d_char);
    }

    char value() const
    {
        return d_u.d_char;
    }

    void print() const
    {
        ASSERT(isalpha(d_u.d_char));
        printf("%c (int: %d)\n", d_u.d_char, (int)d_u.d_char);
    }
};

// FREE OPERATORS
bool operator==(const LargeTestTypeNoAlloc& lhs,
                const LargeTestTypeNoAlloc& rhs)
{
    ASSERT(isalpha(lhs.value()));
    ASSERT(isalpha(rhs.value()));

    return lhs.value() == rhs.value();
}

// LargeTestTypeNoAlloc-specific print function.
void debugprint(const LargeTestTypeNoAlloc& rhs)
{
    printf("%c", rhs.value());
//    fflush(stdout);
}

                       // ==================================
                       // class BitwiseMoveableTestTypeAlloc
                       // ==================================

class BitwiseMoveableTestTypeAlloc : public TestTypeAlloc {
    // This test type is identical to 'TestTypeAlloc' except that it has the
    // bitwise moveable trait.  All members are inherited.

  public:
    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(BitwiseMoveableTestTypeAlloc,
                                   bslma::UsesBslmaAllocator);
    BSLMF_NESTED_TRAIT_DECLARATION(BitwiseMoveableTestTypeAlloc,
                                   bslmf::IsBitwiseMoveable);

    // CREATORS
    explicit
    BitwiseMoveableTestTypeAlloc(bslma::Allocator *ba = 0)
    : TestTypeAlloc(ba)
    {
    }

    explicit
    BitwiseMoveableTestTypeAlloc(int c, bslma::Allocator *ba = 0)
    : TestTypeAlloc(c, ba)
    {
    }

    BitwiseMoveableTestTypeAlloc(const BitwiseMoveableTestTypeAlloc&  original,
                                 bslma::Allocator                    *ba = 0)
    : TestTypeAlloc(original, ba)
    {
    }
};

                       // ====================================
                       // class BitwiseCopyableTestTypeNoAlloc
                       // ====================================

class BitwiseCopyableTestTypeNoAlloc : public SmallTestTypeNoAlloc {
    // This test type is identical to 'SmallTestTypeNoAlloc' except that it has
    // the bitwise copyable and bitwise equality-comparable traits.  All
    // members are inherited.

  public:
    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(BitwiseCopyableTestTypeNoAlloc,
                                   bsl::is_trivially_copyable);
    BSLMF_NESTED_TRAIT_DECLARATION(BitwiseCopyableTestTypeNoAlloc,
                                   bslmf::IsBitwiseEqualityComparable);

    // CREATORS
    BitwiseCopyableTestTypeNoAlloc()
    : SmallTestTypeNoAlloc()
    {
    }

    explicit
    BitwiseCopyableTestTypeNoAlloc(int c)
    : SmallTestTypeNoAlloc(c)
    {
        ++numCharCtorCalls;
    }

    BitwiseCopyableTestTypeNoAlloc(
                                const BitwiseCopyableTestTypeNoAlloc& original)
    : SmallTestTypeNoAlloc(original.value())
    {
    }
};

                               // ==============
                               // class CharList
                               // ==============

template <class TYPE>
class CharList {
    // This array class is a simple wrapper on a 'char' array offering an input
    // iterator access via the 'begin' and 'end' accessors.  The iterator is
    // specifically an *input* iterator and its value type is the (template
    // parameter) 'TYPE'.

    // DATA
    bsl::vector<TYPE> d_value;

  public:
    // TYPES
    typedef bslstl::ForwardIterator<const TYPE, const TYPE *> const_iterator;
        // Input iterator.

    // CREATORS
    CharList() {}
    explicit CharList(const bsl::vector<TYPE>& value);

    // ACCESSORS
    const TYPE& operator[](size_t index) const;
    const_iterator begin() const;
    const_iterator end() const;
};

// CREATORS
template <class TYPE>
CharList<TYPE>::CharList(const bsl::vector<TYPE>& value)
: d_value(value)
{
}

// ACCESSORS
template <class TYPE>
const TYPE&
CharList<TYPE>::operator[](size_t index) const
{
    return d_value[index];
}

template <class TYPE>
typename CharList<TYPE>::const_iterator
CharList<TYPE>::begin() const
{
    return const_iterator(d_value.begin());
}

template <class TYPE>
typename CharList<TYPE>::const_iterator
CharList<TYPE>::end() const
{
    return const_iterator(d_value.end());
}

                              // ===============
                              // class CharArray
                              // ===============

template <class TYPE>
class CharArray {
    // This array class is a simple wrapper on a deque offering an input
    // iterator access via the 'begin' and 'end' accessors.  The iterator is
    // specifically a *random-access* iterator and its value type is the
    // (template parameter) 'TYPE'.

    // DATA
    bsl::vector<TYPE>  d_value;

  public:
    // TYPES
    typedef const TYPE *const_iterator;
        // Random-access iterator.

    // CREATORS
    CharArray() {}
    explicit CharArray(const bsl::vector<TYPE>& value);

    // ACCESSORS
    const TYPE& operator[](size_t index) const;
    const_iterator begin() const;
    const_iterator end() const;
};

// CREATORS
template <class TYPE>
CharArray<TYPE>::CharArray(const bsl::vector<TYPE>& value)
: d_value(value)
{
}

// ACCESSORS
template <class TYPE>
const TYPE& CharArray<TYPE>::operator[](size_t index) const
{
    return d_value[index];
}

template <class TYPE>
typename CharArray<TYPE>::const_iterator CharArray<TYPE>::begin() const
{
    return const_iterator(d_value.begin());
}

template <class TYPE>
typename CharArray<TYPE>::const_iterator CharArray<TYPE>::end() const
{
    return const_iterator(d_value.end());
}

                              // ====================
                              // class LimitAllocator
                              // ====================

template <class ALLOC>
class LimitAllocator : public ALLOC {

  public:
    // TYPES
    typedef typename ALLOC::value_type        value_type;
    typedef typename ALLOC::pointer           pointer;
    typedef typename ALLOC::const_pointer     const_pointer;
    typedef typename ALLOC::reference         reference;
    typedef typename ALLOC::const_reference   const_reference;
    typedef typename ALLOC::size_type         size_type;
    typedef typename ALLOC::difference_type   difference_type;

    template <class OTHER_TYPE>
    struct rebind {
        // It is better not to inherit the rebind template, or else
        // 'rebind<X>::other' would be 'ALLOC::rebind<OTHER_TYPE>::other'
        // instead of 'LimitAlloc<X>'.

        typedef LimitAllocator<typename ALLOC::template
                                             rebind<OTHER_TYPE>::other > other;
    };

  private:
    // PRIVATE TYPES
    typedef ALLOC AllocBase;

    // DATA
    size_type d_limit;

  private:
    // NOT IMPLEMENTED
    LimitAllocator& operator=(const LimitAllocator&);

  public:
    // CREATORS
    LimitAllocator()
    : d_limit(-1) {}

    explicit
    LimitAllocator(bslma::Allocator *mechanism)
    : AllocBase(mechanism)
    , d_limit(-1)
    {
    }

    explicit
    LimitAllocator(const ALLOC& alloc)
    : AllocBase((const AllocBase&)alloc)
    , d_limit(-1)
    {
    }

    ~LimitAllocator()
    {
    }

    // MANIPULATORS
    void setMaxSize(size_type maxSize)
    {
        d_limit = maxSize;
    }

    // ACCESSORS
    size_type max_size() const
    {
        return d_limit;
    }
};

namespace BloombergLP {
namespace bslmf {

template <class ALLOCATOR>
struct IsBitwiseMoveable<LimitAllocator<ALLOCATOR> >
    : IsBitwiseMoveable<ALLOCATOR>
{
};

}  // close namespace bslmf
}  // close enterprise namespace


                        // =========================
                        // class template PageLength
                        // =========================

template <>
struct PageLength<char> {
    typedef char Type;

    BSLMF_ASSERT(sizeof(Type) == 1);  // by definition, but preserves pattern

    enum { k_VALUE = 200 };
};

template <>
struct PageLength<TTA> {
    typedef TTA Type;

    BSLMF_ASSERT(sizeof(Type) == 2 * sizeof(void *));

    enum { k_VALUE = sizeof(char *) == 4 ? 25
                   : sizeof(char *) == 8 ? 16
                   : 0 };
};

template <>
struct PageLength<SmlTT> {
    typedef SmlTT Type;

    BSLMF_ASSERT(sizeof(Type) == 2 * sizeof(void *));

    enum { k_VALUE = sizeof(char *) == 4 ? 25
                   : sizeof(char *) == 8 ? 16
                   : 0 };
};

template <>
struct PageLength<MedTT> {
    typedef MedTT Type;

    BSLMF_ASSERT(sizeof(Type) < k_MINIMUM_PAGE_LENGTH &&
                 sizeof(Type) > k_MINIMUM_PAGE_LENGTH / 16);

    enum { k_VALUE = 16 };
};

template <>
struct PageLength<LrgTT> {
    typedef LrgTT Type;

    BSLMF_ASSERT(sizeof(Type) > k_MINIMUM_PAGE_LENGTH);

    enum { k_VALUE = 16 };
};

template <>
struct PageLength<BMTTA> {
    typedef BMTTA Type;

    BSLMF_ASSERT(sizeof(Type) == 2 * sizeof(void *));

    enum { k_VALUE = sizeof(char *) == 4 ? 25
                   : sizeof(char *) == 8 ? 16
                   : 0 };
};

template <>
struct PageLength<BCTT> {
    typedef BCTT Type;

    BSLMF_ASSERT(sizeof(Type) == 2 * sizeof(void *));

    enum { k_VALUE = sizeof(char *) == 4 ? 25
                   : sizeof(char *) == 8 ? 16
                   : 0 };
};

template <class ITER, class VALUE_TYPE>
struct TestMovableTypeUtil {

  public:
    static ITER findFirstNotMovedInto(ITER begin, ITER end)
    {
        typedef bsltf::TemplateTestFacility TstFacility;
        typedef bsltf::MoveState            MoveState;

        for (; begin != end; ++begin) {
            MoveState::Enum mState = TstFacility::getMovedIntoState(*begin);
            if (MoveState::e_NOT_MOVED == mState) {
                break;
            }
        }
        return begin;
    }
};

struct TestAllocatorUtil {

  public:
    // CLASS METHODS
    template <class TYPE>
    static void test(const TYPE&, const bslma::Allocator&)
    {
    }

    static void test(const bsltf::AllocEmplacableTestType& value,
                     const bslma::Allocator&               allocator)
    {
        ASSERTV(&allocator == value.arg01().allocator());
        ASSERTV(&allocator == value.arg02().allocator());
        ASSERTV(&allocator == value.arg03().allocator());
        ASSERTV(&allocator == value.arg04().allocator());
        ASSERTV(&allocator == value.arg05().allocator());
        ASSERTV(&allocator == value.arg06().allocator());
        ASSERTV(&allocator == value.arg07().allocator());
        ASSERTV(&allocator == value.arg08().allocator());
        ASSERTV(&allocator == value.arg09().allocator());
        ASSERTV(&allocator == value.arg10().allocator());
    }
};

// ============================================================================
//                       TEST DRIVER TEMPLATE
// ----------------------------------------------------------------------------

template <class TYPE, class ALLOC = bsl::allocator<TYPE> >
struct TestDriver {
    // The generating functions interpret the given 'spec' in order from left
    // to right to configure the object according to a custom language.
    // Uppercase letters '[A .. E]' correspond to arbitrary (but unique) 'char'
    // values to be appended to the 'bsl::deque<T>' object.  A tilde ('~')
    // indicates that the logical (but not necessarily physical) state of the
    // object is to be set to its initial, empty state (via the 'clear'
    // method).
    //
    // LANGUAGE SPECIFICATION:
    // -----------------------
    //
    // <SPEC>       ::= <EMPTY>   | <LIST>
    //
    // <EMPTY>      ::=
    //
    // <LIST>       ::= <ITEM>    | <ITEM><LIST>
    //
    // <ITEM>       ::= <ELEMENT> | <POP_BACK> | <CLEAR>
    //
    // <ELEMENT>    ::= 'A' | 'B' | 'C' | 'D' | 'E' | ... | 'H'
    //                                      // unique but otherwise arbitrary
    // <POP_BACK>   ::= '<'
    //
    // <CLEAR>      ::= '~'
    //
    // Spec String  Description
    // -----------  -----------------------------------------------------------
    // ""           Has no effect; leaves the object empty.
    // "A"          Append the value corresponding to A.
    // "AA"         Append two values both corresponding to A.
    // "ABC"        Append three values corresponding to A, B and C.
    // "ABC~"       Append three values corresponding to A, B and C and then
    //              remove all the elements (set array length to 0).  Note that
    //              this spec yields an object that is logically equivalent
    //              (but not necessarily identical internally) to one
    //              yielded by ("").
    // "ABC~DE"     Append three values corresponding to A, B, and C; empty
    //              the object; and append values corresponding to D and E.
    //              (Note that the resulting object has the capacity for at
    //              least three elements at the *front* and none, or the
    //              remainder of the block, at the back.)
    // "ABC<<<~DE"  Append three values corresponding to A, B, and C, then pop
    //              them back; empty the object; and append values
    //              corresponding to D and E.  (Note that the result object has
    //              a capacity for at least one more object at the back, and
    //              none at the front.)
    // ------------------------------------------------------------------------

    // CONSTANTS
    enum {START_POS  = -1 * NUM_PADDING * NOMINAL_BLOCK_BYTES /
                                                           (int) sizeof (TYPE),
        // The integer offset to the first element pointed by the first block
        // pointer of a newly created empty 'bsl::deque'.

          FINISH_POS = ((NUM_PADDING + 1) * NOMINAL_BLOCK_BYTES /
                                                      (int) sizeof (TYPE)) - 1,
        // The integer offset to the last element pointed by the last block
        // pointer of a newly created empty 'bsl::deque'.

          INCREMENT = FINISH_POS - START_POS >= NUM_INTERNAL_STATE_TEST
                    ? (FINISH_POS - START_POS) / NUM_INTERNAL_STATE_TEST
                    : 1
        // The integer increment that will be used when permutating through the
        // various internal states.
    };

    // TYPES
    typedef bsl::deque<TYPE, ALLOC>               Obj;

    typedef typename Obj::iterator                iterator;
    typedef typename Obj::const_iterator          const_iterator;
    typedef typename Obj::reverse_iterator        reverse_iterator;
    typedef typename Obj::const_reverse_iterator  const_reverse_iterator;
    typedef typename Obj::value_type              ValueType;

    typedef typename Obj::const_iterator          CIter;

    typedef bslma::ConstructionUtil               ConstrUtil;
    typedef bslmf::MovableRefUtil                 MoveUtil;

    typedef bsltf::MoveState                      MoveState;
    typedef bsltf::TemplateTestFacility           TstFacility;
    typedef bsltf::TestValuesArray<TYPE>          TestValues;

    typedef TestMovableTypeUtil<CIter, TYPE>      TstMoveUtil;

    typedef bsl::allocator_traits<ALLOC>          AllocatorTraits;

    enum AllocCategory { e_BSLMA, e_STDALLOC, e_ADAPTOR, e_STATEFUL };

    // TEST APPARATUS

    // CLASS DATA
    static
    const AllocCategory s_allocCategory =
                        bsl::is_same<ALLOC, bsl::allocator<TYPE> >::value
                        ? e_BSLMA
                        : bsl::is_same<ALLOC,
                                       bsltf::StdAllocTestType<
                                                bsl::allocator<TYPE> > >::value
                        ? e_STDALLOC
                        : bsl::is_same<ALLOC,
                                       bsltf::StdAllocatorAdaptor<
                                                bsl::allocator<TYPE> > >::value
                        ? e_ADAPTOR
                        : e_STATEFUL;

    static
    const bool s_keyIsMoveEnabled =
                      bsl::is_same<TYPE, bsltf::MovableTestType>::value ||
                      bsl::is_same<TYPE, bsltf::MovableAllocTestType>::value ||
                      bsl::is_same<TYPE, bsltf::MoveOnlyAllocTestType>::value;

    // CLASS METHODS
    static
    const char *allocCategoryAsStr()
    {
        return e_BSLMA == s_allocCategory ? "bslma"
                                          : e_STDALLOC == s_allocCategory
                                          ? "stdalloc"
                                          : e_ADAPTOR == s_allocCategory
                                          ? "adaptor"
                                          : e_STATEFUL == s_allocCategory
                                          ? "stateful"
                                          : "<INVALID>";
    }

    static void primaryManipulatorBack(Obj   *container,
                                       int    identifier)
        // Insert at the back of specified 'container' the value object
        // indicated by the specified 'identifier', ensuring that the overload
        // of the primary manipulator taking a modifiable rvalue is invoked
        // (rather than the one taking an lvalue).

    {
        bsls::ObjectBuffer<ValueType> buffer;
        TstFacility::emplace(buffer.address(),
                             identifier,
                             container->get_allocator());
        bslma::DestructorGuard<ValueType> guard(buffer.address());

        container->push_back(MoveUtil::move(buffer.object()));
    }

    static void primaryManipulatorFront(Obj   *container,
                                        int    identifier)
        // Insert at the front of specified 'container' the value object
        // indicated by the specified 'identifier', ensuring that the overload
        // of the primary manipulator taking a modifiable rvalue is invoked
        // (rather than the one taking an lvalue).
    {
        bsls::ObjectBuffer<ValueType> buffer;
        TstFacility::emplace(buffer.address(),
                             identifier,
                             container->get_allocator());
        bslma::DestructorGuard<ValueType> guard(buffer.address());

        container->push_front(MoveUtil::move(buffer.object()));
    }

    static int getValues(const TYPE **valuesPtr);
        // Load the specified 'valuesPtr' with the address of an array
        // containing initialized values of the (template parameter) 'TYPE',
        // and return the length of that array.

    static int ggg(Obj *object, const char *spec, bool verboseFlag = true);
        // Configure the specified 'object' according to the specified 'spec',
        // using only the primary manipulator function 'push_back' and
        // white-box manipulator 'clear'.  Optionally specify a 'false'
        // 'verboseFlag' to suppress 'spec' syntax error messages.  Return the
        // index of the first invalid character, and a negative value
        // otherwise.  Note that this function is used to implement 'gg' as
        // well as allow for verification of syntax error detection.
        //
        // The language of the grammar consists of characters in the ranges
        // 'a..z' and 'A..Z', to insert a new object to either the front or the
        // back of the deque, '*object', where lower-case indicates the front
        // and upper-case indicates the back.  A sequence of identical objects
        // can be inserted by preceding the letter with a digit, [2..0], where
        // 0 represents 10 objects.  For tests that require filling to a page
        // boundary of the deque, a sequence contained between '<' and '|' will
        // cause a sequence of insertions to fill the deque all the way to the
        // next page boundary with a an object corresponding to a supplied
        // character, where a lower-case character will fill to the front page
        // of the deque, and an upper-case character will fill the back page.
        // Padding to allow a few additional insertions before reaching the
        // page boundary is requested by a sequence of dots, where '...' would
        // mean to leave room for three more elements before reading the page
        // boundary.  Larger sequences of free slots can be requested by a
        // leading digit before a '.', where 0 indicates 10.  No more than 10
        // free slots can be requested this way.  To aid clarity, non-salient
        // whitespace is permitted to easily depict groups of operations.  If
        // the current page is already full, in the desired direction, or does
        // not have sufficient capacity to honor the requested number of free
        // slots, then an error result will be returned.
        //
        // Arbitrary sequences of elements can be pushed to the front and back
        // of a deque using 'ggg' at any point, but the fill-to-boundary
        // functionality is supported only for deques in their initial (default
        // constructed) state, which is empty and a capacity indicating only
        // one page has been allocated.  Any attempt to interpret a 'spec'
        // string containing a fill-to-end range will return an error unless
        // '*object' is in the default-constructed state.  Note that the
        // insert-point for a push in this state must also correspond to the
        // middle of the page, per a default-constructed deque.
        //
        // Examples:
        // ---------
        // "A"      // push_back a single element of configuration 'A'.
        // "a"      // push_front a single element of configuration 'A'.
        // "7A 3b"  // push_back 7 'A' objects, then push_front 3 'B' objects.
        // "A<B|"   // push_back an 'A', then fill-to-back with 'B'.
        // "A<b.|"  // push_back an 'A', then fill-to-front with 'B', leaving
        //          // space at the front for exactly one more object.
        //
        // "<A...|BCC <a...|bcc CCC7D ccc7d"
        //   Fill to the back with 'A' objects, leaving space for 3 more before
        //   an allocation would be required, then push_back a 'B' object
        //   followed by two 'C' objects.  Then, fill to the front with 'A'
        //   objects, leaving space for 3 more before an allocation would be
        //   required, then push_front a 'B' object followed by two 'C'
        //   objects, exactly filling the first page.  Then push_back a 'C'
        //   object (causing an allocation) followed by another 2 C objects,
        //   and then seven 'D' objects.  Then push_front another 'C' object,
        //   followed by two more 'C' objects and then another seven 'D'
        //   objects.
        //
        // "<A| A<A| A<A| 3A"
        //   Fill the current empty deque towards the back with 'A' objects,
        //   filling from the middle of the initial page, creating and filling
        //   two more complete pages, and finally pushed 3 'A' objects to the
        //   start of the fourth page.
        //
        // "<A| A<A| 3A ~ <A| A"
        //   Populate a default-constructed deque to the back of the first page
        //   then fill a newly allocated second page with 'A' objects, and then
        //   insert three more 'A' objects at the start of a third page.  Then
        //   clear the deque, and fill to the back of the current page.
        //   Finally push_back one more 'A' object, which should re-use one of
        //   the existing pages rather than allocate a new page.

    static Obj& gg(Obj *object, const char *spec);
        // Return, by reference, the specified 'object' with its value adjusted
        // according to the specified 'spec'.

    static bsl::vector<TYPE> gV(const char *spec);
        // Return, by value, a new vector corresponding to the specified
        // 'spec'.

    static void storeFirstNElemAddr(typename Obj::const_pointer *pointers,
                                    const Obj&                   object,
                                    size_t                       n)
    {
        size_t i = 0;
        for (CIter b = object.cbegin(); b != object.cend() && i < n; ++b) {
            pointers[i++] = bsls::Util::addressOf(*b);
        }
    }

    static
    int checkFirstNElemAddr(typename Obj::const_pointer *pointers,
                            const Obj&                   object,
                            size_t                       n)
    {
        int    count = 0;
        size_t i     = 0;
        for (CIter b = object.cbegin(); b != object.end() && i < n; ++b) {
            if (pointers[i++] != bsls::Util::addressOf(*b)) {
                ++count;
            }
        }
        return count;
    }

    static void resetMovedInto(Obj *object)
    {
        for (size_t i = 0; i < object->size(); ++i) {
            TstFacility::setMovedIntoState(BSLS_UTIL_ADDRESSOF((*object)[i]),
                                           bsltf::MoveState::e_NOT_MOVED);
        }
    }

    static void setInternalState(Obj *object, int n);
        // Using 'push_back', 'push_front', 'pop_back', and 'pop_front', set
        // the internal state of the specified 'object' by moving the start and
        // finish iterators by the specified 'n' positions.

    static void stretch(Obj         *object,
                        std::size_t  size,
                        int          identifier = int('Z'));
        // Using only primary manipulators, extend the length of the specified
        // 'object' by the specified 'size' by adding 'size' copies of the
        // value indicated by the optionally specified 'identifier'.  If
        // 'identifier' is not specified, 'Z' is used.  The resulting value is
        // not specified.

    static void stretchRemoveAll(Obj         *object,
                                 std::size_t  size,
                                 int          identifier = int('Z'));
        // Using only primary manipulators, extend the capacity of the
        // specified 'object' by the specified 'size' by adding 'size' copies
        // of the value indicated by the optionally specified 'identifier'.  If
        // 'identifier' is not specified, 'Z' is used.  The resulting value is
        // not specified.

    static int findRoomierEnd(Obj *container);
        // Determine which end of the specified '*container' has more room.
        // Return a value > 0 if it's the front, < 0 if it's the back, and
        // 0 if the two ends have the same amount of room.  Note that this
        // function modifies 'container' but restores it to its original state.

    // TEST CASES

    static void testCase11();
        // Test allocator-related concerns.

    static void testCase10();
        // Test streaming functionality.  This test case tests nothing.

    template <bool PROPAGATE_ON_CONTAINER_COPY_ASSIGNMENT_FLAG,
              bool OTHER_FLAGS>
    static void testCase9_propagate_on_container_copy_assignment_dispatch();
    static void testCase9_propagate_on_container_copy_assignment();
        // Test 'propagate_on_container_copy_assignment'.

    static void testCase9();
        // Test copy-assignment operator.

    // static void testCase8();
        // Formerly tested the 'g' function, which is no longer implemented.

    template <bool SELECT_ON_CONTAINER_COPY_CONSTRUCTION_FLAG,
              bool OTHER_FLAGS>
    static void testCase7_select_on_container_copy_construction_dispatch();
    static void testCase7_select_on_container_copy_construction();
        // Test 'select_on_container_copy_construction'.

    static void testCase7();
        // Test copy constructor.

    static void testCase6();
        // Test equality operator ('operator==').

    static void testCase5();
        // Test output (<<) operator.  This test case tests nothing.

    static void testCase4();
        // Test basic accessors ('size' and 'operator[]').

    static void testCase3();
        // Test generator functions 'ggg' and 'gg'.

    static void testCase2a();
        // Old test 2, now 2a.

    static void testCase2();
        // Test primary manipulators ('push_back', 'push_front', and 'clear').

    static void testCase1();
        // Breathing test.  This test *exercises* basic functionality but
        // *test* nothing.
};

template <class TYPE>
class StdBslmaTestDriver : public TestDriver<
                            TYPE,
                            bsltf::StdAllocatorAdaptor<bsl::allocator<TYPE> > >
{
};

                               // --------------
                               // TEST APPARATUS
                               // --------------

template <class TYPE, class ALLOC>
int TestDriver<TYPE,ALLOC>::getValues(const TYPE **valuesPtr)
{
    bslma::DefaultAllocatorGuard
                                guard(&bslma::NewDeleteAllocator::singleton());

    static TYPE values[5]; // avoid DEFAULT_VALUE and UNINITIALIZED_VALUE
    values[0] = TYPE(VA);
    values[1] = TYPE(VB);
    values[2] = TYPE(VC);
    values[3] = TYPE(VD);
    values[4] = TYPE(VE);

    const int NUM_VALUES = 5;

    *valuesPtr = values;
    return NUM_VALUES;
}

template <class TYPE, class ALLOC>
int TestDriver<TYPE,ALLOC>::ggg(Obj        *object,
                                const char *spec,
                                bool        verboseFlag)
{
    BSLS_ASSERT_OPT(object);
    BSLS_ASSERT_OPT(spec);

    // In order to execute the 'spec' program, we are going to need to track a
    // little state.  Based on the known size of a page (counted in objects) we
    // can separately track the available capacity to the front and back of the
    // currently inserting blocks.  The numbers must be tracked through every
    // insertion operation.  Several characters when parsed indicate the start
    // of a command sequence, which should be parsed in full before evaluating
    // as a single command:
    //..
    //  '<'   : Record a command sequence until a '|' character
    //  digit : Repeat the next (simple) command digit times.
    //..
    // Note that a 'digit' can occur in a insert-to-boundary sequence.

    enum { SUCCESS = -1 };

    int  frontCapacity = (PageLength<TYPE>::k_VALUE - 1) / 2;
    int  backCapacity  = PageLength<TYPE>::k_VALUE - 1 - frontCapacity;
    int  repeatCount   = 1;
    int  tailPadding   = 0;
    bool applyToBack   = true;  // 'true' > 'push back', 'false' > 'push_front'
    int  fillPage      = -1;    // index of 'start range' command

    int VALUE = 0;  // 0 when in "immediate mode"; otherwise, the identifier of
                    // the object to fill-to-page-boundary with.

    // The parser below would be much simpler with C++11 lambda expressions.

    for (int i = 0; spec[i]; ++i) {
        if (' ' == spec[i]) {
            continue;   // whitespace is not salient in any parse mode
        }

        if (-1 == fillPage) {  // immediate mode, not forming a fill-page range
            if ('A' <= spec[i] && spec[i] <= 'Y') {
                for (int j = 0; j != repeatCount; ++j) {
                    primaryManipulatorBack(object, spec[i]);
                    if (backCapacity) {
                        --backCapacity;
                    }
                    else {
                        backCapacity = PageLength<TTA>::k_VALUE - 1;
                    }
                }
            }
            else if ('a' <= spec[i] && spec[i] <= 'y') {
                for (int j = 0; j != repeatCount; ++j) {
                    primaryManipulatorFront(object, toupper(spec[i]));
                    if (frontCapacity) {
                        --frontCapacity;
                    }
                    else {
                        frontCapacity = PageLength<TTA>::k_VALUE - 1;
                    }
                }
            }
            else if (1 != repeatCount) {
                if (verboseFlag) {
                    printf("Error, bad character ('%c') in spec \"%s\" at"
                           " position %d: cannot follow a repeat-count.\n",
                           spec[i], spec, i);
                }
                return i;  // Discontinue processing this spec.       // RETURN
            }
            else if ('~' == spec[i]) {
                object->clear();
                backCapacity  = PageLength<TTA>::k_VALUE / 2;
                frontCapacity = (PageLength<TYPE>::k_VALUE + 1) / 2 -1;;
            }
            else if ('<' == spec[i]) {
                fillPage = i;    // Enter fill-page mode
                // Note we cannot diagnose an at-boundary error until we know
                // whether we push at the front or the back, so wait until the
                // range direction is specified before (potentially) reporting
                // an error.
            }
            else if ('2' <= spec[i] && spec[i] <= '9') {
                repeatCount = spec[i] - '0';
            }
            else if ('0' == spec[i]) {
                repeatCount = 10;
            }
            // Any other character is invalid, although we can give stronger
            // diagnostics in some cases.
            else if ('|' == spec[i]) {
                if (verboseFlag) {
                    printf("Error, bad character ('%c') in spec \"%s\" at"
                           " position %d: terminator for an unopened range.\n",
                           spec[i], spec, i);
                }
                return i;  // Discontinue processing this spec.       // RETURN
            }
            else if ('.' == spec[i]) {
                if (verboseFlag) {
                    printf("Error, bad character ('%c') in spec \"%s\" at"
                           " position %d: padding allowed only in a range.\n",
                           spec[i], spec, i);
                }
                return i;  // Discontinue processing this spec.       // RETURN
            }
            else {
                if (verboseFlag) {
                    printf("Error, bad character ('%c') in spec \"%s\" at"
                           " position %d.\n", spec[i], spec, i);
                }
                return i;  // Discontinue processing this spec.       // RETURN
            }
        }
        else {  // forming a fill-page range
            if ('|' == spec[i]) { // execute range-fill
                if (0 == VALUE) {
                    if (verboseFlag) {
                        printf("Error, invalid fill-range in spec \"%s\""
                               " starting at position %d: no fill char.\n",
                               spec, fillPage);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }

                int *capacity = applyToBack ? &backCapacity : &frontCapacity;
                if (*capacity <= tailPadding) {
                    if (verboseFlag) {
                        printf("Error, invalid fill-range in spec \"%s\""
                               " starting at position %d: no room for padding."
                               "\n", spec, fillPage);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }

//T_; P(*capacity);
                while (*capacity != tailPadding) {
                    if (applyToBack) {
                        primaryManipulatorBack(object, VALUE);
                    }
                    else {
                        primaryManipulatorFront(object, VALUE);
                    }
                    --*capacity;
                }

                // Reset state after fill
                fillPage    = -1;
                repeatCount = 1;
                tailPadding = 0;
                VALUE       = 0;
            }
            else if ('A' <= spec[i] && spec[i] <= 'E') {
                if (0 != VALUE) {
                    if (verboseFlag) {
                        printf("Error, bad character ('%c') in spec \"%s\" at"
                               " position %d: only one fill char allowed.\n",
                               spec[i], spec, i);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }

                applyToBack = true;
                VALUE = spec[i];
            }
            else if ('a' <= spec[i] && spec[i] <= 'e') {
                if (0 != VALUE) {
                    if (verboseFlag) {
                        printf("Error, bad character ('%c') in spec \"%s\" at"
                               " position %d: only one fill char allowed.\n",
                               spec[i], spec, i);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }

                applyToBack = false;
                VALUE = toupper(spec[i]);
            }
            else if ('.' == spec[i]) { // tail padding
                if (0 == VALUE) {
                    if (verboseFlag) {
                        printf("Error, bad character ('%c') in spec \"%s\" at"
                               " position %d: padding preceding fill char.\n",
                               spec[i], spec, i);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }
                tailPadding += repeatCount;
            }
// TBD
#if 0  // Future extension      - allow a repeater for tail-padding.
       // Implementation detail - introduces another bool state to track.
       //     If we do not track the state-flag, then a simple trailing digit
       //     would indicate padding, without an optional '.' suffix.  Not sure
       //     if we should allow this, or pick it up as an error, but currently
       //     no expected use cases - will review again after updating the
       //     'insert' test case.
            else if ('2' <= spec[i] && spec[i] <= '9') {
                if (0 != tailPadding) {
                    if (verboseFlag) {
                        printf("Error, bad character ('%c') in spec \"%s\" at"
                               " position %d: digit cannot follow padding.\n",
                               spec[i], spec, i);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }
                if (0 == VALUE) {
                    if (verboseFlag) {
                        printf("Error, bad character ('%c') in spec \"%s\" at"
                               " position %d: no repeated objects in range.\n",
                               spec[i], spec, i);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }
                repeatCount = spec[i] - '0';
            }
            else if ('0' == spec[i]) {
                if (0 != tailPadding) {
                    if (verboseFlag) {
                        printf("Error, bad character ('%c') in spec \"%s\" at"
                               " position %d: digit cannot follow padding.\n",
                               spec[i], spec, i);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }
                if (0 == VALUE) {
                    if (verboseFlag) {
                        printf("Error, bad character ('%c') in spec \"%s\" at"
                               " position %d: no repeated objects in range.\n",
                               spec[i], spec, i);
                    }
                    return i;  // Discontinue processing this spec.   // RETURN
                }
                repeatCount = 10;
            }
#endif
            // We could provide nicer errors for '1', '<', and '~'
            else {
                if (verboseFlag) {
                    printf("Error, bad character ('%c') in spec \"%s\" at"
                           " position %d.\n", spec[i], spec, i);
                }
                return i;  // Discontinue processing this spec.       // RETURN
            }
        }
    }

    if (-1 != fillPage) {  // We have a range command that is not terminated.
        if (verboseFlag) {
            printf("Error, bad character ('<') in spec \"%s\" at"
                   " position %d: nonterminated fill-range.\n",
                   spec, fillPage);
        }
        return fillPage;  // Discontinue processing this spec.        // RETURN
    }

    resetMovedInto(object);

    return SUCCESS;
}

template <class TYPE, class ALLOC>
bsl::deque<TYPE,ALLOC>& TestDriver<TYPE,ALLOC>::gg(Obj        *object,
                                                   const char *spec)
{
    ASSERT(ggg(object, spec) < 0);
    return *object;
}

template <class TYPE, class ALLOC>
bsl::vector<TYPE> TestDriver<TYPE,ALLOC>::gV(const char *spec)
{
    const TYPE *VALUES;
    getValues(&VALUES);
    bsl::vector<TYPE> result;
    for (int i = 0; spec[i]; ++i) {
        if ('A' <= spec[i] && spec[i] <= 'E') {
            result.push_back(VALUES[spec[i] - 'A']);
        }
        else if ('<' == spec[i]) {
            result.pop_back();
        }
        else if ('~' == spec[i]) {
            result.clear();
        }
        else {
            ASSERT(0);
        }
   }
   return result;
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::setInternalState(Obj *object, int n)
{
    ASSERT(object);
    ASSERT(0 == object->size());
    const TYPE value = TYPE();
    if (n > 0) {
        for (; n > 0; --n) {
            object->push_back(value);
            object->pop_front();
        }
    }
    else {
        for (; n < 0; ++n) {
            object->push_front(value);
            object->pop_back();
        }
    }
    ASSERT(0 == object->size());
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::stretch(Obj         *object,
                                     std::size_t  size,
                                     int          identifier)
{
    ASSERT(object);

    const std::size_t initialSize = object->size();

    bslma::TestAllocator scratch;

    for (std::size_t i = 0; i < size; ++i) {
// TBD perhaps add an argument to stretch: front only, back only, both ends
//        if (0 == i % 2) {
            primaryManipulatorBack(object, identifier);
//        }
//        else {
//            primaryManipulatorFront(object, identifier, &scratch);
//        }
    }
    ASSERT(object->size() == initialSize + size);
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::stretchRemoveAll(Obj         *object,
                                              std::size_t  size,
                                              int          identifier)
{
    ASSERT(object);
    stretch(object, size, identifier);
    object->clear();
    ASSERT(0 == object->size());
}

template <class TYPE, class ALLOC>
int TestDriver<TYPE, ALLOC>::findRoomierEnd(Obj *container)
{
    typedef typename Obj::size_type size_type;

    const size_type cap = container->capacity();

    if (container->empty()) {
        // The container will always have room for at least one element without
        // doing any block allocations.

        container->push_back(TYPE(VA));
        const bool backGrew = container->capacity() > cap;
        container->pop_back();

        container->push_front(TYPE(VA));
        const bool frontGrew = container->capacity() > cap;
        container->pop_front();

        ASSERT(backGrew || frontGrew);

        if (backGrew) {
            if (frontGrew) {
                return 0;                                             // RETURN
            }
            return -1;                                                // RETURN
        }
        return 1;                                                     // RETURN
    }

    TYPE val = container->back();
    container->pop_back();
    const bool backShrunk = container->capacity() < cap;
    container->push_back(val);

    val = container->front();
    container->pop_front();
    const bool frontShrunk = container->capacity() < cap;
    container->push_front(val);

    ASSERT(backShrunk || frontShrunk);

    if (backShrunk) {
        if (frontShrunk) {
            return 0;                                                 // RETURN
        }
        return -1;                                                    // RETURN
    }
    return 1;
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::testCase11()
{
    // ------------------------------------------------------------------------
    // TEST ALLOCATOR-RELATED CONCERNS
    //
    // Concerns:
    //: 1 That the deque class has the 'bslma::UsesBslmaAllocator' trait.
    //:
    //: 2 That the allocator is passed through to contained elements.
    //
    // Plan:
    //   We first verify that the 'bsl::deque' class has the trait, then verify
    //   that the allocator is passed to contained elements.
    //
    // Testing:
    //   bslma::UsesBslmaAllocator
    // ------------------------------------------------------------------------

    if (verbose) printf("\nALLOCATOR TEST"
                        "\n==============\n");

    bslma::TestAllocator oa("object", veryVeryVeryVerbose);

    const TYPE         *values     = 0;
    const TYPE *const&  VALUES     = values;
    const int           NUM_VALUES = getValues(&values);
    (void)NUM_VALUES;

    if (verbose) printf("\nTesting 'bslma::UsesBslmaAllocator'.\n");

    ASSERT((bslma::UsesBslmaAllocator<Obj>::value));

    if (verbose) printf("\nTesting that an empty deque does allocate.\n");
    {
        const Obj X(&oa);
        ASSERT(0 < oa.numBytesInUse());
    }

    if (verbose) printf("\nTesting passing allocator through to elements.\n");

    ASSERT((bslma::UsesBslmaAllocator<TYPE>::value));
    {
        Obj mX(1, VALUES[0], &oa);  const Obj& X = mX;
        ASSERT(&oa == X[0].allocator());
    }
    {
        Obj mX(&oa);  const Obj& X = mX;
        mX.push_back(VALUES[0]);
        ASSERT(&oa == X[0].allocator());
    }

    ASSERT(0 == oa.numBytesInUse());
}

template <class TYPE, class ALLOC>
template <bool PROPAGATE_ON_CONTAINER_COPY_ASSIGNMENT_FLAG,
          bool OTHER_FLAGS>
void TestDriver<TYPE, ALLOC>::
                    testCase9_propagate_on_container_copy_assignment_dispatch()
{
    // Set the three properties of 'bsltf::StdStatefulAllocator' that are not
    // under test in this test case to 'false'.

    typedef bsltf::StdStatefulAllocator<
                                   TYPE,
                                   OTHER_FLAGS,
                                   PROPAGATE_ON_CONTAINER_COPY_ASSIGNMENT_FLAG,
                                   OTHER_FLAGS,
                                   OTHER_FLAGS> StdAlloc;

    typedef bsl::deque<TYPE, StdAlloc>          Obj;

    const bool PROPAGATE = PROPAGATE_ON_CONTAINER_COPY_ASSIGNMENT_FLAG;

    static const char *SPECS[] = {
        "",
        "A",
        "BC",
        "CDE",
    };
    const int NUM_SPECS = static_cast<int>(sizeof SPECS / sizeof *SPECS);

    bslma::TestAllocator da("default", veryVeryVeryVerbose);
    bslma::DefaultAllocatorGuard dag(&da);

    // Create control and source objects.
    for (int ti = 0; ti < NUM_SPECS; ++ti) {
        const char *const ISPEC   = SPECS[ti];

        TestValues IVALUES(ISPEC);

        bslma::TestAllocator oas("source", veryVeryVeryVerbose);
        bslma::TestAllocator oat("target", veryVeryVeryVerbose);

        StdAlloc mas(&oas);
        StdAlloc mat(&oat);

        StdAlloc scratch(&da);

        const Obj W(IVALUES.begin(), IVALUES.end(), scratch);  // control

        // Create target object.
        for (int tj = 0; tj < NUM_SPECS; ++tj) {
            const char *const JSPEC   = SPECS[tj];

            TestValues JVALUES(JSPEC);

            {
                IVALUES.resetIterators();

                Obj mY(IVALUES.begin(), IVALUES.end(), mas);
                const Obj& Y = mY;

                if (veryVerbose) { T_ P_(ISPEC) P_(Y) P(W) }

                Obj mX(JVALUES.begin(), JVALUES.end(), mat);
                const Obj& X = mX;

                bslma::TestAllocatorMonitor oasm(&oas);
                bslma::TestAllocatorMonitor oatm(&oat);

                Obj *mR = &(mX = Y);

                ASSERTV(ISPEC, JSPEC,  W,   X,  W == X);
                ASSERTV(ISPEC, JSPEC,  W,   Y,  W == Y);
                ASSERTV(ISPEC, JSPEC, mR, &mX, mR == &mX);

                ASSERTV(ISPEC, JSPEC, PROPAGATE,
                       !PROPAGATE == (mat == X.get_allocator()));
                ASSERTV(ISPEC, JSPEC, PROPAGATE,
                        PROPAGATE == (mas == X.get_allocator()));

                ASSERTV(ISPEC, JSPEC, mas == Y.get_allocator());

                if (PROPAGATE) {
                    ASSERTV(ISPEC, JSPEC, 0 == oat.numBlocksInUse());
                }
                else {
                    ASSERTV(ISPEC, JSPEC, oasm.isInUseSame());
                }
            }
            ASSERTV(ISPEC, 0 == oas.numBlocksInUse());
            ASSERTV(ISPEC, 0 == oat.numBlocksInUse());
        }
    }
    ASSERTV(0 == da.numBlocksInUse());
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE, ALLOC>::
                             testCase9_propagate_on_container_copy_assignment()
{
    // ------------------------------------------------------------------------
    // COPY-ASSIGNMENT OPERATOR: ALLOCATOR PROPAGATION
    //
    // Concerns:
    //: 1 If the 'propagate_on_container_copy_assignment' trait is 'false', the
    //:   allocator used by the target object remains unchanged (i.e., the
    //:   source object's allocator is *not* propagated).
    //:
    //: 2 If the 'propagate_on_container_copy_assignment' trait is 'true', the
    //:   allocator used by the target object is updated to be a copy of that
    //:   used by the source object (i.e., the source object's allocator *is*
    //:   propagated).
    //:
    //: 3 The allocator used by the source object remains unchanged whether or
    //;   not it is propagated to the target object.
    //:
    //: 4 If the allocator is propagated from the source object to the target
    //:   object, all memory allocated from the target object's original
    //:   allocator is released.
    //:
    //: 5 The effect of the 'propagate_on_container_copy_assignment' trait is
    //:   independent of the other three allocator propagation traits.
    //
    // Plan:
    //: 1 Specify a set S of object values with varied differences, ordered by
    //:   increasing length, to be used in the following tests.
    //:
    //: 2 Create two 'bsltf::StdStatefulAllocator' objects with their
    //:   'propagate_on_container_copy_assignment' property configured to
    //:   'false'.  In two successive iterations of P-3, first configure the
    //:   three properties not under test to be 'false', then configure them
    //:   all to be 'true'.
    //:
    //: 3 For each value '(x, y)' in the cross product S x S:  (C-1)
    //:
    //:   1 Initialize an object 'X' from 'x' using one of the allocators from
    //:     P-2.
    //:
    //:   2 Initialize two objects from 'y', a control object 'W' using a
    //:     scratch allocator and an object 'Y' using the other allocator from
    //:     P-2.
    //:
    //:   3 Copy-assign 'Y' to 'X' and use 'operator==' to verify that both
    //:     'X' and 'Y' subsequently have the same value as 'W'.
    //:
    //:   4 Use the 'get_allocator' method to verify that the allocator of 'Y'
    //:     is *not* propagated to 'X' and that the allocator used by 'Y'
    //:     remains unchanged.  (C-1)
    //:
    //: 4 Repeat P-2..3 except that this time configure the allocator property
    //:   under test to 'true' and verify that the allocator of 'Y' *is*
    //:   propagated to 'X'.  Also verify that all memory is released to the
    //:   allocator that was in use by 'X' prior to the assignment.  (C-2..5)
    //
    // Testing:
    //   propagate_on_container_copy_assignment
    // ------------------------------------------------------------------------

    if (verbose) printf("\nCOPY-ASSIGNMENT OPERATOR: ALLOCATOR PROPAGATION"
                        "\n===============================================\n");

    if (verbose)
        printf("\n'propagate_on_container_copy_assignment::value == false'\n");

    testCase9_propagate_on_container_copy_assignment_dispatch<false, false>();
    testCase9_propagate_on_container_copy_assignment_dispatch<false, true>();

    if (verbose)
        printf("\n'propagate_on_container_copy_assignment::value == true'\n");

    testCase9_propagate_on_container_copy_assignment_dispatch<true, false>();
    testCase9_propagate_on_container_copy_assignment_dispatch<true, true>();
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::testCase9()
{
    // ------------------------------------------------------------------------
    // TESTING COPY-ASSIGNMENT OPERATOR
    //
    // Concerns:
    //: 1 The value represented by any object can be assigned to any other
    //:   object regardless of how either value is represented internally.
    //:
    //: 2 The 'rhs' value must not be affected by the operation.
    //:
    //: 3 'rhs' going out of scope has no effect on the value of 'lhs' after
    //:   the assignment.
    //:
    //: 4 Aliasing (x = x): The assignment operator must always work -- even
    //:   when the lhs and rhs are identically the same object.
    //:
    //: 5 The assignment operator must be neutral with respect to memory
    //:   allocation exceptions.
    //:
    //: 6 The copy constructor's internal functionality varies according to
    //:   which bitwise copy/move trait is applied.
    //
    // Plan:
    //   Specify a set S of unique object values with substantial and
    //   varied differences, ordered by increasing length.  For each value
    //   in S, construct an object x along with a sequence of similarly
    //   constructed duplicates x1, x2, ..., xN.  Attempt to affect every
    //   aspect of white-box state by altering each xi in a unique way.
    //   Let the union of all such objects be the set T.
    //
    //   To address concerns 1, 2, and 5, construct tests u = v for all
    //   (u, v) in T X T.  Using canonical controls UU and VV, assert
    //   before the assignment that UU == u, VV == v, and v == u if and only if
    //   VV == UU.  After the assignment, assert that VV == u, VV == v,
    //   and, for grins, that v == u.  Let v go out of scope and confirm
    //   that VV == u.  All of these tests are performed within the 'bslma'
    //   exception testing apparatus.  Since the execution time is lengthy
    //   with exceptions, every permutation is not performed when
    //   exceptions are tested.  Every permutation is also tested
    //   separately without exceptions.
    //
    //   As a separate exercise, we address 4 and 5 by constructing tests
    //   y = y for all y in T.  Using a canonical control X, we will verify
    //   that X == y before and after the assignment, again within
    //   the bslma exception testing apparatus.
    //
    //   To address concern 6, all these tests are performed on user
    //   defined types:
    //          With allocator, copyable
    //          With allocator, moveable
    //          With allocator, not moveable
    //
    // Testing:
    //   deque& operator=(const deque& rhs);
    // ------------------------------------------------------------------------

    bslma::TestAllocator          oa("object", veryVeryVeryVerbose);
    ALLOC                         xoa(&oa);
    bslma::TestAllocator          da("default", veryVeryVeryVerbose);
    bslma::DefaultAllocatorGuard  dag(&da);

    const TYPE                   *values = 0;
    const int                     NUM_VALUES = getValues(&values);
    (void) NUM_VALUES;

    // ------------------------------------------------------------------------

    if (verbose) printf("\nAssign cross product of values "
                        "with varied representations.\n"
                        "Without Exceptions\n");
    {
        static const char *SPECS[] = {
            "",
            "A",
            "BC",
            "CDE",
            "DEAB",
            "CBAEDCBA",
            "EDCBAEDCB",
            0 // null string required as last element
        };

        static const int EXTEND[] = {
            0, 1, 2, 3, 4, 5, 7, 8, 9
        };
        enum { NUM_EXTEND = sizeof EXTEND / sizeof *EXTEND };

        {
            int uOldLen = -1;
            for (int ui = 0; SPECS[ui]; ++ui) {
                const char *const U_SPEC = SPECS[ui];
                const int         uLen   = (int) strlen(U_SPEC);

                if (verbose) {
                    printf("\tFor lhs objects of length %d:\t", uLen);
                    P(U_SPEC);
                }

                LOOP_ASSERT(U_SPEC, uOldLen < uLen);
                uOldLen = uLen;

                Obj mUU;  const Obj& UU = gg(&mUU, U_SPEC);  // control
                LOOP_ASSERT(ui, uLen == (int)UU.size());     // same lengths

                for (int vi = 0; SPECS[vi]; ++vi) {
                    const char *const V_SPEC = SPECS[vi];
                    const int         vLen   = (int) strlen(V_SPEC);

                    if (veryVerbose) {
                        printf("\t\tFor rhs objects of length %d:\t", vLen);
                        P(V_SPEC);
                    }

                    Obj mVV;  const Obj& VV = gg(&mVV, V_SPEC);  // control

                    const bool Z = ui == vi;  // flag indicating same values

                    for (int uj = START_POS; uj <= FINISH_POS;
                                                             uj += INCREMENT) {
                        const int U_N = uj;
                        for (int vj = START_POS; vj <= FINISH_POS;
                                                             vj += INCREMENT) {
                            const int V_N = vj;

                            Obj mU(xoa);
                            setInternalState(&mU, U_N);
                            const Obj& U = mU;
                            gg(&mU, U_SPEC);
                            {
                                Obj mV(xoa);
                                setInternalState(&mV, V_N);
                                const Obj& V = mV;
                                gg(&mV, V_SPEC);
                    // v--------
                    static int firstFew = 2 * NUM_EXTEND * NUM_EXTEND;
                    if (veryVeryVerbose || (veryVerbose && firstFew > 0)) {
                        printf("\t| "); P_(U_N); P_(V_N); P_(U); P(V);
                        --firstFew;
                    }
                    if (!veryVeryVerbose && veryVerbose && 0 == firstFew) {
                        printf("\t| ... (omitted from now on\n");
                        --firstFew;
                    }

                    LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, UU == U);
                    LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, VV == V);
                    LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, Z==(V==U));

                    const int NUM_CTOR = numCopyCtorCalls;
                    const int NUM_DTOR = numDestructorCalls;
                    const size_t OLD_LENGTH = U.size();

                    mU = V;  // test assignment here

                    ASSERT((numCopyCtorCalls - NUM_CTOR) <= (int)V.size());
                    ASSERT((numDestructorCalls - NUM_DTOR) <=
                                                 (int)(V.size() + OLD_LENGTH));

                    LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, VV == U);
                    LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, VV == V);
                    LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N,  V == U);
                    // ---------v
                            }
                            //  'mV' (and therefore 'V') now out of scope
                            LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, VV == U);
                        }
                    }
                }
            }
        }
    }

    if (verbose) printf("\nAssign cross product of values "
                        "with varied representations.\n"
                        "With Exceptions\n");
    {
        static const char *SPECS[] = {  // len: 0-2, 4, 9,
            "",        "A",    "BC",     "DEAB",    "EDCBAEDCB",
            0
        };  // null string required as last element

        static const int EXTEND[] = {
            0, 1, 3, 5
        };
        enum { NUM_EXTEND = sizeof EXTEND / sizeof *EXTEND };

        int iterationModulus = 1;
        int iteration = 0;
        {
            int uOldLen = -1;
            for (int ui = 0; SPECS[ui]; ++ui) {
                const char *const U_SPEC = SPECS[ui];
                const int         uLen   = (int) strlen(U_SPEC);

                if (verbose) {
                    printf("\tFor lhs objects of length %d:\t", uLen);
                    P(U_SPEC);
                }

                LOOP_ASSERT(U_SPEC, uOldLen < uLen);
                uOldLen = uLen;

                Obj mUU;  const Obj& UU = gg(&mUU, U_SPEC);  // control
                LOOP_ASSERT(ui, uLen == (int)UU.size());     // same lengths

                // int vOldLen = -1;
                for (int vi = 0; SPECS[vi]; ++vi) {
                    const char *const V_SPEC = SPECS[vi];
                    const int         vLen   = (int) strlen(V_SPEC);

                    if (veryVerbose) {
                        printf("\t\tFor rhs objects of length %d:\t", vLen);
                        P(V_SPEC);
                    }

                    Obj mVV;  const Obj& VV = gg(&mVV, V_SPEC);  // control

                    for (int uj = START_POS; uj <= FINISH_POS;
                                                             uj += INCREMENT) {
                        const int U_N = uj;
                        for (int vj = START_POS; vj <= FINISH_POS;
                                                             vj += INCREMENT) {
                            const int V_N = vj;

                            if (0 == iteration % iterationModulus) {
                                BSLMA_TESTALLOCATOR_EXCEPTION_TEST_BEGIN(oa) {
                    //--------------^
                    const int AL = (int) oa.allocationLimit();
                    oa.setAllocationLimit(-1);
                    Obj mU(xoa);
                    setInternalState(&mU, U_N);
                    const Obj& U = mU;
                    gg(&mU, U_SPEC);
                    {
                        Obj mV(xoa);
                        setInternalState(&mV, V_N);
                        const Obj& V = mV;
                        gg(&mV, V_SPEC);

                        static int firstFew = 2 * NUM_EXTEND * NUM_EXTEND;
                        if (veryVeryVerbose || (veryVerbose && firstFew > 0)) {
                            printf("\t| "); P_(U_N); P_(V_N); P_(U); P(V);
                            --firstFew;
                        }
                        if (!veryVeryVerbose && veryVerbose && 0 == firstFew) {
                            printf("\t| ... (omitted from now on\n");
                            --firstFew;
                        }

                        oa.setAllocationLimit(AL);
                        {
                            mU = V;  // test assignment here
                        }

                        LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, VV == U);
                        LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, VV == V);
                        LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N,  V == U);
                    }
                    // 'mV' (and therefore 'V') now out of scope
                    LOOP4_ASSERT(U_SPEC, U_N, V_SPEC, V_N, VV == U);
                    //--------------v
                                } BSLMA_TESTALLOCATOR_EXCEPTION_TEST_END
                            }
                            ++iteration;
                        }
                    }
                }
            }
        }
    }

    if (verbose) printf("\nTesting self assignment (Aliasing).");
    {
        static const char *SPECS[] = {
            "",      "A",      "BC",     "CDE",    "DEAB",   "EABCD",
            "ABCDEAB",         "ABCDEABC",         "ABCDEABCD",
            "ABCDEABCDEABCDE", "ABCDEABCDEABCDEA", "ABCDEABCDEABCDEAB",
            0 // null string required as last element
        };

        int oldLen = -1;
        for (int ti = 0; SPECS[ti]; ++ti) {
            const char *const SPEC = SPECS[ti];
            const int curLen = (int) strlen(SPEC);

            if (verbose) {
                printf("\tFor an object of length %d:\t", curLen);
                P(SPEC);
            }
            LOOP_ASSERT(SPEC, oldLen < curLen);  // strictly increasing
            oldLen = curLen;

            // control
            Obj mX;  const Obj X = gg(&mX, SPEC);
            LOOP_ASSERT(ti, curLen == (int)X.size());  // same lengths

            for (int tj = START_POS; tj <= FINISH_POS; tj += INCREMENT) {
                BSLMA_TESTALLOCATOR_EXCEPTION_TEST_BEGIN(oa) {
                    const int AL = (int) oa.allocationLimit();
                    oa.setAllocationLimit(-1);

                    const int N = tj;
                    Obj mY(xoa);
                    setInternalState(&mY, N);
                    const Obj& Y = mY;
                    gg(&mY, SPEC);

                    if (veryVerbose) { T_; T_; P_(N); P(Y); }

                    LOOP2_ASSERT(SPEC, N, Y == Y);
                    LOOP2_ASSERT(SPEC, N, X == Y);

                    oa.setAllocationLimit(AL);

                    {
                        ExceptionProctor<Obj, ALLOC> proctor(&mY, Y, L_);
                        mY = Y;  // test assignment here
                    }

                    LOOP2_ASSERT(SPEC, N, Y == Y);
                    LOOP2_ASSERT(SPEC, N, X == Y);
                } BSLMA_TESTALLOCATOR_EXCEPTION_TEST_END
            }
        }
    }
}

template <class TYPE, class ALLOC>
template <bool SELECT_ON_CONTAINER_COPY_CONSTRUCTION_FLAG,
          bool OTHER_FLAGS>
void TestDriver<TYPE, ALLOC>::
                     testCase7_select_on_container_copy_construction_dispatch()
{
    const int TYPE_ALLOC = bslma::UsesBslmaAllocator<TYPE>::value;

    // Set the three properties of 'bsltf::StdStatefulAllocator' that are not
    // under test in this test case to 'false'.

    typedef bsltf::StdStatefulAllocator<
                                    TYPE,
                                    SELECT_ON_CONTAINER_COPY_CONSTRUCTION_FLAG,
                                    OTHER_FLAGS,
                                    OTHER_FLAGS,
                                    OTHER_FLAGS> StdAlloc;

    typedef bsl::deque<TYPE, StdAlloc>           Obj;

    const bool PROPAGATE = SELECT_ON_CONTAINER_COPY_CONSTRUCTION_FLAG;

    static const char *SPECS[] = {
        "",
        "A",
        "BC",
        "CDE",
    };
    const int NUM_SPECS = static_cast<int>(sizeof SPECS / sizeof *SPECS);

    for (int ti = 0; ti < NUM_SPECS; ++ti) {
        const char *const SPEC   = SPECS[ti];
        const size_t      LENGTH = strlen(SPEC);

        TestValues VALUES(SPEC);

        bslma::TestAllocator da("default", veryVeryVeryVerbose);
        bslma::TestAllocator oa("object",  veryVeryVeryVerbose);

        bslma::DefaultAllocatorGuard dag(&da);

        StdAlloc ma(&oa);

        {
            const Obj W(VALUES.begin(), VALUES.end(), ma);  // control

            ASSERTV(ti, LENGTH == W.size());  // same lengths
            if (veryVerbose) { printf("\tControl Obj: "); P(W); }

            VALUES.resetIterators();

            Obj mX(VALUES.begin(), VALUES.end(), ma);
            const Obj& X = mX;

            if (veryVerbose) { printf("\t\tDynamic Obj: "); P(X); }

            bslma::TestAllocatorMonitor dam(&da);
            bslma::TestAllocatorMonitor oam(&oa);

            const Obj Y(X);

            ASSERTV(SPEC, W == Y);
            ASSERTV(SPEC, W == X);
            ASSERTV(SPEC, PROPAGATE, PROPAGATE == (ma == Y.get_allocator()));
            ASSERTV(SPEC, PROPAGATE,               ma == X.get_allocator());

            if (PROPAGATE) {
                ASSERTV(SPEC, 0 != TYPE_ALLOC || dam.isInUseSame());
                ASSERTV(SPEC, 0 ==     LENGTH || oam.isInUseUp());
            }
            else {
                ASSERTV(SPEC, 0 ==     LENGTH || dam.isInUseUp());
                ASSERTV(SPEC, oam.isTotalSame());
            }
        }
        ASSERTV(SPEC, 0 == da.numBlocksInUse());
        ASSERTV(SPEC, 0 == oa.numBlocksInUse());
    }
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE, ALLOC>::testCase7_select_on_container_copy_construction()
{
    // ------------------------------------------------------------------------
    // COPY CONSTRUCTOR: ALLOCATOR PROPAGATION
    //
    // Concerns:
    //: 1 The allocator of a source object using a standard allocator is
    //:   propagated to the newly constructed object according to the
    //:   'select_on_container_copy_construction' method of the allocator.
    //:
    //: 2 In the absence of a 'select_on_container_copy_construction' method,
    //:   the allocator of a source object using a standard allocator is always
    //:   propagated to the newly constructed object (C++03 semantics).
    //:
    //: 3 The effect of the 'select_on_container_copy_construction' trait is
    //:   independent of the other three allocator propagation traits.
    //
    // Plan:
    //: 1 Specify a set S of object values with varied differences, ordered by
    //:   increasing length, to be used in the following tests.
    //:
    //: 2 Create a 'bsltf::StdStatefulAllocator' with its
    //:   'select_on_container_copy_construction' property configured to
    //:   'false'.  In two successive iterations of P-3..5, first configure the
    //:   three properties not under test to be 'false', then configure them
    //:   all to be 'true'.
    //:
    //: 3 For each value in S, initialize objects 'W' (a control) and 'X' using
    //:   the allocator from P-2.
    //:
    //: 4 Copy construct 'Y' from 'X' and use 'operator==' to verify that both
    //:   'X' and 'Y' subsequently have the same value as 'W'.
    //:
    //: 5 Use the 'get_allocator' method to verify that the allocator of 'X'
    //:   is *not* propagated to 'Y'.
    //:
    //: 6 Repeat P-2..5 except that this time configure the allocator property
    //:   under test to 'true' and verify that the allocator of 'X' *is*
    //:   propagated to 'Y'.  (C-1)
    //:
    //: 7 Repeat P-2..5 except that this time use a 'StatefulStlAllocator',
    //:   which does not define a 'select_on_container_copy_construction'
    //:   method, and verify that the allocator of 'X' is *always* propagated
    //:   to 'Y'.  (C-2..3)
    //
    // Testing:
    //   select_on_container_copy_construction
    // ------------------------------------------------------------------------

    if (verbose) printf("\n'select_on_container_copy_construction' "
                        "propagates *default* allocator.\n");

    testCase7_select_on_container_copy_construction_dispatch<false, false>();
    testCase7_select_on_container_copy_construction_dispatch<false, true>();

    if (verbose) printf("\n'select_on_container_copy_construction' "
                        "propagates allocator of source object.\n");

    testCase7_select_on_container_copy_construction_dispatch<true, false>();
    testCase7_select_on_container_copy_construction_dispatch<true, true>();

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (verbose) printf("\nVerify C++03 semantics (allocator has no "
                        "'select_on_container_copy_construction' method).\n");

    typedef StatefulStlAllocator<TYPE>  Allocator;
    typedef bsl::deque<TYPE, Allocator> Obj;

    {
        static const char *SPECS[] = {
            "",
            "A",
            "BC",
            "CDE",
        };
        const int NUM_SPECS = static_cast<int>(sizeof SPECS / sizeof *SPECS);

        for (int ti = 0; ti < NUM_SPECS; ++ti) {
            const char *const SPEC   = SPECS[ti];
            const size_t      LENGTH = strlen(SPEC);
            TestValues VALUES(SPEC);

            const int ALLOC_ID = ti + 73;

            Allocator a;  a.setId(ALLOC_ID);

            const Obj W(VALUES.begin(), VALUES.end(), a);  // control

            ASSERTV(ti, LENGTH == W.size());  // same lengths
            if (veryVerbose) { printf("\tControl Obj: "); P(W); }

            VALUES.resetIterators();

            Obj mX(VALUES.begin(), VALUES.end(), a);  const Obj& X = mX;

            if (veryVerbose) { printf("\t\tDynamic Obj: "); P(X); }

            const Obj Y(X);

            ASSERTV(SPEC,        W == Y);
            ASSERTV(SPEC,        W == X);
            ASSERTV(SPEC, ALLOC_ID == Y.get_allocator().id());
        }
    }
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::testCase7()
{
    // ------------------------------------------------------------------------
    // TESTING COPY CONSTRUCTOR
    //
    // Concerns:
    //: 1 The new object's value is the same as that of the original object
    //:   (relying on the equality operator) and created with the correct
    //:   capacity.
    //:
    //: 2 All internal representations of a given value can be used to create a
    //:   new object of equivalent value.
    //:
    //: 3 The value of the original object is left unaffected.
    //:
    //: 4 Subsequent changes in or destruction of the source object have no
    //:   effect on the copy-constructed object.
    //:
    //: 5 Subsequent changes ('push_back's) on the created object have no
    //:   effect on the original and change the capacity of the new object
    //:   correctly.
    //:
    //: 6 The object has its internal memory management system hooked up
    //:   properly so that *all* internally allocated memory draws from a
    //:   user-supplied allocator whenever one is specified.
    //:
    //: 7 The method is exception neutral w.r.t. memory allocation.
    //
    // Plan:
    //   Specify a set S of object values with substantial and varied
    //   differences, ordered by increasing length, to be used in the
    //   following tests.
    //
    //   For concerns 1 - 4, for each value in S, initialize objects w and
    //   x, copy construct y from x and use 'operator==' to verify that
    //   both x and y subsequently have the same value as w.  Let x go out
    //   of scope and again verify that w == y.
    //
    //   For concern 5, for each value in S initialize objects w and x,
    //   and copy construct y from x.  Change the state of y, by using the
    //   *primary* *manipulator* 'push_back'.  Using the 'operator!=' verify
    //   that y differs from x and w, and verify that the capacity of y
    //   changes correctly.
    //
    //   To address concern 6, we will perform tests performed for concern 1:
    //     - While passing a test allocator as a parameter to the new object
    //       and ascertaining that the new object gets its memory from the
    //       provided 'oa' allocator.  Also perform test for concerns 2 and 5.
    //    - Where the object is constructed with an object allocator, and
    //        neither of global and default allocator is used to supply memory.
    //
    //   To address concern 7, perform tests for concern 1 performed
    //   in the presence of exceptions during memory allocations using a
    //   'bslma::TestAllocator' and varying its *allocation* *limit*.
    //
    // Testing:
    //   deque(const deque& original, const A& = A());
    // ------------------------------------------------------------------------

    bslma::TestAllocator oa("object", veryVeryVeryVerbose);
    ALLOC                xoa(&oa);

    const TYPE         *values     = 0;
    const TYPE *const&  VALUES     = values;
    const int           NUM_VALUES = getValues(&values);

    // if moveable, moves do not count as allocations
    const int TYPE_MOVE  = ! bslmf::IsBitwiseMoveable<TYPE>::value;
    const int TYPE_ALLOC = bslma::UsesBslmaAllocator<TYPE>::value;

    if (verbose)
        printf("\nTesting parameters: TYPE_ALLOC = %d, TYPE_MOVE = %d.\n",
               TYPE_ALLOC, TYPE_MOVE);
    {
        static const char *SPECS[] = {
            "",
            "A",
            "BC",
            "CDE",
            "DEAB",
            "EABCD",
            "ABCDEAB",
            "ABCDEABC",
            "ABCDEABCD",
            "ABCDEABCDEABCDE",
            "ABCDEABCDEABCDEA",
            "ABCDEABCDEABCDEAB",
            0  // null string required as last element
        };

        int oldLen = -1;
        for (int ti = 0; SPECS[ti]; ++ti) {
            const char *const SPEC   = SPECS[ti];
            const int         LENGTH = (int) strlen(SPEC);

            if (verbose) {
                printf("\nFor an object of length %d:\n", LENGTH);
                P(SPEC);
            }

            LOOP_ASSERT(SPEC, oldLen < (int) LENGTH); // strictly increasing
            oldLen = LENGTH;

            // Create control object 'W'.
            Obj mW;  const Obj& W = gg(&mW, SPEC);

            LOOP_ASSERT(ti, LENGTH == (int) W.size()); // same lengths
            if (veryVerbose) { printf("\tControl Obj: "); P(W); }

            // Permutate through the initial internal representation.
            for (int ei = START_POS; ei <= FINISH_POS; ei += INCREMENT) {

                const int N = ei;
                if (veryVerbose) { printf("\t\tExtend By  : "); P(N); }

                Obj *pX = new Obj(xoa);
                Obj& mX = *pX;

                setInternalState(&mX, N);
                const Obj& X = mX;  gg(&mX, SPEC);

                if (veryVerbose) { printf("\t\tDynamic Obj: "); P(X); }

                {   // Testing concern 1.

                    if (veryVerbose) { printf("\t\t\tRegular Case :"); }

                    const Obj Y0(X);

                    if (veryVerbose) {
                        printf("\tObj : "); P_(Y0); P(Y0.capacity());
                    }

                    LOOP2_ASSERT(SPEC, N, W == Y0);
                    LOOP2_ASSERT(SPEC, N, W == X);
                    LOOP2_ASSERT(SPEC, N, Y0.get_allocator() ==
                                           bslma::Default::defaultAllocator());
                }
                {   // Testing concern 5.

                    if (veryVerbose) printf("\t\t\tInsert into created obj, "
                                            "without test allocator:\n");

                    Obj Y1(X);

                    if (veryVerbose) {
                        printf("\t\t\t\tBefore Insert: "); P(Y1);
                    }

                    for (int i = 1; i < N+1; ++i) {

                        stretch(&Y1, 1,
                           TstFacility::getIdentifier(VALUES[i % NUM_VALUES]));

                        if (veryVerbose) {
                            printf("\t\t\t\tAfter Insert : ");
                            P_(Y1.capacity()); P_(i); P(Y1);
                        }

                        LOOP3_ASSERT(SPEC, N, i, (int)Y1.size() == LENGTH + i);
                        LOOP3_ASSERT(SPEC, N, i, W != Y1);
                        LOOP3_ASSERT(SPEC, N, i, X != Y1);
                    }
                }
                {   // Testing concern 5 with test allocator.

                    if (veryVerbose)
                        printf("\t\t\tInsert into created obj, "
                                "with test allocator:\n");

                    const Int64 BB = oa.numBlocksTotal();
                    const Int64  B = oa.numBlocksInUse();

                    if (veryVerbose) {
                        printf("\t\t\t\tBefore Creation: "); P_(BB); P(B);
                    }

                    Obj Y11(X, xoa);

                    const Int64 AA = oa.numBlocksTotal();
                    const Int64  A = oa.numBlocksInUse();

                    if (veryVerbose) {
                        printf("\t\t\t\tAfter Creation: "); P_(AA); P(A);
                        printf("\t\t\t\tBefore Append: "); P(Y11);
                    }

                    for (int i = 1; i < N+1; ++i) {
                        stretch(&Y11, 1,
                           TstFacility::getIdentifier(VALUES[i % NUM_VALUES]));

                        // Blocks allocated should increase only when trying to
                        // add more than capacity.  When adding the first
                        // element, 'numBlocksInUse' will increase by 1.  In
                        // all other conditions 'numBlocksInUse' should remain
                        // the same.

                        if (veryVerbose) {
                            printf("\t\t\t\tAfter Append : ");
                            P_(i); P(Y11);
                        }

                        LOOP3_ASSERT(SPEC, N, i, (int) Y11.size() == LENGTH+i);
                        LOOP3_ASSERT(SPEC, N, i, W != Y11);
                        LOOP3_ASSERT(SPEC, N, i, X != Y11);
                        LOOP3_ASSERT(SPEC, N, i,
                                     Y11.get_allocator() == X.get_allocator());
                    }
                }
                {   // Exception checking.

                    const Int64 BB = oa.numBlocksTotal();
                    const Int64  B = oa.numBlocksInUse();

                    if (veryVerbose) {
                        printf("\t\t\t\tBefore Creation: "); P_(BB); P(B);
                    }

                    BSLMA_TESTALLOCATOR_EXCEPTION_TEST_BEGIN(oa) {
                        const Obj Y2(X, xoa);
                        if (veryVerbose) {
                            printf("\t\t\tException Case  :\n");
                            printf("\t\t\t\tObj : "); P(Y2);
                        }
                        LOOP2_ASSERT(SPEC, N, W == Y2);
                        LOOP2_ASSERT(SPEC, N, W == X);
                        LOOP2_ASSERT(SPEC, N,
                                     Y2.get_allocator() == X.get_allocator());
                    } BSLMA_TESTALLOCATOR_EXCEPTION_TEST_END

                    const Int64 AA = oa.numBlocksTotal();
                    const Int64  A = oa.numBlocksInUse();

                    if (veryVerbose) {
                        printf("\t\t\t\tAfter Creation: "); P_(AA); P(A);
                    }
                }
                {   // with 'original' destroyed
                    Obj Y5(X);
                    if (veryVerbose) {
                        printf("\t\t\tWith Original deleted: \n");
                        printf("\t\t\t\tBefore Delete : "); P(Y5);
                    }

                    delete pX;

                    LOOP2_ASSERT(SPEC, N, W == Y5);

                    for (int i = 1; i < N+1; ++i) {
                        stretch(&Y5, 1,
                           TstFacility::getIdentifier(VALUES[i % NUM_VALUES]));
                        if (veryVerbose) {
                            printf("\t\t\t\tAfter Append to new obj : ");
                            P_(i);P(Y5);
                        }
                        LOOP3_ASSERT(SPEC, N, i, W != Y5);
                    }
                }
            }
        }
    }
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::testCase6()
{
    // ------------------------------------------------------------------------
    // TESTING EQUALITY COMPARISON OPERATORS
    //
    // Concerns:
    //: 1 Objects constructed with the same values are returned as equal.
    //:
    //: 2 Objects constructed such that they have same (logical) value but
    //:   different internal representation (due to the lack or presence of an
    //:   allocator, and/or different capacities) are always returned as equal.
    //:
    //: 3 Unequal objects are always returned as unequal.
    //:
    //: 4 Correctly selects the 'bitwiseEqualityComparable' traits.
    //
    // Plan:
    //   For concerns 1 and 3, Specify a set A of unique allocators including
    //   no allocator.  Specify a set S of unique object values having various
    //   minor or subtle differences, ordered by non-decreasing length.
    //   Verify the correctness of 'operator==' and 'operator!=' (returning
    //   either true or false) using all elements '(u, ua, v, va)' of the
    //   cross product S X A X S X A.
    //
    //   For concern 2 create two objects using all elements in S one at a
    //   time.  For the second object change its internal representation by
    //   extending it by different amounts in the set E, followed by erasing
    //   its contents using 'clear'.  Then recreate the original value and
    //   verify that the second object still return equal to the first.
    //
    //   For concern 4, we instantiate this test driver on a test type having
    //   allocators or not, and possessing the bitwise-equality-comparable
    //   trait or not.
    //
    // Testing:
    //   bool operator==(const deque& lhs, const deque& rhs);
    //   bool operator!=(const deque& lhs, const deque& rhs);
    // ------------------------------------------------------------------------

    bslma::TestAllocator oa1("object1", veryVeryVeryVerbose);
    bslma::TestAllocator oa2("object2", veryVeryVeryVerbose);

    ALLOC ALLOCATOR[] = {
        ALLOC(&oa1),
        ALLOC(&oa2)
    };
    enum { NUM_ALLOCATOR = sizeof ALLOCATOR / sizeof *ALLOCATOR };

    static const char *SPECS[] = {
        "",
        "A",      "B",
        "AA",     "AB",     "BB",     "BA",
        "AAA",    "BAA",    "ABA",    "AAB",
        "AAAA",   "BAAA",   "ABAA",   "AABA",   "AAAB",
        "AAAAA",  "BAAAA",  "ABAAA",  "AABAA",  "AAABA",  "AAAAB",
        "AAAAAA", "BAAAAA", "AABAAA", "AAABAA", "AAAAAB",
        "AAAAAAA",          "BAAAAAA",          "AAAAABA",
        "AAAAAAAA",         "ABAAAAAA",         "AAAAABAA",
        "AAAAAAAAA",        "AABAAAAAA",        "AAAAABAAA",
        "AAAAAAAAAA",       "AAABAAAAAA",       "AAAAABAAAA",
        "AAAAAAAAAAA",      "AAAABAAAAAA",      "AAAAABAAAAA",
        "AAAAAAAAAAAA",     "AAAABAAAAAAA",     "AAAAABAAAAAA",
        "AAAAAAAAAAAAA",    "AAAABAAAAAAAA",    "AAAAABAAAAAAA",
        "AAAAAAAAAAAAAA",   "AAAABAAAAAAAAA",   "AAAAABAAAAAAAA",
        "AAAAAAAAAAAAAAA",  "AAAABAAAAAAAAAA",  "AAAAABAAAAAAAAA",
        0  // null string required as last element
    };

    if (verbose) printf("\nCompare each pair of similar and different"
                        " values (u, ua, v, va) in S X A X S X A"
                        " without perturbation.\n");
    {

        int oldLen = -1;

        // Create first object.
        for (int si = 0; SPECS[si]; ++si) {
            for (int ai = 0; ai < NUM_ALLOCATOR; ++ai) {

                const char *const U_SPEC = SPECS[si];
                const int         LENGTH = (int) strlen(U_SPEC);

                Obj mU(ALLOCATOR[ai]);  const Obj& U = gg(&mU, U_SPEC);
                LOOP2_ASSERT(si, ai, LENGTH == (int) U.size()); // same lengths

                if (LENGTH != oldLen) {
                    if (verbose)
                        printf( "\tUsing lhs objects of length %d.\n", LENGTH);
                    LOOP_ASSERT(U_SPEC, oldLen <= LENGTH);  //non-decreasing
                    oldLen = LENGTH;
                }

                if (veryVerbose) { T_; T_;
                    P_(si); P_(U_SPEC); P(U); }

                // Create second object.
                for (int sj = 0; SPECS[sj]; ++sj) {
                    for (int aj = 0; aj < NUM_ALLOCATOR; ++aj) {

                        const char *const V_SPEC = SPECS[sj];
                        Obj mV(ALLOCATOR[aj]);
                        const Obj& V = gg(&mV, V_SPEC);

                        if (veryVerbose) {
                            T_; T_; P_(sj); P_(V_SPEC); P(V);
                        }

                        const bool isSame = si == sj;
                        LOOP2_ASSERT(si, sj,  isSame == (U == V));
                        LOOP2_ASSERT(si, sj, !isSame == (U != V));
                    }
                }
            }
        }
    }

    if (verbose) printf("\nCompare each pair of similar values (u, ua, v, va)"
                        " in S X A X S X A after perturbing.\n");
    {
        int oldLen = -1;

        // Create first object.
        for (int si = 0; SPECS[si]; ++si) {
            for (int ai = 0; ai < NUM_ALLOCATOR; ++ai) {

                const char *const U_SPEC = SPECS[si];
                const int         LENGTH = (int) strlen(U_SPEC);

                Obj mU(ALLOCATOR[ai]);  const Obj& U = mU;
                gg(&mU, U_SPEC);
                LOOP_ASSERT(si, LENGTH == (int) U.size());  // same lengths

                if (LENGTH != oldLen) {
                    if (verbose)
                        printf( "\tUsing lhs objects of length %d.\n", LENGTH);
                    LOOP_ASSERT(U_SPEC, oldLen <= (int)LENGTH);
                    oldLen = LENGTH;
                }

                if (veryVerbose) { P_(si); P_(U_SPEC); P(U); }
                // Create second object.
                for (int sj = 0; SPECS[sj]; ++sj) {
                    for (int aj = 0; aj < NUM_ALLOCATOR; ++aj) {
                        //Perform perturbation
                        for (int e = START_POS; e <= FINISH_POS; e+=INCREMENT){

                            const char *const V_SPEC = SPECS[sj];
                            Obj mV(ALLOCATOR[aj]);  const Obj& V = mV;

                            setInternalState(&mV, e);
                            gg(&mV, V_SPEC);

                            if (veryVerbose) {
                                T_; T_; P_(sj); P_(V_SPEC); P(V);
                            }

                            const bool isSame = si == sj;
                            LOOP2_ASSERT(si, sj,  isSame == (U == V));
                            LOOP2_ASSERT(si, sj, !isSame == (U != V));
                        }
                    }
                }
            }
        }
    }
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::testCase4()
{
    // ------------------------------------------------------------------------
    // TESTING BASIC ACCESSORS
    //
    // Concerns:
    //: 1 The values returned by 'operator[]' and 'at' are correct as long as
    //:   'pos < size()'.
    //:
    //: 2 The 'at' method throws an 'out_of_range' exception if
    //:   'pos >= size()'.
    //:
    //: 3 Changing the internal representation to get the same (logical) final
    //:   value should not change the result of the element accessor methods.
    //:
    //: 4 The internal memory management is correctly hooked up so that changes
    //:   made to the state of the object via these accessors do change the
    //:   state of the object.
    //
    // Plan:
    //   For 1 and 3 do the following:
    //   Specify a set S of representative object values ordered by
    //   increasing length.  For each value w in S, initialize a newly
    //   constructed object x with w using 'gg' and verify that each basic
    //   accessor returns the expected result.  Reinitialize and repeat
    //   the same test on an existing object y after perturbing y so as to
    //   achieve an internal state representation of w that is potentially
    //   different from that of x.
    //
    //   For 2, check that 'at' throws an 'out_of_range' exception when
    //   'pos >= size()'.
    //
    //   For 4, For each value w in S, create a object x with w using
    //   'gg'.  Create another empty object y and make it 'resize' capacity
    //   equal to the size of x.  Now using the element accessor methods
    //   recreate the value of x in y.  Verify that x == y.
    //   Note - Using untested resize(int).
    //
    // Testing:
    //   allocator_type get_allocator() const;
    //   size_type size() const;
    //   const_reference operator[](size_type position) const;
    //   const_reference at(size_type position) const;
    // ------------------------------------------------------------------------

    bslma::TestAllocator oa( "object",  veryVeryVeryVerbose);
    bslma::TestAllocator oa1("object1", veryVeryVeryVerbose);
    bslma::TestAllocator oa2("object2", veryVeryVeryVerbose);

    bslma::Allocator *ALLOCATOR[] = {
        &oa,
        &oa1,
        &oa2
    };
    enum { NUM_ALLOCATOR = sizeof ALLOCATOR / sizeof *ALLOCATOR };

    const int MAX_LENGTH = 32;

    static const struct {
        int         d_lineNum;                   // source line number
        const char *d_spec_p;                    // specification string
        int         d_length;                    // expected length
        char        d_elements[MAX_LENGTH + 1];  // expected element values
    } DATA[] = {
        //line  spec            length  elements
        //----  --------------  ------  ------------------------
        { L_,   "",                  0, { }                     },
        { L_,   "A",                 1, { VA }                  },
        { L_,   "B",                 1, { VB }                  },
        { L_,   "AB",                2, { VA, VB }              },
        { L_,   "BC",                2, { VB, VC }              },
        { L_,   "BCA",               3, { VB, VC, VA }          },
        { L_,   "CAB",               3, { VC, VA, VB }          },
        { L_,   "CDAB",              4, { VC, VD, VA, VB }      },
        { L_,   "DABC",              4, { VD, VA, VB, VC }      },
        { L_,   "ABCDE",             5, { VA, VB, VC, VD, VE }  },
        { L_,   "EDCBA",             5, { VE, VD, VC, VB, VA }  },
        { L_,   "ABCDEA",            6, { VA, VB, VC, VD, VE,
                                          VA }                  },
        { L_,   "ABCDEAB",           7, { VA, VB, VC, VD, VE,
                                          VA, VB }              },
        { L_,   "BACDEABC",          8, { VB, VA, VC, VD, VE,
                                          VA, VB, VC }          },
        { L_,   "CBADEABCD",         9, { VC, VB, VA, VD, VE,
                                          VA, VB, VC, VD }      },
        { L_,   "CBADEABCDAB",      11, { VC, VB, VA, VD, VE,
                                          VA, VB, VC, VD, VA,
                                          VB }                  },
        { L_,   "CBADEABCDABC",     12, { VC, VB, VA, VD, VE,
                                          VA, VB, VC, VD, VA,
                                          VB, VC }              },
        { L_,   "CBADEABCDABCDE",   14, { VC, VB, VA, VD, VE,
                                          VA, VB, VC, VD, VA,
                                          VB, VC, VD, VE }      },
        { L_,   "CBADEABCDABCDEA",  15, { VC, VB, VA, VD, VE,
                                          VA, VB, VC, VD, VA,
                                          VB, VC, VD, VE, VA }  },
        { L_,   "CBADEABCDABCDEAB", 16, { VC, VB, VA, VD, VE,
                                          VA, VB, VC, VD, VA,
                                          VB, VC, VD, VE, VA,
                                          VB }                  },
        { L_,   "CBADEABCDABCDEABCBADEABCDABCDEA", 31,
                                        { VC, VB, VA, VD, VE,
                                          VA, VB, VC, VD, VA,
                                          VB, VC, VD, VE, VA,
                                          VB, VC, VB, VA, VD,
                                          VE, VA, VB, VC, VD,
                                          VA, VB, VC, VD, VE,
                                          VA }                  },
        { L_,   "CBADEABCDABCDEABCBADEABCDABCDEAB", 32,
                                        { VC, VB, VA, VD, VE,
                                          VA, VB, VC, VD, VA,
                                          VB, VC, VD, VE, VA,
                                          VB, VC, VB, VA, VD,
                                          VE, VA, VB, VC, VD,
                                          VA, VB, VC, VD, VE,
                                          VA, VB }              }
    };

    enum { NUM_DATA = sizeof DATA / sizeof *DATA };

    if (verbose) printf("\nTesting 'const' and non-'const' versions of "
                        "'operator[]' and 'at' where 'pos < size()'.\n");
    {
        int oldLen = -1;
        for (int ti = 0; ti < NUM_DATA; ++ti) {
            const int         LINE   = DATA[ti].d_lineNum;
            const char *const SPEC   = DATA[ti].d_spec_p;
            const int         LENGTH = DATA[ti].d_length;
            const char *const e      = DATA[ti].d_elements;

            Obj mExp;
            const Obj& EXP = gg(&mExp, e);   // expected spec

            ASSERT(LENGTH <= MAX_LENGTH);

            for (int ai = 0; ai < NUM_ALLOCATOR; ++ai) {
                Obj mX(ALLOCATOR[ai]);

                const Obj& X = gg(&mX, SPEC);  // canonical organization

                ASSERTV(ALLOCATOR[ai] == X.get_allocator());

                LOOP2_ASSERT(ti, ai, LENGTH == (int) X.size()); // same lengths

                if (veryVerbose) {
                    printf( "\ton objects of length %d:\n", LENGTH);
                }

                if (LENGTH != oldLen) {
                    LOOP2_ASSERT(LINE, ai, oldLen <= (int)LENGTH);
                          // non-decreasing
                    oldLen = LENGTH;
                }

                if (veryVerbose) printf("\t\tSpec = \"%s\"\n", SPEC);

                if (veryVerbose) {
                    T_; T_; T_; P(X);
                }

                int i;
                for (i = 0; i < LENGTH; ++i) {
                    LOOP3_ASSERT(LINE, ai, i, EXP[i] == mX[i]);
                    LOOP3_ASSERT(LINE, ai, i, EXP[i] == X[i]);
                    LOOP3_ASSERT(LINE, ai, i, EXP[i] == mX.at(i));
                    LOOP3_ASSERT(LINE, ai, i, EXP[i] == X.at(i));
                }

                for (; i < MAX_LENGTH; ++i) {
                    LOOP3_ASSERT(LINE, ai, i, 0 == e[i]);
                }

                // Perform the perturbation.
                for (int i = START_POS; i <= FINISH_POS; i += INCREMENT) {

                    Obj mY(ALLOCATOR[ai]);
                    setInternalState(&mY, i);

                    const Obj& Y = gg(&mY, SPEC);

                    ASSERTV(ALLOCATOR[ai] == Y.get_allocator());

                    if (veryVerbose) { T_; T_; T_; P_(i); P(Y); }

                    int j;
                    for (j = 0; j < LENGTH; ++j) {
                        LOOP4_ASSERT(LINE, ai, j, i, EXP[j] == mY[j]);
                        LOOP4_ASSERT(LINE, ai, j, i, EXP[j] == Y[j]);
                        LOOP4_ASSERT(LINE, ai, j, i, EXP[j] == mY.at(j));
                        LOOP4_ASSERT(LINE, ai, j, i, EXP[j] == Y.at(j));
                    }

                    for (; j < MAX_LENGTH; ++j) {
                        LOOP4_ASSERT(LINE, ai, j, i, 0 == e[j]);
                    }
                }
            }
        }
    }

    if (verbose) printf("\nTesting non-'const' versions of 'operator[]' and "
                        "'at' modify state of object correctly.\n");
    {

        int oldLen = -1;
        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const int         LINE  = DATA[ti].d_lineNum;
            const char *const SPEC  = DATA[ti].d_spec_p;
            const int       LENGTH  = DATA[ti].d_length;
            const char *const e     = DATA[ti].d_elements;

            for (int ai = 0; ai < NUM_ALLOCATOR; ++ai) {
                Obj mX(ALLOCATOR[ai]);  const Obj& X = gg(&mX, SPEC);

                ASSERTV(ALLOCATOR[ai] == X.get_allocator());

                LOOP2_ASSERT(ti, ai, LENGTH == (int) X.size()); // same lengths

                if (veryVerbose) {
                    printf("\tOn objects of length %d:\n", LENGTH);
                }

                if (LENGTH != oldLen) {
                    LOOP2_ASSERT(LINE, ai, oldLen <= (int)LENGTH);
                          // non-decreasing
                    oldLen = LENGTH;
                }

                if (veryVerbose) printf( "\t\tSpec = \"%s\"\n", SPEC);

                if (veryVerbose) {
                    T_; T_; T_; P(X);
                }

                Obj mY(ALLOCATOR[ai]);  const Obj& Y = mY;
                Obj mZ(ALLOCATOR[ai]);  const Obj& Z = mZ;

                ASSERTV(ALLOCATOR[ai] == Y.get_allocator());
                ASSERTV(ALLOCATOR[ai] == Z.get_allocator());

                mY.resize(LENGTH);
                mZ.resize(LENGTH);

                // Change state of Y and Z so its same as X.

                for (int j = 0; j < LENGTH; j++) {
                    mY[j]    = TYPE(e[j]);
                    mZ.at(j) = TYPE(e[j]);
                }

                if (veryVerbose) {
                    printf("\t\tNew object1: "); P(Y);
                    printf("\t\tNew object2: "); P(Z);
                }

                LOOP2_ASSERT(ti, ai, Y == X);
                LOOP2_ASSERT(ti, ai, Z == X);
            }
        }
    }

#ifdef BDE_BUILD_TARGET_EXC
    if (verbose) printf("\tTesting for 'out_of_range' exceptions thrown"
                        " by 'at' when 'pos >= size()'.\n");
    {

        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const int         LINE    = DATA[ti].d_lineNum;
            const char *const SPEC    = DATA[ti].d_spec_p;
            const int         LENGTH  = DATA[ti].d_length;

            for (int ai = 0; ai < NUM_ALLOCATOR; ++ai) {
                int exceptions, trials;

                const int NUM_TRIALS = 2;

                // Check exception behavior for non-'const' version of 'at'.
                // Checking the behavior for 'pos == size()' and
                // 'pos > size()'.

                for (exceptions = 0, trials = 0; trials < NUM_TRIALS
                                               ; ++trials) {
                    try {
                        Obj mX(ALLOCATOR[ai]);
                        gg(&mX, SPEC);
                        mX.at(LENGTH + trials);
                    } catch (const std::out_of_range& ex) {
                        ++exceptions;
                        if (veryVerbose) {
                            printf("In out_of_range exception.\n");
                            printf("Exception: %s\n", ex.what());
                            P_(LINE); P(trials);
                        }
                        continue;
                    }
                }

                ASSERT(exceptions == trials);

                // Check exception behavior for 'const' version of 'at'.
                for (exceptions = 0, trials = 0; trials < NUM_TRIALS
                                               ; ++trials) {

                    try {
                        Obj mX(ALLOCATOR[ai]);
                        const Obj& X = gg(&mX, SPEC);
                        X.at(LENGTH + trials);
                    } catch (const std::out_of_range& ex) {
                        ++exceptions;
                        if (veryVerbose) {
                            printf("In out_of_range exception.\n" );
                            printf("Exception: %s\n", ex.what());
                            P_(LINE); P(trials);
                        }
                        continue;
                    }
                }

                ASSERT(exceptions == trials);
            }
        }
    }
#endif // BDE_BUILD_TARGET_EXC
}

template <class TYPE,
           bool TYPE_USES_ALLOCATOR = bslma::UsesBslmaAllocator<TYPE>::value>
struct AssertAllocator {
    void operator() (const TYPE& element, bslma::Allocator *allocator) const
    {
        // Assert that the specified 'element' has the specified 'allocator'.
        // This function template exists to allow conditional testing of
        // allocator properties in template code where not all types use
        // allocators, but all branches within the function must parse
        // correctly for any type.

        ASSERT(element.allocator() == allocator);
    }
};

template <class TYPE>
struct AssertAllocator<TYPE, false> {
    void operator() (const TYPE&, bslma::Allocator *) const
    {}
};

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::testCase3()
{
    // ------------------------------------------------------------------------
    // TESTING PRIMITIVE GENERATOR FUNCTIONS 'gg' AND 'ggg'
    //   Having demonstrated that our primary manipulators work as expected
    //   under normal conditions, we want to verify (1) that valid generator
    //   syntax produces expected results and (2) that invalid syntax is
    //   detected and reported.
    //
    // Plan:
    //   For each of an enumerated sequence of 'spec' values, ordered by
    //   increasing 'spec' length, use the primitive generator function
    //   'gg' to set the state of a newly created object.  Verify that 'gg'
    //   returns a valid reference to the modified argument object and,
    //   using basic accessors, that the value of the object is as
    //   expected.  Repeat the test for a longer 'spec' generated by
    //   prepending a string ending in a '~' character (denoting
    //   'clear').  Note that we are testing the parser only; the
    //   primary manipulators are already assumed to work.
    //
    //   For each of an enumerated sequence of 'spec' values, ordered by
    //   increasing 'spec' length, use the primitive generator function
    //   'ggg' to set the state of a newly created object.  Verify that
    //   'ggg' returns the expected value corresponding to the location of
    //   the first invalid value of the 'spec'.  Repeat the test for a
    //   longer 'spec' generated by prepending a string ending in a '~'
    //   character (denoting 'clear').  Note that we are testing the
    //   parser only; the primary manipulators are already assumed to work.
    //
    // Testing:
    //   int ggg(Obj *object, const char *spec, bool vF = true);
    //   Obj& gg(Obj *object, const char *spec);
    // ------------------------------------------------------------------------

    // Primary inspector for this test are the iterators returned by 'begin'
    // and 'end'.  We must not rely on 'capacity' or 'size', which will be
    // tested in case 4 or later.
    typedef typename Obj::const_iterator const_iterator;

    const AssertAllocator<TYPE> assertAllocator = {};

    bslma::TestAllocator oa("object", veryVeryVeryVerbose);
    ALLOC                xoa(&oa);

    const TYPE *values = 0;
    (void)getValues(&values);

    if (verbose) printf("\nTesting generator on simple specs.\n");
    {
        const int MAX_LENGTH = 10;
        static const struct {
            int         d_lineNum;               // source line number
            const char *d_spec_p;                // specification string
            int         d_length;                // expected length
            char        d_elements[MAX_LENGTH];  // expected element values
        } DATA[] = {
            //line  spec            length  elements
            //----  --------------  ------  ------------------------
            { L_,   "",             0,      { 0 }                   },

            { L_,   "A",            1,      { VA }                  },
            { L_,   "B",            1,      { VB }                  },
            { L_,   "~",            0,      { 0 }                   },

            { L_,   "CD",           2,      { VC, VD }              },
            { L_,   "E~",           0,      { 0 }                   },
            { L_,   "~E",           1,      { VE }                  },
            { L_,   "~~",           0,      { 0 }                   },

            { L_,   "ABC",          3,      { VA, VB, VC }          },
            { L_,   "~BC",          2,      { VB, VC }              },
            { L_,   "A~C",          1,      { VC }                  },
            { L_,   "AB~",          0,      { 0 }                   },
            { L_,   "~~C",          1,      { VC }                  },
            { L_,   "~B~",          0,      { 0 }                   },
            { L_,   "A~~",          0,      { 0 }                   },
            { L_,   "~~~",          0,      { 0 }                   },

            { L_,   "ABCD",         4,      { VA, VB, VC, VD }      },
            { L_,   "~BCD",         3,      { VB, VC, VD }          },
            { L_,   "A~CD",         2,      { VC, VD }              },
            { L_,   "AB~D",         1,      { VD }                  },
            { L_,   "ABC~",         0,      { 0 }                   },

            { L_,   "ABCDE",        5,      { VA, VB, VC, VD, VE }  },
            { L_,   "~BCDE",        4,      { VB, VC, VD, VE }      },
            { L_,   "AB~DE",        2,      { VD, VE }              },
            { L_,   "ABCD~",        0,      { 0 }                   },
            { L_,   "A~C~E",        1,      { VE }                  },
            { L_,   "~B~D~",        0,      { 0 }                   },

            { L_,   "~CBA~~ABCDE",  5,      { VA, VB, VC, VD, VE }  },

            { L_,   "ABCDE~CDEC~E", 1,      { VE }                  }
        };
        enum { NUM_DATA = sizeof DATA / sizeof *DATA };

        int oldLen = -1;
        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const int         LINE   = DATA[ti].d_lineNum;
            const char *const SPEC   = DATA[ti].d_spec_p;
            const size_t      LENGTH = DATA[ti].d_length;
            const char *const e      = DATA[ti].d_elements;
            const int         curLen = (int)strlen(SPEC);

            Obj mX(xoa);
            const Obj& X = gg(&mX, SPEC);  // original spec

            static const char *const MORE_SPEC = "~ABCDEFGHIJKLMNOPQRST~";
            char buf[100]; strcpy(buf, MORE_SPEC); strcat(buf, SPEC);

            Obj mY(xoa);
            const Obj& Y = gg(&mY, buf);  // extended spec

            if (curLen != oldLen) {
                if (verbose) printf("\tof length %d:\n", curLen);
                LOOP_ASSERT(LINE, oldLen <= curLen);  // non-decreasing
                oldLen = curLen;
            }

            if (veryVerbose) {
                printf("\t\tSpec = \"%s\"\n", SPEC);
                printf("\t\tBigSpec = \"%s\"\n", buf);
                T_; T_; T_; P(X);
                T_; T_; T_; P(Y);
            }

            LOOP_ASSERT(LINE, LENGTH == X.size());
            LOOP_ASSERT(LINE, LENGTH == Y.size());
            for (size_t i = 0; i < LENGTH; ++i) {
                LOOP2_ASSERT(LINE, i, TYPE(e[i]) == X[i]);
                LOOP2_ASSERT(LINE, i, TYPE(e[i]) == Y[i]);
            }
        }
    }

    Int64 numAllocsPerItem;
    {
        BSLMF_ASSERT(PageLength<TYPE>::k_VALUE >= 2);    // This won't work if
                                                         // page length is < 2.
        const TYPE *VALUES;
        getValues(&VALUES);

        Obj         mX(xoa);
        const Int64 PRE_A = oa.numAllocations();
        mX.push_back(VALUES[0]);
        numAllocsPerItem = oa.numAllocations() - PRE_A;
    }

    if (verbose) printf("\nTesting generator on invalid simple specs.\n");
    {
        static const struct {
            int         d_lineNum;  // source line number
            const char *d_spec_p;   // specification string
            int         d_index;    // offending character index
        } DATA[] = {
            //line  spec            index
            //----  -------------   -----
            { L_,   "",             -1,     }, // control
            { L_,   " ",            -1,     }, // now valid
            { L_,   "  ",           -1,     }, // now valid
            { L_,   "   ",          -1,     }, // now valid

            { L_,   "~",            -1,     }, // control
            { L_,   "\t",            0,     },
            { L_,   ".",             0,     },
            { L_,   "Y",            -1,     }, // control
            { L_,   "Z",             0,     },
            { L_,   "y",            -1,     }, // control
            { L_,   "z",             0,     },

            { L_,   "Yy",           -1,     }, // control
            { L_,   "zZ",            0,     },
            { L_,   "Zz",            0,     },
            { L_,   ".~",            0,     },
            { L_,   "~!",            1,     },

            { L_,   "ABC",          -1,     }, // control
            { L_,   "\tBC",          0,     },
            { L_,   "A\nC",          1,     },
            { L_,   "AB\t",          2,     },
            { L_,   "?#:",           0,     },
            { L_,   "???",           0,     },

            { L_,   "ABCDE",        -1,     }, // control
            { L_,   "ZBCDE",         0,     },
            { L_,   "ABZDE",         2,     },
            { L_,   "ABCDZ",         4,     },
            { L_,   "AZCZE",         1,     }
        };
        enum { NUM_DATA = sizeof DATA / sizeof *DATA };

        int oldLen = -1;
        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const int         LINE   = DATA[ti].d_lineNum;
            const char *const SPEC   = DATA[ti].d_spec_p;
            const int         INDEX  = DATA[ti].d_index;
            const int         LENGTH = (int)strlen(SPEC);

            Obj mX(xoa);

            if (LENGTH != oldLen) {
                if (verbose) printf("\tof length %d:\n", LENGTH);
                oldLen = LENGTH;
            }

            if (veryVerbose) printf("\t\tSpec = \"%s\"\n", SPEC);

            int result = ggg(&mX, SPEC, veryVerbose);

            ASSERTV(LINE, SPEC, INDEX, result, INDEX == result);
        }
    }

    if (verbose) printf("\nTesting simple fill to back of the first page.\n");
    {
        // Repeat the same test with a variety of representative whitespace
        // patterns, and then deem white-space largely tested.

        static const struct {
            int         d_lineNum;  // source line number
            const char *d_spec_p;   // specification string
        } DATA[] = {
            //line  spec
            //----  -------------
            { L_,   "<A|"             },
            { L_,   " <A|"            },
            { L_,   "< A|"            },
            { L_,   "<A |"            },
            { L_,   "<A| "            },
            { L_,   "  <A|"           },
            { L_,   "<A|  "           },
            { L_,   "<  A  |"         },
            { L_,   " < A | "         },
            { L_,   "A<A|"            },
            { L_,   "A <A.| A"        },
            { L_,   "<A...| AAA"      },
            { L_,   "<A . . . | AAA"  }
        };
        enum { NUM_DATA = sizeof DATA / sizeof *DATA };

        const bool  TYPE_ALLOCATES = bslma::UsesBslmaAllocator<TYPE>::value ||
                                     bsl::uses_allocator<TYPE, ALLOC>::value;

        const int   EXPECTED_LENGTH   = PageLength<TYPE>::k_VALUE / 2;
        const int   EXTRA_ALLOCATIONS = TYPE_ALLOCATES
                                      ? 2 * EXPECTED_LENGTH
                                      : 0;
        const TYPE *VALUES;
        getValues(&VALUES);

        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const char *const SPEC    = DATA[ti].d_spec_p;

            const Int64 PREB = oa.numAllocations();

            Obj         mX(xoa);
            const Int64 AB = oa.numAllocations();

            // exactly 2 allocations for a default-constructed 'deque'

            ASSERTV(PREB + 2, AB, PREB + 2 == AB);

            const Obj&  X  = gg(&mX, SPEC);    // generate
            const Int64 BB = oa.numAllocations();

            // Confirm expected value, including expected length.
            for (const_iterator it = X.begin(); it != X.end(); ++it) {
                // Do not want to introduce cyclic dependency on
                // <bsl_algorithm.h>.  Would use range-for loop in C++11.

                ASSERT(*it == *VALUES);

                // This part should already be guaranteed by bootstrap test
                // case 2 for 'push_back', but added while observing an
                // unexpected number of allocations in testing.

                if (TYPE_ALLOCATES) {
                    assertAllocator(*X.begin(), &oa);
                }
            }

            const IntPtr OBSERVED_LENGTH = X.end() - X.begin();
            ASSERTV(OBSERVED_LENGTH,   EXPECTED_LENGTH,
                    OBSERVED_LENGTH == EXPECTED_LENGTH);

            // Confirm expected number of allocations.
            ASSERTV(AB + EXTRA_ALLOCATIONS,   BB, sizeof(TYPE),
                    AB + EXTRA_ALLOCATIONS == BB);

            // Confirm that one additional push allocates a fresh data page.

            mX.push_back(values[0]);          // push_back onto next data page
            const Int64 CB = oa.numAllocations();

            // Reconfirm expected value, including expected length.
            for (const_iterator it = X.begin(); it != X.end(); ++it) {
                // Do not want to introduce cyclic dependency on
                // <bsl_algorithm.h>.  Would use range-for loop in C++11.

                ASSERT(*it == *VALUES);

                // This part should already be guaranteed by bootstrap test
                // case 2 for 'push_back', but added while observing an
                // unexpected number of allocations in testing.

                if (TYPE_ALLOCATES) {
                    assertAllocator(*--X.end(), &oa);
                }
            }

            const IntPtr REVISED_LENGTH = X.end() - X.begin();
            ASSERTV(REVISED_LENGTH,   EXPECTED_LENGTH + 1,
                    REVISED_LENGTH == EXPECTED_LENGTH + 1);

            ASSERTV(BB, CB, (BB + TYPE_ALLOCATES) ? 1 : 2 == CB);
        }

        // Pushing one item to the back should allocate another page.

        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const char *const SPEC    = DATA[ti].d_spec_p;

            Obj mX(xoa);
            (void) gg(&mX, SPEC);    // generate

            const Int64 PRE_PUSH  = oa.numAllocations();
            mX.push_back(values[0]);
            const Int64 POST_PUSH = oa.numAllocations();

            // Should have allocated another page.

            ASSERTV(POST_PUSH, PRE_PUSH, numAllocsPerItem,
                                 POST_PUSH == PRE_PUSH + 1 + numAllocsPerItem);
        }
    }

    if (verbose) printf("\nTesting simple fill to front of the first page.\n");
    {
        // Repeat the same test with a variety of representative whitespace
        // patterns, and then deem white-space largely tested.

        static const struct {
            int         d_lineNum;  // source line number
            const char *d_spec_p;   // specification string
        } DATA[] = {
            //line  spec
            //----  -------------
            { L_,   "<b|"             },
            { L_,   " <b|"            },
            { L_,   "< b|"            },
            { L_,   "<b |"            },
            { L_,   "<b| "            },
            { L_,   "  <b|"           },
            { L_,   "<b|  "           },
            { L_,   "<  b  |"         },
            { L_,   " < b | "         },
            { L_,   "b<b|"            },
            { L_,   "b <b.| b"        },
            { L_,   "<b...| bbb"      },
            { L_,   "<b . . . | bbb"  }
        };
        enum { NUM_DATA = sizeof DATA / sizeof *DATA };

        const bool  TYPE_ALLOCATES = bslma::UsesBslmaAllocator<TYPE>::value ||
                                     bsl::uses_allocator<TYPE, ALLOC>::value;

        const int   EXPECTED_LENGTH   = (PageLength<TYPE>::k_VALUE + 1)/2 - 1;
        const int   EXTRA_ALLOCATIONS = TYPE_ALLOCATES
                                      ? 2 * EXPECTED_LENGTH
                                      : 0;
        const TYPE *VALUES;
        getValues(&VALUES);

        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const char *const SPEC    = DATA[ti].d_spec_p;

            const Int64 PREF = oa.numAllocations();

            Obj         mX(xoa);
            const Int64 AF = oa.numAllocations();

            // exactly 2 allocations for a default-constructed 'deque'

            ASSERTV(PREF + 2, AF, PREF + 2 == AF);

            const Obj&  X  = gg(&mX, SPEC);
            const Int64 BF = oa.numAllocations();

            // Confirm expected value, including expected length.
            for (const_iterator it = X.begin(); it != X.end(); ++it) {
                // Do not want to introduce cyclic dependency on
                // <bsl_algorithm.h>.  Would use range-for loop in C++11.

                ASSERT(*it == VALUES[1]);

                // This part should already be guaranteed by bootstrap test
                // case 2 for 'push_back', but added while observing an
                // unexpected number of allocations in testing.

                if (TYPE_ALLOCATES) {
                    assertAllocator(*X.begin(), &oa);
                }
            }

            const IntPtr OBSERVED_LENGTH = X.end() - X.begin();
            ASSERTV(OBSERVED_LENGTH,   EXPECTED_LENGTH,
                    OBSERVED_LENGTH == EXPECTED_LENGTH);

            // Confirm expected number of allocations.
            ASSERTV(AF + EXTRA_ALLOCATIONS,   BF, sizeof(TYPE),
                    AF + EXTRA_ALLOCATIONS == BF);

            // Confirm that one additional push allocates a fresh data page.

            mX.push_front(VALUES[1]);
            const Int64 CF = oa.numAllocations();

            // Reconfirm expected value, including expected length.
            for (const_iterator it = X.begin(); it != X.end(); ++it) {
                // Do not want to introduce cyclic dependency on
                // <bsl_algorithm.h>.  Would use range-for loop in C++11.

                ASSERT(*it == VALUES[1]);

                // This part should already be guaranteed by bootstrap test
                // case 2 for 'push_back', but added while observing an
                // unexpected number of allocations in testing.

                if (TYPE_ALLOCATES) {
                    assertAllocator(*X.begin(), &oa);
                }
            }

            const IntPtr REVISED_LENGTH = X.end() - X.begin();
            ASSERTV(REVISED_LENGTH,   EXPECTED_LENGTH + 1,
                    REVISED_LENGTH == EXPECTED_LENGTH + 1);

            ASSERTV(BF, CF, (BF + TYPE_ALLOCATES) ? 1 : 2 == CF);
        }

        // Pushing one item to the front should allocate another page.

        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const char *const SPEC = DATA[ti].d_spec_p;

            Obj mX(xoa);
            (void) gg(&mX, SPEC);    // generate

            const Int64 PRE_PUSH  = oa.numAllocations();
            mX.push_front(VALUES[0]);
            const Int64 POST_PUSH = oa.numAllocations();

            // Should have allocated another page.

            ASSERT(POST_PUSH == PRE_PUSH + 1 + numAllocsPerItem);
        }
    }

    if (verbose) printf("\nTesting generator on invalid range specs.\n");
    {
        static const struct {
            int         d_lineNum;  // source line number
            const char *d_spec_p;   // specification string
            int         d_index;    // offending character index
        } DATA[] = {
            //line  spec            index
            //----  -------------   -----
            { L_,   "",             -1,     }, // control

            { L_,   "~",            -1,     }, // control
            { L_,   "<",             0,     },
            { L_,   "|",             0,     },
            { L_,   "E",             -1,    }, // control

            { L_,   "AE",           -1,     }, // control
            { L_,   "< ",            0,     },
            { L_,   "<A",            0,     },
            { L_,   "<|",            1,     },
            { L_,   "<<",            1,     },
            { L_,   "<~",            1,     },
            { L_,   " <",            1,     },
            { L_,   "eA",           -1,     }, // control

            { L_,   "<A|",          -1,     }, // control
            { L_,   "<.|",           1,     },
            { L_,   "<7|",           1,     },
            { L_,   "<~|",           1,     },
            { L_,   "<||",           1,     },
            { L_,   "< |",           2,     },

            { L_,   "<A.|",         -1,     }, // control
            { L_,   "<~A|",          1,     },
            { L_,   "<2A|",          1,     },
            { L_,   "<.A|",          1,     },
            { L_,   "< A|",         -1,     }, // control
            { L_,   "<AA|",          2,     },
            { L_,   "<AB|",          2,     },
            { L_,   "<A~|",          2,     },
            { L_,   "<A3|",          2,     }, // index 3 if implement repeats
            { L_,   "<A |",         -1,     }, // control

            { L_,   "<A| A <A|",    -1,     }, // control
            { L_,   "<A| <A|",       6,     },
            { L_,   "<b| <b|",       6,     },
            { L_,   "<b| b <b|",    -1,     }, // control
            { L_,   "<A.| A <A|",    9,     },
            { L_,   "<b.| b <b|",    9,     },
            { L_,   "<A.| AA <A|",  -1,     }, // control
            { L_,   "<b.| bb <b|",  -1,     }, // control
        };
        enum { NUM_DATA = sizeof DATA / sizeof *DATA };

        int oldLen = -1;
        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const int         LINE    = DATA[ti].d_lineNum;
            const char *const SPEC    = DATA[ti].d_spec_p;
            const int         INDEX   = DATA[ti].d_index;
            const int         LENGTH  = (int) strlen(SPEC);

            Obj mX(xoa);

            if (LENGTH != oldLen) {
                if (verbose) printf("\tof length %d:\n", LENGTH);
                oldLen = LENGTH;
            }

            if (veryVerbose) printf("\t\tSpec = \"%s\"\n", SPEC);

            int result = ggg(&mX, SPEC, veryVerbose);

            ASSERTV(LINE, SPEC, INDEX, result, INDEX == result);
        }
    }

    // The following test will need to be custom tailored beyond a loop, but
    // initial testing with a loop verifies that the scripts are successfully
    // interpreted.  Extra testing regarding contents of the deque and memory
    // allocation crossing boundaries will require per-spec testing though, so
    // this table is only a place-holder for future testing.
    if (verbose) printf("\nTesting generator on sample range specs.\n");
    {
        static const struct {
            int         d_lineNum;  // source line number
            const char *d_spec_p;   // specification string
        } DATA[] = {
            //line  spec
            //----  -------------
            { L_,   ""             }, // control

            { L_,   "<A|"          }, // tested
            { L_,   "<B|"          }, // tested
            { L_,   "A<B|"         },
            { L_,   "A <B|"        },

            { L_,   "<a|"          },
            { L_,   "<b|"          },
            { L_,   "a<b|"         },
            { L_,   "a <b|"        },

            { L_,   "a<B|"         },
            { L_,   "A <b|"        },

            { L_,   "a<B|  <c|"    },
            { L_,   "A <b| <C|"    },
        };
        enum { NUM_DATA = sizeof DATA / sizeof *DATA };
        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const char *const SPEC = DATA[ti].d_spec_p;

            Obj mX(xoa);
            const Obj& X = gg(&mX, SPEC);  // original spec
            (void) X;
        }
    }

    if (verbose) printf(
                    "\nTesting simple fill to both ends of the first page.\n");
    {
        // Repeat the same test with a variety of representative whitespace
        // patterns, and then deem white-space largely tested.

        static const struct {
            int         d_lineNum;  // source line number
            const char *d_spec_p;   // specification string
        } DATA[] = {
            //line  spec
            //----  -------------
            { L_,   "<A|<b|"          },
            { L_,   " <A| <b|"        },
            { L_,   "< A|< b|"        },
            { L_,   "<A |<b |"        },
            { L_,   "<A| <b| "        },
            { L_,   "  <A|  <b|"      },
            { L_,   "<A|  <b|  "      },
            { L_,   "<  A  |<  b  |"  },
            { L_,   " < A |  < b | "  },
            { L_,   "A<A|b<b|"        },
            { L_,   "A <A.| Ab <b.| b" },
            { L_,   "<A...| AAA<b...| bbb" },
            { L_,   "<A . . . | AAA<b . . . | bbb"  },
            { L_,   "<b|<A|"          },
            { L_,   " <b| <A|"        },
            { L_,   "< b|< A|"        },
            { L_,   "<b |<A |"        },
            { L_,   "<b| <A| "        },
            { L_,   "  <b|  <A|"      },
            { L_,   "<b|  <A|  "      },
            { L_,   "<  b  |<  A  |"  },
            { L_,   " < b |  < A | "  },
            { L_,   "b<b|A<A|"        },
            { L_,   "b <b.| bA <A.| A" },
            { L_,   "<b...| bbb<A...| AAA" },
            { L_,   "<b . . . | bbb<A . . . | AAA"  }
        };
        enum { NUM_DATA = sizeof DATA / sizeof *DATA };

        const bool  TYPE_ALLOCATES = bslma::UsesBslmaAllocator<TYPE>::value ||
                                     bsl::uses_allocator<TYPE, ALLOC>::value;

        const int   EXPECTED_LENGTH   = PageLength<TYPE>::k_VALUE - 1;
        const int   EXTRA_ALLOCATIONS = TYPE_ALLOCATES ? 2 * EXPECTED_LENGTH
                                                       : 0;
        const TYPE *VALUES;
        getValues(&VALUES);

        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const char *const SPEC = DATA[ti].d_spec_p;

            const Int64 PREB = oa.numAllocations();

            Obj         mX(xoa);
            const Int64 AB = oa.numAllocations();

            // exactly 2 allocations for a default-constructed 'deque'

            ASSERTV(PREB + 2, AB, PREB + 2 == AB);

            const Obj&  X  = gg(&mX, SPEC);    // generate
            const Int64 BB = oa.numAllocations();

            ASSERT(VALUES[0] == X.back());
            ASSERT(VALUES[1] == X.front());

            const IntPtr OBSERVED_LENGTH = X.end() - X.begin();
            ASSERTV(OBSERVED_LENGTH,   EXPECTED_LENGTH,
                    OBSERVED_LENGTH == EXPECTED_LENGTH);

            // Confirm expected number of allocations.

            ASSERTV(AB + EXTRA_ALLOCATIONS,   BB, sizeof(TYPE),
                    AB + EXTRA_ALLOCATIONS == BB);

            // Confirm pushing to either end allocates exactly one more page.

            Int64 prePush  = oa.numAllocations();
            mX.push_back(values[0]);
            Int64 postPush = oa.numAllocations();

            // Should have allocated another page.

            ASSERT(prePush + 1 + numAllocsPerItem == postPush);

            prePush = oa.numAllocations();
            mX.push_front(values[0]);
            postPush = oa.numAllocations();

            // Should have allocated another page.

            ASSERT(prePush + 1 + numAllocsPerItem == postPush);
        }
    }
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::testCase2a()
{
    // ------------------------------------------------------------------------
    // TESTING PRIMARY MANIPULATORS (BOOTSTRAP):
    //   The basic concern is that the default constructor, the destructor,
    //   and, under normal conditions (i.e., no aliasing), the primary
    //   manipulators:
    //      - push_back             (black-box)
    //      - clear                 (white-box)
    //   operate as expected.
    //
    // Concerns:
    //: 1 The default constructor:
    //:   a) creates the correct initial value.
    //:   b) allocates the minimum expected memory.
    //:   c) has the internal memory management system hooked up properly so
    //:      that *all* internally allocated memory draws from the same
    //:      user-supplied allocator whenever one is specified.
    //:
    //: 2 The destructor properly deallocates all allocated memory to its
    //:   corresponding allocator from any attainable state.
    //:
    //: 3 'push_back':
    //:   a) produces the expected value.
    //:   b) increases capacity as needed.
    //:   c) maintains valid internal state.
    //:   d) is exception-neutral with respect to memory allocation.
    //:
    //: 4 'clear':
    //:   a) produces the expected value (empty).
    //:   b) properly destroys each contained element value.
    //:   c) maintains valid internal state.
    //:   d) does not allocate memory.
    //
    // Plan:
    //   To address concerns 1a - 1c, create an object using the default
    //   constructor:
    //    - With and without passing in an allocator.
    //    - In the presence of exceptions during memory allocations using
    //      a 'bslma::TestAllocator' and varying its *allocation* *limit*.
    //    - Where the object is constructed with an object allocator, and
    //      neither of global and default allocator is used to supply memory.
    //
    //   To address concerns 3a - 3c, construct a series of independent
    //   objects, ordered by increasing length.  In each test, allow the
    //   object to leave scope without further modification, so that the
    //   destructor asserts internal object invariants appropriately.
    //   After the final insert operation in each test, use the (untested)
    //   basic accessors to cross-check the value of the object
    //   and the 'bslma::TestAllocator' to confirm whether a resize has
    //   occurred.
    //
    //   To address concerns 4a-4c, construct a similar test, replacing
    //   'push_back' with 'clear'; this time, however, use the test
    //   allocator to record *numBlocksInUse* rather than *numBlocksTotal*.
    //
    //   To address concerns 2, 3d, 4d, create a small "area" test that
    //   exercises the construction and destruction of objects of various
    //   lengths and capacities in the presence of memory allocation
    //   exceptions.  Two separate tests will be performed.
    //
    //   Let S be the sequence of integers { 0 .. N - 1 }.
    //      (1) for each i in S, use the default constructor and 'push_back'
    //          to create an object of length i, confirm its value (using
    //           basic accessors), and let it leave scope.
    //      (2) for each (i, j) in S X S, use 'push_back' to create an
    //          object of length i, use 'clear' to clear its value and
    //          confirm (with 'length'), use insert to set the object to a
    //          value of length j, verify the value, and allow the object to
    //          leave scope.
    //
    //   The first test acts as a "control" in that 'clear' is not
    //   called; if only the second test produces an error, we know that
    //   'clear' is to blame.  We will rely on 'bslma::TestAllocator'
    //   and purify to address concern 2, and on the object invariant
    //   assertions in the destructor to address concerns 3d and 4d.
    //
    // Testing:
    //   deque(const A& a = A());
    //   ~deque();
    //   void clear();
    //   BOOTSTRAP: void push_back(const T& value);
    // ------------------------------------------------------------------------

    const TYPE         *values     = 0;
    const TYPE *const&  VALUES     = values;
    const int           NUM_VALUES = getValues(&values);

    // ------------------------------------------------------------------------

    bslma::TestAllocator oa("object", veryVeryVeryVerbose);
    ALLOC                xoa(&oa);

    bslma::TestAllocator         da("default", veryVeryVeryVerbose);
    bslma::DefaultAllocatorGuard guard(&da);

    if (verbose) printf("\n\tTesting default ctor (thoroughly).\n");

    // Default construction should perform exactly two allocations, the first
    // for the block array and the second for the first block.  An empty
    // container should otherwise allocate no additional memory.

    if (verbose) printf("\t\tWithout passing in an allocator.\n");
    {
        const Int64 B = da.numBlocksInUse();

        ASSERT(bslma::Default::allocator() == &da);

        BSLMA_TESTALLOCATOR_EXCEPTION_TEST_BEGIN(da) {
            const Int64 BB = da.numBlocksTotal();
            const Int64 B  = da.numBlocksInUse();

            const Obj X;

            if (veryVerbose) { T_; T_; P(X); }

            ASSERT(0   == X.size());
            ASSERT(&da == X.get_allocator().mechanism());

            ASSERTV(BB, da.numBlocksTotal(), BB + 2 == da.numBlocksTotal());
            ASSERTV(B,  da.numBlocksInUse(),  B + 2 == da.numBlocksInUse());
        } BSLMA_TESTALLOCATOR_EXCEPTION_TEST_END

        ASSERTV(B, da.numBlocksInUse(), B == da.numBlocksInUse());
    }

    if (verbose) printf("\t\tWith passing a test allocator.\n");
    {
        bslma::TestAllocatorMonitor dam(&da);

        ASSERT(0 == oa.numBlocksInUse());

        BSLMA_TESTALLOCATOR_EXCEPTION_TEST_BEGIN(oa) {
            const Int64 BB = oa.numBlocksTotal();
            const Int64 B  = oa.numBlocksInUse();

            const Obj X(xoa);

            if (veryVerbose) { T_; T_; P(X); }

            ASSERT(0              == X.size());
            ASSERT(&oa == X.get_allocator().mechanism());

            ASSERTV(BB, oa.numBlocksTotal(), BB + 2 == oa.numBlocksTotal());
            ASSERTV( B, oa.numBlocksInUse(), B + 2 == oa.numBlocksInUse());
            ASSERT(dam.isTotalSame());
        } BSLMA_TESTALLOCATOR_EXCEPTION_TEST_END

        ASSERTV(oa.numBlocksInUse(), 0 == oa.numBlocksInUse());
        ASSERT(dam.isTotalSame());
    }

    // ------------------------------------------------------------------------

    if (verbose)
        printf("\n\tTesting 'push_back' (bootstrap) without allocator.\n");
    {
        const size_t NUM_TRIALS = LARGE_SIZE_VALUE;

        for (size_t li = 0; li < NUM_TRIALS; ++li) {
            if (verbose) {
                printf("\t\tOn an object of initial length %llu.\n",
                                                                   (Uint64)li);
            }

            Obj mX;  const Obj& X = mX;

            for (size_t i = 0; i < li; ++i) {
                mX.push_back(VALUES[i % NUM_VALUES]);
            }

            LOOP_ASSERT(li, li == X.size());

            if (veryVerbose){
                printf("\t\t\tBEFORE: "); P_(X.capacity()); P(X);
            }

            mX.push_back(VALUES[li % NUM_VALUES]);

            if (veryVerbose){
                printf("\t\t\tAFTER : "); P_(X.capacity()); P(X);
            }

            LOOP_ASSERT(li, li + 1 == X.size());

            for (size_t i = 0; i < li; ++i) {
                LOOP2_ASSERT(li, i, VALUES[i % NUM_VALUES] == X[i]);
            }

            LOOP_ASSERT(li, VALUES[li % NUM_VALUES] == X[li]);
        }
    }

    // ------------------------------------------------------------------------

    if (verbose)
        printf("\n\tTesting 'push_back' (bootstrap) with allocator.\n");
    {
        const size_t NUM_TRIALS = LARGE_SIZE_VALUE;

        for (size_t li = 0; li < NUM_TRIALS; ++li) {
            if (verbose) {
                printf("\t\tOn an object of initial length %llu.\n",
                                                                   (Uint64)li);
            }

            Obj mX(xoa);  const Obj& X = mX;

            for (size_t i = 0; i < li; ++i) {
                mX.push_back(VALUES[i % NUM_VALUES]);
            }

            LOOP_ASSERT(li, li == X.size());

            const Int64 BB = oa.numBlocksTotal();
            const Int64 B  = oa.numBlocksInUse();

            if (veryVerbose) {
                printf("\t\t\tBEFORE: ");
                P_(BB); P_(B); P_(X.capacity()); P(X);
            }

            mX.push_back(VALUES[li % NUM_VALUES]);

            const Int64 AA = oa.numBlocksTotal();
            const Int64 A  = oa.numBlocksInUse();

            if (veryVerbose) {
                printf("\t\t\t AFTER : ");
                P_(AA); P_(A); P_(X.capacity()); P(X);
            }

            LOOP_ASSERT(li, li + 1 == X.size());

            for (size_t i = 0; i < li; ++i) {
                LOOP2_ASSERT(li, i, VALUES[i % NUM_VALUES] == X[i]);
            }
            LOOP_ASSERT(li, VALUES[li % NUM_VALUES] == X[li]);
        }
    }
    ASSERT(0 == oa.numMismatches());
    ASSERT(0 == oa.numBlocksInUse());

    // ------------------------------------------------------------------------

    if (verbose) printf("\n\tTesting 'clear' without allocator.\n");
    {
        const size_t NUM_TRIALS = LARGE_SIZE_VALUE;

        for (size_t li = 0; li < NUM_TRIALS; ++li) {
            if (verbose) {
                printf("\t\tOn an object of initial length %llu.\n",
                                                                   (Uint64)li);
            }

            Obj mX;  const Obj& X = mX;

            for (size_t i = 0; i < li; ++i) {
                mX.push_back(VALUES[i % NUM_VALUES]);
            }

            if (veryVerbose){
                printf("\t\t\tBEFORE: ");
                P_(X.capacity()); P(X);
            }

            LOOP_ASSERT(li, li == X.size());

            mX.clear();

            if (veryVerbose){
                printf("\t\t\tAFTER : ");
                P_(X.capacity()); P(X);
            }

            LOOP_ASSERT(li, 0 == X.size());

            for (size_t i = 0; i < li; ++i) {
                mX.push_back(VALUES[i % NUM_VALUES]);
            }

            LOOP_ASSERT(li, li == X.size());

            if (veryVerbose){
                printf("\t\t\tAFTER SECOND INSERT: ");
                P_(X.capacity()); P(X);
            }
        }
    }
    ASSERT(0 == oa.numMismatches());
    ASSERT(0 == oa.numBlocksInUse());

    // ------------------------------------------------------------------------

    if (verbose) printf("\n\tTesting 'clear' with allocator.\n");
    {
        const size_t NUM_TRIALS = LARGE_SIZE_VALUE;

        for (size_t li = 0; li < NUM_TRIALS; ++li) {
            if (verbose) {
                printf("\t\tOn an object of initial length %llu.\n",
                                                                   (Uint64)li);
            }

            Obj mX(xoa);  const Obj& X = mX;

            for (size_t i = 0; i < li; ++i) {
                mX.push_back(VALUES[i % NUM_VALUES]);
            }

            LOOP_ASSERT(li, li == X.size());

            const Int64 BB = oa.numBlocksTotal();
            const Int64 B  = oa.numBlocksInUse();

            if (veryVerbose) {
                printf("\t\t\tBEFORE: ");
                P_(BB); P_(B); int Cap = (int) X.capacity();P_(Cap);P(X);
            }

            mX.clear();

            const Int64 AA = oa.numBlocksTotal();
            const Int64 A  = oa.numBlocksInUse();

            if (veryVerbose) {
                printf("\t\t\tAFTER : ");
                P_(AA); P_(A); P_(X.capacity()); P(X);
            }

            for (size_t i = 0; i < li; ++i) {
                mX.push_back(VALUES[i % NUM_VALUES]);
            }

            LOOP_ASSERT(li, li == X.size());

            const Int64 CC = oa.numBlocksTotal();
            const Int64 C  = oa.numBlocksInUse();

            if (veryVerbose){
                printf("\t\t\tAFTER SECOND INSERT: ");
                P_(CC); P_(C); P_(X.capacity()); P(X);
            }

            LOOP_ASSERT(li, li == X.size());
        }
    }
    ASSERT(0 == oa.numMismatches());
    ASSERT(0 == oa.numBlocksInUse());

    // ------------------------------------------------------------------------

    if (verbose) printf("\n\tTesting the destructor and exception neutrality "
                        "with allocator.\n");

    if (verbose) printf("\t\tWith 'push_back' only.\n");
    {
        // For each lengths li up to some modest limit:
        //    1. create an object
        //    2. insert { V0, V1, V2, V3, V4, V0, ... }  up to length 'li'
        //    3. verify initial length and contents
        //    4. allow the object to leave scope
        //    5. make sure that the destructor cleans up

        const size_t NUM_TRIALS = LARGE_SIZE_VALUE;
        for (size_t li = 0; li < NUM_TRIALS; ++li) { // i is the length
            if (verbose) {
                printf("\t\t\tOn an object of length %llu.\n", (Uint64)li);
            }

            BSLMA_TESTALLOCATOR_EXCEPTION_TEST_BEGIN(oa) {
                Obj mX(xoa);  const Obj& X = mX;                         // 1.
                for (size_t i = 0; i < li; ++i) {                        // 2.
                    ExceptionProctor<Obj, ALLOC> proctor(&mX, X, L_);
                    mX.push_back(VALUES[i % NUM_VALUES]);
                    proctor.release();
                }

                LOOP_ASSERT(li, li == X.size());                         // 3.
                for (size_t i = 0; i < li; ++i) {
                    LOOP2_ASSERT(li, i, VALUES[i % NUM_VALUES] == X[i]);
                }

            } BSLMA_TESTALLOCATOR_EXCEPTION_TEST_END                     // 4.
            LOOP_ASSERT(li, 0 == oa.numBlocksInUse());                   // 5.
        }
    }
    ASSERT(0 == oa.numMismatches());
    ASSERT(0 == oa.numBlocksInUse());

    if (verbose) printf("\t\tWith 'push_back' and 'clear'\n");
    {
        // For each pair of lengths (i, j) up to some modest limit:
        //    1. create an instance
        //    2. insert V0 values up to a length of i
        //    3. verify initial length and contents
        //    4. clear contents from instance
        //    5. verify length is 0
        //    6. insert { V0, V1, V2, V3, V4, V0, ... }  up to length j
        //    7. verify new length and contents
        //    8. allow the instance to leave scope
        //    9. make sure that the destructor cleans up

        const size_t NUM_TRIALS = LARGE_SIZE_VALUE;
        for (size_t i = 0; i < NUM_TRIALS; ++i) {  // 'i' is first length
            if (verbose) {
                printf("\t\t\tOn an object of initial length %llu.\n",
                                                                    (Uint64)i);
            }

            for (size_t j = 0; j < NUM_TRIALS; ++j) {  // 'j' is second length
                if (veryVerbose) {
                    printf("\t\t\t\tAnd with final length %llu.\n", (Uint64)j);
                }

                BSLMA_TESTALLOCATOR_EXCEPTION_TEST_BEGIN(oa) {
                    size_t k;  // loop index

                    Obj mX(xoa);  const Obj& X = mX;                    // 1.
                    for (k = 0; k < i; ++k) {                           // 2.
                        ExceptionProctor<Obj, ALLOC> proctor(&mX, X, L_);
                        mX.push_back(VALUES[0]);
                        proctor.release();
                    }

                    LOOP2_ASSERT(i, j, i == X.size());                  // 3.
                    for (k = 0; k < i; ++k) {
                        LOOP3_ASSERT(i, j, k, VALUES[0] == X[k]);
                    }

                    mX.clear();                                         // 4.
                    LOOP2_ASSERT(i, j, 0 == X.size());                  // 5.

                    for (k = 0; k < j; ++k) {                           // 6.
                        ExceptionProctor<Obj, ALLOC> proctor(&mX, X, L_);
                        mX.push_back(VALUES[k % NUM_VALUES]);
                        proctor.release();
                    }

                    LOOP2_ASSERT(i, j, j == X.size());                  // 7.
                    for (k = 0; k < j; ++k) {
                        LOOP3_ASSERT(i, j, k, VALUES[k % NUM_VALUES] == X[k]);
                    }

                } BSLMA_TESTALLOCATOR_EXCEPTION_TEST_END                // 8.
                LOOP_ASSERT(i, 0 == oa.numBlocksInUse());               // 9.
            }
        }
    }
    ASSERT(0 == oa.numMismatches());
    ASSERT(0 == oa.numBlocksInUse());
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE, ALLOC>::testCase2()
{
    // ------------------------------------------------------------------------
    // TESTING PRIMARY MANIPULATORS (BOOTSTRAP):
    //   The basic concern is that the default constructor, the destructor,
    //   and the primary manipulators:
    //      - void push_back(T&&);
    //      - void push_front(T&&);
    //      - void clear();
    //   operate as expected.
    //
    // Concerns:
    //: 1 An object created with the default constructor (with or without a
    //:   supplied allocator) has the contractually specified default value.
    //:
    //: 2 If an allocator is NOT supplied to the default constructor, the
    //:   default allocator in effect at the time of construction becomes the
    //:   object allocator for the resulting object.
    //:
    //: 3 If an allocator IS supplied to the default constructor, that
    //:   allocator becomes the object allocator for the resulting object.
    //:
    //: 4 Supplying a null allocator address has the same effect as not
    //:   supplying an allocator.
    //:
    //: 5 Supplying an allocator to the default constructor has no effect on
    //:   subsequent object values.
    //:
    //: 6 Any memory allocation is from the object allocator.
    //:
    //: 7 There is no temporary allocation from any allocator.
    //:
    //: 8 Every object releases any allocated memory at destruction.
    //:
    //: 9 QoI: The default constructor allocates minimal memory.
    //:
    //:10 'push_back' appends an additional element to the object.
    //:
    //:11 'push_front' prepends an additional element to the object.
    //:
    //:12 'clear' properly destroys each contained element.
    //:
    //:13 'clear' does not allocate memory.
    //
    // Plan:
    //: 1 For each value of increasing length, 'L', repeat the following for
    //:   'push_back' and 'push_front':
    //:
    //:   2 Using a loop-based approach, default-construct three distinct
    //:     objects, in turn, but configured differently: (a) without passing
    //:     an allocator, (b) passing a null allocator address explicitly,
    //:     and (c) passing the address of a test allocator distinct from the
    //:     default.  For each of these three iterations:  (C-1..13)
    //:
    //:     1 Create three 'bslma::TestAllocator' objects, and install one as
    //:       the current default allocator (note that a ubiquitous test
    //:       allocator is already installed as the global allocator).
    //:
    //:     2 Use the default constructor to dynamically create an object
    //:       'X', with its object allocator configured appropriately (see
    //:       P-2); use a distinct test allocator for the object's footprint.
    //:
    //:     3 Use the (as yet unproven) 'get_allocator' accessor to ensure that
    //:       its object allocator is properly installed.  (C-2..4)
    //:
    //:     4 Use the appropriate test allocators to verify that minimal memory
    //:       is allocated by the default constructor.  (C-9)
    //:
    //:     5 Use the individual (as yet unproven) salient attribute accessors
    //:       to verify the default-constructed value.  (C-1)
    //:
    //:     6 Append (in the 'push_back' testing loop) or prepend (in the
    //:       'push_front' testing loop) 'L' elements in order of increasing
    //:       value to the container and use the (as yet unproven) basic
    //:       accessors to verify the container has the expected values.
    //:       (C-5..6, 10..11)
    //:
    //:     7 Verify that no temporary memory is allocated from the object
    //:       allocator.  (C-7)
    //:
    //:     8 Invoke 'clear' and verify that the container is empty.  Verify
    //:       that no memory is allocated.  (C-12..13)
    //:
    //:     9 Verify that all object memory is released when the object is
    //:       destroyed.  (C-8)
    //
    // Testing:
    //   deque(const A& a = A());
    //   ~deque();
    //   void clear();
    //   BOOTSTRAP: void push_front(T&& rvalue);
    //   BOOTSTRAP: void push_back(T&& rvalue);
    // ------------------------------------------------------------------------
    const TestValues VALUES;  // contains 52 distinct increasing values

    const size_t MAX_LENGTH = 18;

    if (veryVerbose) printf("\n\tTest loop using 'push_back'.\n");
    {
        for (size_t ti = 0; ti < MAX_LENGTH; ++ti) {
            const size_t LENGTH = ti;

            if (verbose) {
                printf("\nTesting with various allocator configurations.\n");
            }

            for (char cfg = 'a'; cfg <= 'c'; ++cfg) {
                const char CONFIG = cfg;  // how we specify the allocator

                bslma::TestAllocator da("default",   veryVeryVeryVerbose);
                bslma::TestAllocator fa("footprint", veryVeryVeryVerbose);
                bslma::TestAllocator sa("supplied",  veryVeryVeryVerbose);
                ALLOC                xsa(&sa);

                bslma::DefaultAllocatorGuard dag(&da);

                // ------------------------------------------------------------

                if (veryVerbose) printf("\n\tTesting default constructor.\n");

                Obj                  *objPtr;
                bslma::TestAllocator *objAllocatorPtr;

                switch (CONFIG) {
                  case 'a': {
                    objPtr = new (fa) Obj();
                    objAllocatorPtr = &da;
                  } break;
                  case 'b': {
                    objPtr = new (fa) Obj(ALLOC(0));
                    objAllocatorPtr = &da;
                  } break;
                  case 'c': {
                    objPtr = new (fa) Obj(xsa);
                    objAllocatorPtr = &sa;
                  } break;
                  default: {
                    ASSERTV(CONFIG, !"Bad allocator config.");
                    return;                                           // RETURN
                  } break;
                }

                Obj&                   mX = *objPtr;  const Obj& X = mX;
                bslma::TestAllocator&  oa = *objAllocatorPtr;
                bslma::TestAllocator& noa = 'c' != CONFIG ? sa : da;

                // Verify any attribute allocators are installed properly.

                ASSERTV(LENGTH, CONFIG, &oa == X.get_allocator());

                // Verify minimal allocation from the object/non-object
                // allocators.

                ASSERTV(LENGTH, CONFIG, oa.numBlocksTotal(),
                        2 ==  oa.numBlocksTotal());
                ASSERTV(LENGTH, CONFIG, noa.numBlocksTotal(),
                        0 == noa.numBlocksTotal());

                ASSERTV(LENGTH, CONFIG, 0          == X.size());
                ASSERTV(LENGTH, CONFIG, X.cbegin() == X.cend());

                // ------------------------------------------------------------

                if (veryVerbose)
                    printf("\n\tTesting 'push_back' (bootstrap).\n");

                if (0 < LENGTH) {
                    if (verbose) {
                        printf("\t\tOn an object of initial length " ZU ".\n",
                               LENGTH);
                    }

                    for (size_t tj = 0; tj < LENGTH; ++tj) {
                        const int id = TstFacility::getIdentifier(VALUES[tj]);
                        primaryManipulatorBack(&mX, id);
                        ASSERT(tj + 1 == X.size());

                        for (size_t tk = 0; tk <= tj; ++tk) {
                             ASSERTV(LENGTH, tj, tk, CONFIG,
                                     VALUES[tk] == X[tk]);
                        }
                    }

                    ASSERTV(LENGTH, CONFIG, LENGTH == X.size());

                    if (veryVerbose) { printf("\t\t\tAFTER : "); P(X); }
                }

                // ------------------------------------------------------------

                if (veryVerbose)
                    printf("\n\tTesting 'clear' w/'push_back'.\n");
                {
                    const Int64 BB = oa.numBlocksTotal();

                    mX.clear();

                    ASSERTV(LENGTH, CONFIG, 0          == X.size());
                    ASSERTV(LENGTH, CONFIG, X.cbegin() == X.cend());

                    const Int64 AA = oa.numBlocksTotal();
                    const Int64 A  = oa.numBlocksInUse();

                    ASSERTV(LENGTH, CONFIG, BB == AA);
                    ASSERTV(LENGTH, CONFIG, 2  == A);

                    for (size_t tj = 0; tj < LENGTH; ++tj) {
                        const int id = TstFacility::getIdentifier(VALUES[tj]);
                        primaryManipulatorBack(&mX, id);
                        ASSERT(tj + 1 == X.size());

                        for (size_t tk = 0; tk <= tj; ++tk) {
                             ASSERTV(LENGTH, tj, tk, CONFIG,
                                     VALUES[tk] == X[tk]);
                        }
                    }

                    ASSERTV(LENGTH, CONFIG, LENGTH == X.size());
                }

                // ------------------------------------------------------------

                // Reclaim dynamically allocated object under test.

                fa.deleteObject(objPtr);

                // Verify all memory is released on object destruction.

                ASSERTV(LENGTH, CONFIG, da.numBlocksInUse(),
                        0 == da.numBlocksInUse());
                ASSERTV(LENGTH, CONFIG, fa.numBlocksInUse(),
                        0 == fa.numBlocksInUse());
                ASSERTV(LENGTH, CONFIG, sa.numBlocksInUse(),
                        0 == sa.numBlocksInUse());
            }
        }
    }

    if (veryVerbose) printf("\n\tTest loop using 'push_front'.\n");
    {
        for (size_t ti = 0; ti < MAX_LENGTH; ++ti) {
            const size_t LENGTH = ti;

            if (verbose) {
                printf("\nTesting with various allocator configurations.\n");
            }

            for (char cfg = 'a'; cfg <= 'c'; ++cfg) {
                const char CONFIG = cfg;  // how we specify the allocator

                bslma::TestAllocator da("default",   veryVeryVeryVerbose);
                bslma::TestAllocator fa("footprint", veryVeryVeryVerbose);
                bslma::TestAllocator sa("supplied",  veryVeryVeryVerbose);
                ALLOC                xsa(&sa);

                bslma::DefaultAllocatorGuard dag(&da);

                // ------------------------------------------------------------

                if (veryVerbose) printf("\n\tTesting default constructor.\n");

                Obj                  *objPtr;
                bslma::TestAllocator *objAllocatorPtr;

                switch (CONFIG) {
                  case 'a': {
                    objPtr = new (fa) Obj();
                    objAllocatorPtr = &da;
                  } break;
                  case 'b': {
                    objPtr = new (fa) Obj(ALLOC(0));
                    objAllocatorPtr = &da;
                  } break;
                  case 'c': {
                    objPtr = new (fa) Obj(xsa);
                    objAllocatorPtr = &sa;
                  } break;
                  default: {
                    ASSERTV(CONFIG, !"Bad allocator config.");
                    return;                                           // RETURN
                  } break;
                }

                Obj&                   mX = *objPtr;  const Obj& X = mX;
                bslma::TestAllocator&  oa = *objAllocatorPtr;
                bslma::TestAllocator& noa = 'c' != CONFIG ? sa : da;

                // Verify any attribute allocators are installed properly.

                ASSERTV(LENGTH, CONFIG, &oa == X.get_allocator());

                // Verify minimal allocation from the object/non-object
                // allocators.

                ASSERTV(LENGTH, CONFIG, oa.numBlocksTotal(),
                        2 ==  oa.numBlocksTotal());
                ASSERTV(LENGTH, CONFIG, noa.numBlocksTotal(),
                        0 == noa.numBlocksTotal());

                ASSERTV(LENGTH, CONFIG, 0          == X.size());
                ASSERTV(LENGTH, CONFIG, X.cbegin() == X.cend());

                // ------------------------------------------------------------

                if (veryVerbose)
                    printf("\n\tTesting 'push_front' (bootstrap).\n");

                if (0 < LENGTH) {
                    if (verbose) {
                        printf("\t\tOn an object of initial length " ZU ".\n",
                               LENGTH);
                    }

                    for (size_t tj = 0; tj < LENGTH; ++tj) {
                        const int id = TstFacility::getIdentifier(VALUES[tj]);
                        primaryManipulatorFront(&mX, id);
                        ASSERT(tj + 1 == X.size());

                        for (size_t tk = 0; tk <= tj; ++tk) {
                             ASSERTV(LENGTH, tj, tk, CONFIG,
                                     VALUES[tj - tk] == X[tk]);
                        }
                    }

                    ASSERTV(LENGTH, CONFIG, LENGTH == X.size());

                    if (veryVerbose) { printf("\t\t\tAFTER : "); P(X); }
                }

                // ------------------------------------------------------------

                if (veryVerbose)
                    printf("\n\tTesting 'clear' w/'push_front'.\n");
                {
                    const Int64 BB = oa.numBlocksTotal();

                    mX.clear();

                    ASSERTV(LENGTH, CONFIG, 0          == X.size());
                    ASSERTV(LENGTH, CONFIG, X.cbegin() == X.cend());

                    const Int64 AA = oa.numBlocksTotal();
                    const Int64 A  = oa.numBlocksInUse();

                    ASSERTV(LENGTH, CONFIG, BB == AA);
                    ASSERTV(LENGTH, CONFIG, 2  == A);

                    for (size_t tj = 0; tj < LENGTH; ++tj) {
                        const int id = TstFacility::getIdentifier(VALUES[tj]);
                        primaryManipulatorFront(&mX, id);
                        ASSERT(tj + 1 == X.size());

                        for (size_t tk = 0; tk <= tj; ++tk) {
                             ASSERTV(LENGTH, tj, tk, CONFIG,
                                     VALUES[tj - tk] == X[tk]);
                        }
                    }

                    ASSERTV(LENGTH, CONFIG, LENGTH == X.size());
                }

                // ------------------------------------------------------------

                // Reclaim dynamically allocated object under test.

                fa.deleteObject(objPtr);

                // Verify all memory is released on object destruction.

                ASSERTV(LENGTH, CONFIG, da.numBlocksInUse(),
                        0 == da.numBlocksInUse());
                ASSERTV(LENGTH, CONFIG, fa.numBlocksInUse(),
                        0 == fa.numBlocksInUse());
                ASSERTV(LENGTH, CONFIG, sa.numBlocksInUse(),
                        0 == sa.numBlocksInUse());
            }
        }
    }
}

template <class TYPE, class ALLOC>
void TestDriver<TYPE,ALLOC>::testCase1()
{
    // ------------------------------------------------------------------------
    // BREATHING TEST
    //   We want to exercise basic value-semantic functionality.  In
    //   particular we want to demonstrate a base-line level of correct
    //   operation of the following methods and operators:
    //      - default and copy constructors (and also the destructor)
    //      - the assignment operator (including aliasing)
    //      - equality operators: 'operator==' and 'operator!='
    //      - primary manipulators: 'push_back' and 'clear' methods
    //      - basic accessors: 'size' and 'operator[]'
    //   In addition we would like to exercise objects with potentially
    //   different internal organizations representing the same value.
    //
    // Plan:
    //   Create four objects using both the default and copy constructors.
    //   Exercise these objects using primary manipulators, basic
    //   accessors, equality operators, and the assignment operator.
    //   Invoke the primary manipulator [1&5], copy constructor [2&8], and
    //   assignment operator [9&10] in situations where the internal data
    //   (i) does *not* and (ii) *does* have to resize.  Try aliasing with
    //   assignment for a non-empty instance [11] and allow the result to
    //   leave scope, enabling the destructor to assert internal object
    //   invariants.  Display object values frequently in verbose mode:
    //
    // 1) Create an object x1 (default ctor).        { x1:                    }
    // 2) Create a second object x2 (copy from x1).  { x1:  x2:               }
    // 3) Append an element value A to x1).          { x1:A x2:               }
    // 4) Append the same element value A to x2).    { x1:A x2:A              }
    // 5) Append another element value B to x2).     { x1:A x2:AB             }
    // 6) Remove all elements from x1.               { x1:  x2:AB             }
    // 7) Create a third object x3 (default ctor).   { x1:  x2:AB x3:         }
    // 8) Create a forth object x4 (copy of x2).     { x1:  x2:AB x3:   x4:AB }
    // 9) Assign x2 = x1 (non-empty becomes empty).  { x1:  x2:   x3:   x4:AB }
    //10) Assign x3 = x4 (empty becomes non-empty).  { x1:  x2:   x3:AB x4:AB }
    //11) Assign x4 = x4 (aliasing).                 { x1:  x2:   x3:AB x4:AB }
    //
    // Testing:
    //   This "test" *exercises* basic functionality.
    // ------------------------------------------------------------------------

    bslma::TestAllocator oa("object", veryVeryVeryVerbose);

    const TYPE         *values     = 0;
    const TYPE *const&  VALUES     = values;
    const int           NUM_VALUES = getValues(&values);
    (void) NUM_VALUES;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 1) Create an object x1 (default ctor)."
                        "\t\t\t{ x1: }\n");

    Obj mX1(&oa);  const Obj& X1 = mX1;
    if (verbose) { T_;  P(X1); }

    if (verbose) printf("\ta) Check initial state of x1.\n");

    ASSERT(0 == X1.size());

    if (veryVerbose){
        int capacity = (int) X1.capacity();
        T_; T_;
        P(capacity);
    }

    if (verbose) printf("\tb) Try equality operators: x1 <op> x1.\n");
    ASSERT(  X1 == X1 );          ASSERT(!(X1 != X1));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 2) Create a second object x2 (copy from x1)."
                         "\t\t{ x1: x2: }\n");
    Obj mX2(X1, &oa);  const Obj& X2 = mX2;
    if (verbose) { T_;  P(X2); }

    if (verbose) printf(
        "\ta) Check the initial state of x2.\n");
    ASSERT(0 == X2.size());

    if (verbose) printf(
        "\tb) Try equality operators: x2 <op> x1, x2.\n");
    ASSERT(  X2 == X1 );          ASSERT(!(X2 != X1));
    ASSERT(  X2 == X2 );          ASSERT(!(X2 != X2));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 3) Append an element value A to x1)."
                        "\t\t\t{ x1:A x2: }\n");
    mX1.push_back(VALUES[0]);
    if (verbose) { T_;  P(X1); }

    if (verbose) printf(
        "\ta) Check new state of x1.\n");
    ASSERT(1 == X1.size());
    ASSERT(VALUES[0] == X1[0]);

    if (verbose) printf(
        "\tb) Try equality operators: x1 <op> x1, x2.\n");
    ASSERT(  X1 == X1 );          ASSERT(!(X1 != X1));
    ASSERT(!(X1 == X2));          ASSERT(  X1 != X2 );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 4) Append the same element value A to x2)."
                         "\t\t{ x1:A x2:A }\n");
    mX2.push_back(VALUES[0]);
    if (verbose) { T_;  P(X2); }

    if (verbose) printf(
        "\ta) Check new state of x2.\n");
    ASSERT(1 == X2.size());
    ASSERT(VALUES[0] == X2[0]);

    if (verbose) printf(
        "\tb) Try equality operators: x2 <op> x1, x2.\n");
    ASSERT(  X2 == X1 );          ASSERT(!(X2 != X1));
    ASSERT(  X2 == X2 );          ASSERT(!(X2 != X2));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 5) Append another element value B to x2)."
                         "\t\t{ x1:A x2:AB }\n");
    mX2.push_back(VALUES[1]);
    if (verbose) { T_;  P(X2); }

    if (verbose) printf(
        "\ta) Check new state of x2.\n");
    ASSERT(2 == X2.size());
    ASSERT(VALUES[0] == X2[0]);
    ASSERT(VALUES[1] == X2[1]);

    if (verbose) printf(
        "\tb) Try equality operators: x2 <op> x1, x2.\n");
    ASSERT(!(X2 == X1));          ASSERT(  X2 != X1 );
    ASSERT(  X2 == X2 );          ASSERT(!(X2 != X2));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 6) Remove all elements from x1."
                         "\t\t\t{ x1: x2:AB }\n");
    mX1.clear();
    if (verbose) { T_;  P(X1); }

    if (verbose) printf(
        "\ta) Check new state of x1.\n");
    ASSERT(0 == X1.size());

    if (verbose) printf(
        "\tb) Try equality operators: x1 <op> x1, x2.\n");
    ASSERT(  X1 == X1 );          ASSERT(!(X1 != X1));
    ASSERT(!(X1 == X2));          ASSERT(  X1 != X2 );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 7) Create a third object x3 (default ctor)."
                         "\t\t{ x1: x2:AB x3: }\n");

    Obj mX3(&oa);  const Obj& X3 = mX3;
    if (verbose) { T_;  P(X3); }

    if (verbose) printf(
        "\ta) Check new state of x3.\n");
    ASSERT(0 == X3.size());

    if (verbose) printf(
        "\tb) Try equality operators: x3 <op> x1, x2, x3.\n");
    ASSERT(  X3 == X1 );          ASSERT(!(X3 != X1));
    ASSERT(!(X3 == X2));          ASSERT(  X3 != X2 );
    ASSERT(  X3 == X3 );          ASSERT(!(X3 != X3));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 8) Create a forth object x4 (copy of x2)."
                         "\t\t{ x1: x2:AB x3: x4:AB }\n");

    Obj mX4(X2, &oa);  const Obj& X4 = mX4;
    if (verbose) { T_;  P(X4); }

    if (verbose) printf(
        "\ta) Check new state of x4.\n");

    ASSERT(2 == X4.size());
    ASSERT(VALUES[0] == X4[0]);
    ASSERT(VALUES[1] == X4[1]);

    if (verbose) printf(
        "\tb) Try equality operators: x4 <op> x1, x2, x3, x4.\n");
    ASSERT(!(X4 == X1));          ASSERT(  X4 != X1 );
    ASSERT(  X4 == X2 );          ASSERT(!(X4 != X2));
    ASSERT(!(X4 == X3));          ASSERT(  X4 != X3 );
    ASSERT(  X4 == X4 );          ASSERT(!(X4 != X4));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n 9) Assign x2 = x1 (non-empty becomes empty)."
                         "\t\t{ x1: x2: x3: x4:AB }\n");

    mX2 = X1;
    if (verbose) { T_;  P(X2); }

    if (verbose) printf(
        "\ta) Check new state of x2.\n");
    ASSERT(0 == X2.size());

    if (verbose) printf(
        "\tb) Try equality operators: x2 <op> x1, x2, x3, x4.\n");
    ASSERT(  X2 == X1 );          ASSERT(!(X2 != X1));
    ASSERT(  X2 == X2 );          ASSERT(!(X2 != X2));
    ASSERT(  X2 == X3 );          ASSERT(!(X2 != X3));
    ASSERT(!(X2 == X4));          ASSERT(  X2 != X4 );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n10) Assign x3 = x4 (empty becomes non-empty)."
                         "\t\t{ x1: x2: x3:AB x4:AB }\n");

    mX3 = X4;
    if (verbose) { T_;  P(X3); }

    if (verbose) printf(
        "\ta) Check new state of x3.\n");
    ASSERT(2 == X3.size());
    ASSERT(VALUES[0] == X3[0]);
    ASSERT(VALUES[1] == X3[1]);

    if (verbose) printf(
        "\tb) Try equality operators: x3 <op> x1, x2, x3, x4.\n");
    ASSERT(!(X3 == X1));          ASSERT(  X3 != X1 );
    ASSERT(!(X3 == X2));          ASSERT(  X3 != X2 );
    ASSERT(  X3 == X3 );          ASSERT(!(X3 != X3));
    ASSERT(  X3 == X4 );          ASSERT(!(X3 != X4));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (verbose) printf("\n11) Assign x4 = x4 (aliasing)."
                         "\t\t\t\t{ x1: x2: x3:AB x4:AB }\n");

    mX4 = X4;
    if (verbose) { T_;  P(X4); }

    if (verbose) printf(
        "\ta) Check new state of x4.\n");
    ASSERT(2 == X4.size());
    ASSERT(VALUES[0] == X4[0]);
    ASSERT(VALUES[1] == X4[1]);

    if (verbose)
        printf("\tb) Try equality operators: x4 <op> x1, x2, x3, x4.\n");
    ASSERT(!(X4 == X1));          ASSERT(  X4 != X1 );
    ASSERT(!(X4 == X2));          ASSERT(  X4 != X2 );
    ASSERT(  X4 == X3 );          ASSERT(!(X4 != X3));
    ASSERT(  X4 == X4 );          ASSERT(!(X4 != X4));
}

// ============================================================================
//                              MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;

                verbose = argc > 2;
            veryVerbose = argc > 3;
        veryVeryVerbose = argc > 4;
    veryVeryVeryVerbose = argc > 5;

    bslma::TestAllocator defaultAllocator("default", veryVeryVeryVerbose);
    bslma::DefaultAllocatorGuard dag(&defaultAllocator);

    bslma::TestAllocator globalAllocator("global", veryVeryVeryVerbose);
    bslma::Default::setGlobalAllocator(&globalAllocator);

    // Verify sanity of DEFAULT_DATA

    {
        typedef TestDriver<int> TD;

        const size_t NUM_DATA                  = DEFAULT_NUM_DATA;
        const DefaultDataRow (&DATA)[NUM_DATA] = DEFAULT_DATA;

        bslma::TestAllocator sa("scratch");

        TD::Obj mX(&sa); const TD::Obj& X = mX;
        TD::Obj mY(&sa); const TD::Obj& Y = mY;

        for (unsigned ii = 0; ii < NUM_DATA; ++ii) {
            const int   INDEX1  = DATA[ii].d_index;
            const char *SPEC1   = DATA[ii].d_spec;
            const char *RESULT1 = DATA[ii].d_results;

            ASSERT(!ii || DATA[ii-1].d_index <= INDEX1);
            ASSERT(strlen(SPEC1) == strlen(RESULT1));

            mX.clear();
            TD::gg(&mX, SPEC1);
            ASSERTV(ii, SPEC1, RESULT1, verifySpec(X, RESULT1));

            mY.clear();
            TD::gg(&mY, RESULT1);
            ASSERTV(ii, SPEC1, RESULT1, X == Y);

            for (unsigned jj = 0; jj < NUM_DATA; ++jj) {
                ASSERT((INDEX1 == DATA[jj].d_index) ==
                                    !std::strcmp(RESULT1, DATA[jj].d_results));
            }
        }
    }

    printf("TEST " __FILE__ " CASE %d\n", test);

    switch (test) { case 0:  // Zero is always the leading case.
      case 34: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE 2
        //
        // Concerns:
        //   Demonstrate properties of deque with an abstract demonstration
        //   showing numerous properties of the container.
        // --------------------------------------------------------------------

        if (verbose) printf("\nUSAGE EXAMPLE 2\n"
                              "==============\n");
//..
// Then, we define a type 'MyDeque', which is a 'deque' containing 'int's, and
// a forward iterator for it:
//..
        typedef bsl::deque<int> MyDeque;
        typedef MyDeque::iterator It;
//..
// Next, we observe that an iterator to a 'deque', unlike an iterator to a
// 'vector', is not a pointer:
//..
        ASSERT(! (bsl::is_same<int *, It>::value));
        ASSERT(! bsl::is_pointer<It>::value);
//..
// Then, we create an allocator to use for the 'deque', and some test data to
// load into it:
//..
        bslma::TestAllocator ta;

        int DATA1[] = {   0,  2 , 4,  6,  8 };
        int DATA2[] = { -10, -8, -6, -4, -2 };
        enum { NUM_DATA =  sizeof DATA1 / sizeof *DATA1 };
        ASSERT(NUM_DATA == sizeof DATA2 / sizeof *DATA2 );
//..
// Next, we create a 'deque' 'd' and initialize it at construction to have the
// 5 elements of 'DATA1', and check our invariants for it.
//..
        MyDeque d(DATA1 + 0, DATA1 + NUM_DATA, &ta);

        checkInvariants(d);
//..
// Then, we verify that it has as many elements as we expect:
//..
        ASSERT(NUM_DATA == d.size());
        ASSERT(NUM_DATA == d.end()  - d.begin());
        ASSERT(NUM_DATA == d.rend() - d.rbegin());
//..
// Next, we examine the first and last elements in a number of ways:
//..
        ASSERT(0 == d.front());
        ASSERT(0 == d[0]);
        ASSERT(0 == *d.begin());
        ASSERT(0 == *(d.rbegin() + NUM_DATA - 1));

        ASSERT(8 == d.back());
        ASSERT(8 == d[NUM_DATA - 1]);
        ASSERT(8 == *d.rbegin());
        ASSERT(8 == *(d.begin()  + NUM_DATA - 1));
//..
// Then, we verify the expected value of all the elements:
//..
        for (int i = 0; i < NUM_DATA; ++i) {
            ASSERT(2 * i == d[i]);
            ASSERT(2 * i == *(d.begin() + i));
        }
//..
// Next we create a second empty 'deque' and check invariants on it:
//..
        MyDeque d2(&ta);

        checkInvariants(d2);
//..
// Then, we assign the value of the first 'deque' to the second, check
// invariants, and verify they are equal:
//..
        d2 = d;
        checkInvariants(d2);
        ASSERT(d == d2);
//..
// Next, we assert the elements of 'DATA2' at the beginning of the second
// 'deque', and check invariants:
//..
        d2.insert(d2.begin(), DATA2 + 0, DATA2 + NUM_DATA);

        checkInvariants(d2);
//..
// Then, the two 'deque's should now be unequal.  Record whether 'd2' is now
// less than 'd':
//..
        ASSERT(d != d2);
        const bool lesserFlag = d2 < d;
        ASSERT((d2 >= d) != lesserFlag);
        ASSERT((d2 >  d) != lesserFlag);
//..
// Next, we swap 'd2' and 'd', check invariants of both 'deque's after the
// swap, and verify that the equality comparisons and size are as expected:
//..
        d.swap(d2);

        checkInvariants(d);
        checkInvariants(d2);

        ASSERT(d != d2);
        ASSERT((d <  d2) == lesserFlag);
        ASSERT((d >= d2) != lesserFlag);
        ASSERT((d >  d2) != lesserFlag);

        ASSERT(2 * NUM_DATA == d.size());
//..
// Note that had we had any iterators to 'd2', both the 'insert' and the 'swap'
// would have invalidated them.
//
// Then, we verify that 'front' and 'back' are as expected:
//..
        ASSERT(-10 == d.front());
        ASSERT(-10 == d[0]);

        ASSERT(  8 == d.back());
        ASSERT(  8 == d[2 * NUM_DATA - 1]);
//..
// Next, we verify all elements in the 'deque':
//..
        for (int i = 0; i < (int) d.size(); ++i) {
            ASSERT(-10 + 2 * i == d[i]);
            ASSERT(-10 + 2 * i == *(d.begin() + i));
            ASSERT(  8 - 2 * i == *(d.end() - 1 - i));
            ASSERT(  8 - 2 * i == *(d.rbegin() + i));
        }
//..
// Then, we push more elements to the front & back:
//
// Note that if we had any iterators to 'd', these pushes would invalidate them
//..
        d.push_back(10);
        d.push_front(-12);
//..
// Next, using iterators, we verify the sequence of all elements in 'd'
//..
        const It endMinus1 = d.end() - 1;
        for (It it = d.begin(); it < endMinus1; ++it) {
            ASSERT(*it + 2 == *(it + 1));
        }
//..
// Then, we create an iterator and set it to point to the element containing
// '0', and take a reference to the same element:
//..
        It zeroIt = d.begin();
        while (*zeroIt < 0) {
            ++zeroIt;
        }
        ASSERT(0 == *zeroIt);

        ASSERT((bsl::is_same<int&, MyDeque::reference>::value));

        MyDeque::reference zeroRef = *zeroIt;

        ASSERT(0 == zeroRef);
//..
// Next, create an iterator and a reference pointing to 'd.back()':
//..
        It backIt = d.begin() + d.size() - 1;

        ASSERT(10 == *backIt);

        int& backRef = *backIt;
//..
// Then, we pop the front element off 'd'.  Note that 'pop_front' and
// 'pop_back' do not invalidate iterators or references to surviving elements:
//..
        d.pop_front();

        checkInvariants(d);
//..
// Now, we 'erase' all elements before the element containing '0'.  Note that
// 'erase' at either end of the deque does not invalidate iterators or
// references to surviving elements
//..
        d.erase(d.begin(), zeroIt);

        checkInvariants(d);
//..
// Finally, we use our (still valid) iterators and references to verify that
// the contents of 'd' are as expected:
//..
        ASSERT( 0 == *zeroIt);
        ASSERT( 0 == zeroRef);

        ASSERT(10 == *backIt);
        ASSERT(10 == backRef);

        ASSERT(&*zeroIt == &d.front());
        ASSERT(&zeroRef == &d.front());

        ASSERT(&*backIt == &d.back());
        ASSERT(&backRef == &d.back());

        ASSERT( 0 == d.front());
        ASSERT( 0 == d[0]);
        ASSERT(10 == d.back());
        ASSERT(10 == d[5]);

        for (unsigned u = 0; u < d.size(); ++u) {
            ASSERT(2 * (int) u == d[u]);
        }
//..
      } break;
      case 33: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE 1
        //
        // Concerns:
        //   Demonstrate a context in which a 'deque' might be useful.
        // --------------------------------------------------------------------

        if (verbose) printf("USAGE EXAMPLE 1\n"
                            "===============\n");

        // Then, we define (and default construct) our laundry queue:

        bslma::TestAllocator ta;
        LaundryQueue q(&ta);

        // Next, we add a few customers:

        q.push("Steve Firm");
        q.push("Sally Johnson");
        q.push("Joe Sampson");

        // Then, the following customer bribes the merchant and gets pushed to
        // the front of the queue:

        q.expeditedPush("Dirty Dan");

        // Next, a couple of more regular customers are pushed:

        q.push("Wally Walters");
        q.push("Fred Flintstone");

        // Then, we see who is now next up to have their laundry done, and
        // verify that it is "Dirty Dan":

        bsl::string nxt = q.next();
        ASSERT("Dirty Dan" == nxt);

        // Next, we verify that "Dirty Dan" is no longer in the queue:

        ASSERT(! q.find("Dirty Dan"));

        // Then, we verify that "Sally Johnson" is still in the queue:

        ASSERT(  q.find("Sally Johnson"));

        // Now, we iterate, printing out the names of people whose laundry
        // remains to be done:

        while (true) {
            bsl::string customerName = q.next();
            if ("(* empty *)" == customerName) {
                break;
            }

            printf("Next: %s\n", customerName.c_str());
        }

        // Finally, we observe that these names are printed, in the following
        // order:
        //..
        //  Next: Steve Firm
        //  Next: Sally Johnson
        //  Next: Joe Sampson
        //  Next: Wally Walters
        //  Next: Fred Flintstone
        //..

      } break;
      case 32: // falls through
      case 31: // falls through
      case 30: // falls through
      case 29: // falls through
      case 28: // falls through
      case 27: // falls through
      case 26: // falls through
      case 25: // falls through
      case 24: // falls through
      case 23: // falls through
      case 22: // falls through
      case 21: // falls through
      case 20: // falls through
      case 19: // falls through
      case 18: // falls through
      case 17: // falls through
      case 16: // falls through
      case 15: // falls through
      case 14: // falls through
      case 13: // falls through
      case 12: {
        if (verbose) printf(
                  "\nTEST CASE %d IS DELEGATED TO 'bslstl_deque_test.t.cpp'"
                  "\n======================================================\n",
                  test);
      } break;
      case 11: {
        // --------------------------------------------------------------------
        // TESTING ALLOCATOR-RELATED CONCERNS
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) printf("TESTING ALLOCATOR-RELATED CONCERNS\n"
                            "==================================\n");

        TestDriver<TTA>::testCase11();

      } break;
      case 10: {
        // --------------------------------------------------------------------
        // TESTING BDEX STREAMING FUNCTIONALITY
        // --------------------------------------------------------------------

        if (verbose) printf("TESTING BDEX STREAMING FUNCTIONALITY\n"
                            "====================================\n");

        if (verbose)
            printf("There is no BDEX streaming for this component.\n");

      } break;
      case 9: {
        // --------------------------------------------------------------------
        // TESTING COPY-ASSIGNMENT OPERATOR
        //   Now that we can generate many values for our test objects, and
        //   compare results of assignments, we can test the assignment
        //   operator.    This is achieved by the 'testCase9' class method of
        //   the test driver template, instantiated for the basic test type.
        //   See that function for a list of concerns and a test plan.
        //
        // Testing:
        //   deque& operator=(const deque& rhs);
        // --------------------------------------------------------------------

        if (verbose) printf("\nTESTING COPY-ASSIGNMENT OPERATOR"
                            "\n================================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase9,
                      char,
                      TTA,
                      SmlTT,
                      MedTT,
                      LrgTT,
                      BMTTA,
                      BCTT);

        // 'propagate_on_container_copy_assignment' testing

        RUN_EACH_TYPE(TestDriver,
                      testCase9_propagate_on_container_copy_assignment,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::MovableTestType,
                      bsltf::MovableAllocTestType);

      } break;
      case 8: {
        // --------------------------------------------------------------------
        // TESTING GENERATOR FUNCTION G
        //
        // Concerns:
        //: 1 None.
        //
        // Plan:
        //: 1 None.  (C-1)
        //
        // Testing:
        //   Obj g(const char *spec);
        // --------------------------------------------------------------------

        if (verbose) printf("TESTING GENERATOR FUNCTION G\n"
                            "============================\n");

        if (verbose) printf("\nThis test has been disabled.\n");

      } break;
      case 7: {
        // --------------------------------------------------------------------
        // TESTING COPY CONSTRUCTORS
        //   Having now full confidence in 'operator==', we can use it
        //   to test that copy constructors preserve the notion of
        //   value.  This is achieved by the 'testCase7' class method of the
        //   test driver template, instantiated for the basic test type.  See
        //   that function for a list of concerns and a test plan.
        //
        // Testing:
        //   deque(const deque& original, const A& = A());
        // --------------------------------------------------------------------

        if (verbose) printf("TESTING COPY CONSTRUCTORS\n"
                            "\n=======================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase7,
                      char,
                      TTA,
                      SmlTT,
                      MedTT,
                      LrgTT,
                      BMTTA,
                      BCTT);

        typedef bsltf::StdAllocTestType<bsl::allocator<int> > AllocInt;
        StdBslmaTestDriver<AllocInt>::testCase7();

        // 'select_on_container_copy_construction' testing

        if (verbose) printf("\nCOPY CONSTRUCTOR: ALLOCATOR PROPAGATION"
                            "\n=======================================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase7_select_on_container_copy_construction,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::MovableTestType,
                      bsltf::MovableAllocTestType);

      } break;
      case 6: {
        // --------------------------------------------------------------------
        // TESTING EQUALITY COMPARISON OPERATORS
        //   Since 'operators==' is implemented in terms of basic accessors,
        //   it is sufficient to verify only that a difference in value of any
        //   one basic accessor for any two given objects implies inequality.
        //   However, to test that no other internal state information is
        //   being considered, we want also to verify that 'operator==' reports
        //   true when applied to any two objects whose internal
        //   representations may be different yet still represent the same
        //   (logical) value.  This is achieved by the 'testCase6' class
        //   method of the test driver template, instantiated for the basic
        //   test type.  See that function for a list of concerns and a test
        //   plan.
        //
        // Testing:
        //   bool operator==(const deque& lhs, const deque& rhs);
        //   bool operator!=(const deque& lhs, const deque& rhs);
        // --------------------------------------------------------------------

        if (verbose) printf("\nTESTING EQUALITY COMPARISON OPERATORS"
                            "\n=====================================\n");

        if (verbose) printf("\n... with 'char'.\n");
        TestDriver<char>::testCase6();

        if (verbose) printf("\n... with 'TestTypeAlloc'.\n");
        TestDriver<TTA>::testCase6();

        RUN_EACH_TYPE(StdBslmaTestDriver,
                      testCase6,
                      bsltf::StdAllocTestType<bsl::allocator<int> >,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_PRIMITIVE);

      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING OUTPUT (<<) OPERATOR
        // --------------------------------------------------------------------

        if (verbose) printf("TESTING OUTPUT (<<) OPERATOR\n"
                            "============================\n");

        if (verbose)
            printf("There is no output operator for this component.\n");

      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING BASIC ACCESSORS
        //   Having implemented an effective generation mechanism, we now would
        //   like to test thoroughly the basic accessor methods:
        //     - size() const
        //     - operator[](size_type) const
        //   Also, we want to ensure that various internal state
        //   representations for a given value produce identical results.  This
        //   is achieved by the 'testCase4' class method of the test driver
        //   template, instantiated for the basic test type.  See that function
        //   for a list of concerns and a test plan.
        //
        // Testing:
        //   allocator_type get_allocator() const;
        //   size_type size() const;
        //   const_reference operator[](size_type position) const;
        //   const_reference at(size_type position) const;
        // --------------------------------------------------------------------

        if (verbose) printf("TESTING BASIC ACCESSORS\n"
                            "=======================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase4,
                      char,
                      TTA,
                      SmlTT,
                      MedTT,
                      LrgTT,
                      BMTTA,
                      BCTT);

      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING GENERATOR FUNCTIONS
        //   This is achieved by the 'testCase3' class method of the test
        //   driver template, instantiated for the basic test type.  See that
        //   function for a list of concerns and a test plan.
        //
        // Testing:
        //   int ggg(Obj *object, const char *spec, bool vF = true);
        //   Obj& gg(Obj *object, const char *spec);
        // --------------------------------------------------------------------

        if (verbose) printf("TESTING GENERATOR FUNCTIONS\n"
                            "===========================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase3,
                      char,
                      TTA,
                      SmlTT,
                      MedTT,
                      LrgTT,
                      BMTTA,
                      BCTT);

        typedef bsltf::StdAllocTestType<bsl::allocator<int> > AllocInt;
        StdBslmaTestDriver<AllocInt>::testCase3();

      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING PRIMARY MANIPULATORS (BOOTSTRAP)
        //   We want to ensure that the primary manipulators:
        //      - push_back(T&&)     (black-box)
        //      - push_front(T&&)    (black-box)
        //      - clear              (white-box)
        //   operate as expected.  This is achieved by the 'testCase2' class
        //   method of the test driver template, instantiated for the basic
        //   test types.  See that function for a list of concerns and a test
        //   plan.
        //
        // Testing:
        //   deque(const A& a = A());
        //   ~deque();
        //   void clear();
        //   BOOTSTRAP: void push_front(T&& rvalue);
        //   BOOTSTRAP: void push_back(T&& rvalue);
        // --------------------------------------------------------------------

        if (verbose) printf("TESTING PRIMARY MANIPULATORS (BOOTSTRAP)\n"
                            "========================================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase2,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::MovableTestType,
                      bsltf::MovableAllocTestType,
                      bsltf::MoveOnlyAllocTestType);

        RUN_EACH_TYPE(StdBslmaTestDriver,
                      testCase2,
                      bsltf::StdAllocTestType<bsl::allocator<int> >,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_PRIMITIVE);

        RUN_EACH_TYPE(TestDriver,
                      testCase2a,
                      char,
                      TTA,
                      SmlTT,
                      MedTT,
                      LrgTT,
                      BMTTA,
                      BCTT);

        typedef bsltf::StdAllocTestType<bsl::allocator<int> > AllocInt;
        StdBslmaTestDriver<AllocInt>::testCase2a();

      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //   We want to exercise basic value-semantic functionality.  This is
        //   achieved by the 'testCase1' class method of the test driver
        //   template, instantiated for a few basic test types.  See that
        //   function for a list of concerns and a test plan.  In addition, we
        //   want to make sure that we can use any standard-compliant
        //   allocator, including not necessarily rebound to the same type as
        //   the contained element, and that various manipulators and accessors
        //   work as expected in normal operation.
        //
        // Testing:
        //   BREATHING TEST
        // --------------------------------------------------------------------

        if (verbose) printf("BREATHING TEST\n"
                            "==============\n");

        if (verbose) printf("\nStandard value-semantic test.\n");

        if (verbose) printf("\n\t... with 'char' type.\n");
        TestDriver<char>::testCase1();

        if (verbose) printf("\n\t... with 'TestTypeAlloc'.\n");
        TestDriver<TTA>::testCase1();

        if (verbose) printf("\n... with 'SmallTestTypeNoAlloc'.\n");
        TestDriver<SmlTT>::testCase1();

        if (verbose) printf("\n... with 'MediumTestTypeNoAlloc'.\n");
        TestDriver<MedTT>::testCase1();

        if (verbose) printf("\n... with 'LargeTestTypeNoAlloc'.\n");
        TestDriver<LrgTT>::testCase1();

        if (verbose) printf("\n\t... with 'BitwiseMoveableTestTypeAlloc'.\n");
        TestDriver<BMTTA>::testCase1();

        if (verbose)
            printf("\n\t... with 'BitwiseCopyableTestTypeNoAlloc'.\n");
        TestDriver<BCTT>::testCase1();

        if (verbose) printf("\nAdditional tests: allocators.\n");
        {
            bslma::TestAllocator oa("object", veryVeryVeryVerbose);

            bsl::allocator<int> zza(&oa);

            // Disabled: in order to use 'bsl::deque', we disabled this very
            // infrequent usage for 'bsl::deque' (it will be flagged by
            // 'BSLMF_ASSERT'):
            //..
            //  bsl::deque<int, bsl::allocator<void *> > zz1, zz2(zza);
            //..

            if (verbose) printf("\nAdditional tests: miscellaneous.\n");

            bsl::deque<char> myDeq(5, 'a');
            bsl::deque<char>::const_iterator citer;
            ASSERT(5 == myDeq.size());
            ASSERT(5 == myDeq.end() - myDeq.begin());
            ASSERT(5 == myDeq.cend() - myDeq.cbegin());
            for (citer = myDeq.begin(); citer != myDeq.end(); ++citer) {
                LOOP_ASSERT(*citer, 'a' == *citer);
            }
            if (verbose) P(myDeq);

            myDeq.insert(myDeq.begin(), 'z');
            ASSERT(6 == myDeq.size());
            ASSERT(6 == myDeq.end() - myDeq.begin());
            ASSERT(6 == myDeq.cend() - myDeq.cbegin());
            ASSERT('z' == myDeq[0]);
            for (citer = myDeq.begin() + 1; citer != myDeq.end(); ++citer) {
                ASSERT('a' == *citer);
            }
            if (verbose) P(myDeq);

            myDeq.erase(myDeq.begin() + 2, myDeq.begin() + 4);
            ASSERT(4 == myDeq.size());
            ASSERT(4 == myDeq.end() - myDeq.begin());
            ASSERT(4 == myDeq.cend() - myDeq.cbegin());
            ASSERT('z' == myDeq[0]);
            for (citer = myDeq.begin() + 1; citer != myDeq.end(); ++citer) {
                ASSERT('a' == *citer);
            }
            if (verbose) P(myDeq);

            bsl::deque<bsl::deque<char> > vv;
            vv.push_back(myDeq);
            if (verbose) P(myDeq);

            if (verbose) printf("\nAdditional tests: traits.\n");

            ASSERT(bslmf::IsBitwiseMoveable<bsl::deque<char> >::value);
            ASSERT(bslmf::IsBitwiseMoveable<bsl::deque<TTA> >::value);
            ASSERT(
               bslmf::IsBitwiseMoveable<bsl::deque<bsl::deque<int> > >::value);
        }

#ifdef BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS
        if (verbose) printf("\nAdditional tests: initializer lists.\n");
        {
            ASSERT((0 == []() -> bsl::deque<int> { return {}; }().size()));
            ASSERT((1 == []() -> bsl::deque<int> { return {1}; }().size()));
            ASSERT((3 == []() -> bsl::deque<int> {
                return {3, 1, 3};
            }().size()));
        }
#endif

      } break;
      default: {
        fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }

    ASSERTV(defaultAllocator.numBlocksInUse(),
            0 == defaultAllocator.numBlocksInUse());

    // CONCERN: In no case does memory come from the global allocator.
    ASSERTV(globalAllocator.numBlocksTotal(),
            0 == globalAllocator.numBlocksTotal());

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
