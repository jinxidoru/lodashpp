/*
The MIT License (MIT)

Copyright (c) 2015 Michael Paul Bailey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __LODASHPP_HPP__
#define __LODASHPP_HPP__

#include <type_traits>
#include <functional>
#include <iterator>
#include <vector>
#include <list>

// ---- macros ----

#if defined(LD_MACROS_ENABLED)
  #define LD_PROP(name)     [](auto&& v) { return v.name; }
  #define LD_PROP_PTR(name) [](auto&& v) { return v->name; }
#endif


namespace lodashpp {

  //! This is the main class that is used by each step of lodashpp.  With each applied mutation
  //! (e.g. map, pluck), a new generator is created which links to the previous stages of the
  //! pipeline.  Most importantly, the Generator class is the class that encapsulates all of the
  //! mutation functions.  They are called on a Generator which then results in either another
  //! Generator instance or the end of the pipeline.
  //!
  //! Fundamentally, a generator is a collection of items behind a function.  The function accepts
  //! a single argument lambda.  When executed, the generator will iterate over each item in the
  //! collection, calling the lambda for each until the end of the collection or the lambda returns
  //! false.
  template <class T, class GenFn>
  struct Generator {
    typedef T ValueType;
    typedef typename std::remove_reference_t<std::remove_cv_t<T>> BaseValue;

    Generator( GenFn&& gen ) : m_gen(gen) {}

    template <class Fn>
    void operator()( Fn&& fn ) {
      m_gen(fn);
    }

    template <class Fn>
    auto map( Fn&& fn ) {
      typedef decltype(fn( std::function<T()>{}() )) NextType;

      return link<NextType>([=](auto&& next) {
        m_gen([=](auto&& v) {
          return next(fn(v));
        });
      });
    }

    //! Returns the given field from each item.
    template <class U, class C> auto pluck( U C::*fptr ) {
      return map([=](auto&& v) { return v.*fptr; });
    }

    //! Returns the result of the given function from each item.
    template <class U, class C> auto pluck( U (C::*fptr)() ) {
      return map([=](auto&& v) { return (v.*fptr)(); });
    }

    //! Alias for map().  This is typically used with LD_PROP().
    template <class Fn> auto pluck( Fn&& fn ) { return map(fn); }

    //! Returns true if every item evaluates to true.
    template <class Fn> bool every( Fn&& fn ) {
      bool rval = true;
      m_gen([&](auto&& v) {
        if ( !fn(v) ) {
          rval = false;
          return false;
        } else {
          return true;
        }
      });
      return rval;
    }

    //! Same as each(), but does not stop the pipeline.
    template <class Fn> auto peek( Fn&& fn ) {
      return link<T>([=](auto&& next) {
        m_gen([=](auto&& v) {
          fn(v);
          return next(v);
        });
      });
    }


    // ---- aliases ----
    #define LD_ALIAS_FN(from,to) \
      template <class... Ts> auto from(Ts&&... args) { return to(std::forward<Ts>(args)...); }

    LD_ALIAS_FN(all,every);

    #undef LD_ALIAS_FN


    // ---- drain functions ----
    //! Call this function for each item.
    template <class Fn> void each( Fn&& fn ) {
      m_gen([&](auto&& v) { fn(v); return true; });
    }

    //! Drain all of the items into the provided output iterator.
    template <class OutIter> OutIter drain( OutIter out ) {
      each([&](auto&& v) {
        *out = v;
        ++out;
      });
      return out;
    }

    //! Convert to a vector.
    template <class U> operator std::vector<U>() {
      std::vector<U> rval;
      drain(std::back_inserter(rval));
      return rval;
    }

    //! Convert to a list.
    template <class U> operator std::list<U>() {
      std::list<U> rval;
      drain(std::back_inserter(rval));
      return rval;
    }


    // ---- drain aliases ----
    std::vector<BaseValue> vector() { return *this; }
    std::list<BaseValue> list() { return *this; }


  private:
    GenFn m_gen;

    //! Create another link in the chain
    template <class U, class Fn>
    auto link( Fn&& fn ) {
      return Generator<U,Fn>(std::move(fn));
    }
  };

  //! This class provides a simple method of starting a lodashpp pipeline.  It is made to resemble
  //! the calling structure of lodash.js and underscore.js.
  struct LoDash {
    
    //! Create a generator that iterates over the contents of a collection.
    template <class T>
    auto operator()( T& coll, typename T::iterator* = nullptr ) {
      return (*this)(coll.begin(), coll.end());
    }

    //! Create a generator that iterates over the contents of an iterator range.
    template <class FwdIter>
    auto operator()( FwdIter first, FwdIter last ) {
      return from<decltype(*first)>([=](auto&& next) {
        for ( auto iter=first; iter!=last && next(*iter); ++iter ) {}
      });
    }


  private:

    //! This is the main function that creates a generator from a given lambda.  The `T` type is
    //! what defines the type that is provided by this current step.
    template <class T, class Fn>
    auto from( Fn&& fn ) {
      return Generator<T,Fn>(std::move(fn));
    }
  };

  static LoDash _;
}

#endif
