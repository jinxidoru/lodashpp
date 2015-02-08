#include "./lodashpp.hpp"
#include <iostream>

using lodashpp::_;
using std::cout;
using std::endl;

namespace std {

  template <class FwdIter>
  std::ostream& outp( std::ostream& out, FwdIter first, FwdIter last ) {
    out << '{';
    if ( first != last ) {
      out << *first;
      for ( ++first; first != last; ++first ) {
        out << ", " << *first;
      }
    }
    return out << '}';
  }

  template <class T>
  std::ostream& operator<<( std::ostream& out, std::vector<T> const& coll ) {
    return outp(out,coll.begin(),coll.end());
  }

  template <class T>
  std::ostream& operator<<( std::ostream& out, std::list<T> const& coll ) {
    return outp(out,coll.begin(),coll.end());
  }
}

static int results_pass = 0;
static int results_fail = 0;

template <class T>
bool test( int lineNo, T const& a, T const& b) {
  if ( a == b ) {
    results_pass++;
    return true;
  } else {
    results_fail++;
    cout << "FAILURE ON LINE #" << lineNo << endl;
    cout << a << endl;
    cout << " DOES NOT MATCH " << endl;
    cout << b << endl;
    return false;
  }
}


void test_stl_drain() {
  const std::vector<int> nums = { 1, 2, 3, 4 };
  auto gen = _(nums)
    .map([](auto&& v) { return v*.2; })
    .map([](auto&& v) -> int { return v*40; });

  // run some tests
  test(__LINE__, nums, nums);
  test(__LINE__, gen.vector(), {8,16,24,32});
  test(__LINE__, gen.list(), {8,16,24,32});
  test(__LINE__, (std::vector<int64_t>)gen, {8,16,24,32});
}



int main() {

  // run the tests
  test_stl_drain();

  // show the results
  cout << endl;
  if ( results_fail ) {
    cout << "\e[31m" << results_fail << " FAILED; " << results_pass << " PASSED\e[0m" << endl;
  } else {
    cout << "\e[32mALL " << results_pass << " TESTS PASSED\e[0m" << endl;
  }

  return 0;
}
