#define LD_MACROS_ENABLED
#include "./lodashpp.hpp"
#include <iostream>


using lodashpp::_;
using std::cout;
using std::endl;
using std::string;


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

struct Person {
  int id;
  string name;
  std::vector<int> childIds;

  auto getName() { return name; }

  Person( int id_, string name_, std::vector<int> const& childIds_ )
  : id(id_), name(name_), childIds(childIds_) {}

  Person( Person&& rhs )
  : id(rhs.id), name(rhs.name), childIds(std::move(rhs.childIds)) {}

  //! NOTE: we do this to make sure that we're not performing unnecessary copies along the pipeline.
  Person( Person const& ) = delete;
};



// ---- tests ----

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

void test_pluck() {
  std::list<Person> people;
  people.push_back(Person{1, "Michael", {2,3}});
  people.push_back({ 2, "Jessica", {4} });
  people.push_back({ 3, "Edward", {} });
  people.push_back({ 4, "Jonathan", {} });

  test(__LINE__, _(people).pluck(&Person::id).vector(), {1,2,3,4});
  test(__LINE__, _(people).pluck(&Person::getName).vector(),
    {"Michael","Jessica","Edward","Jonathan"});
  test(__LINE__, _(people).map(LD_PROP(id)).vector(), {1,2,3,4});
  test(__LINE__, _(people).map(LD_PROP(name)).vector(), {"Michael","Jessica","Edward","Jonathan"});
  test(__LINE__, _(people).pluck(LD_PROP(id)).vector(), {1,2,3,4});
  test(__LINE__, _(people).pluck(&Person::id).map([](int v) { return v*2; }).vector(), {2,4,6,8});
}



int main() {

  // run the tests
  test_stl_drain();
  test_pluck();

  // show the results
  cout << endl;
  if ( results_fail ) {
    cout << "\e[31m" << results_fail << " FAILED; " << results_pass << " PASSED\e[0m" << endl;
  } else {
    cout << "\e[32mALL " << results_pass << " TESTS PASSED\e[0m" << endl;
  }

  return 0;
}
