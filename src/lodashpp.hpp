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
#include <vector>


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
    typedef typename std::remove_reference_t<std::remove_cv_t<T>> BaseValueType;

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

    auto vtr() {
      std::vector<BaseValueType> rval;
      m_gen([&](auto&& v) {
        rval.push_back(v);
        return true;
      });
      return rval;
    }

    template <class Fn>
    void each( Fn&& fn ) {
      m_gen(fn);
    }

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
