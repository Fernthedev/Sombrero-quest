#pragma once

#include "linq.hpp"
#include <algorithm>

// Holds the functional wrappers for performing transformations on ranges
namespace Sombrero::Linq::Functional {

    template<class F>
    struct Where {
        F function;
        explicit Where(F&& func) : function(func) {}
        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::WhereIterable<decltype(range.begin()), F>(range, std::forward<F>(function));
        }
    };
    template<class F>
    struct Select {
        F function;
        explicit Select(F&& func) : function(func) {}
        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::SelectIterable<decltype(range.begin()), F, decltype(function(*range.begin()))>(range, std::forward<F>(function));
        }
    };

    template<class F>
    struct First {
        F function;
        explicit First(F&& func) : function(func) {}

        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::First(std::forward<T>(range), function);
        }
    };

    template<class F>
    struct FirstOrDefault {
        F function;
        explicit FirstOrDefault(F&& func) : function(func) {}

        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::FirstOrDefault(std::forward<T>(range), function);
        }
    };

    template<class F>
    struct Last {
        F function;
        explicit Last(F&& func) : function(func) {}

        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::Last(std::forward<T>(range), function);
        }
    };

    template<class F>
    struct LastOrDefault {
        F function;
        explicit LastOrDefault(F&& func) : function(func) {}

        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::LastOrDefault(std::forward<T>(range), function);
        }
    };


    template<class F>
    struct Any {
        F function;
        explicit Any(F&& func) : function(func) {}

        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::Any(std::forward<T>(range), function);
        }
    };

    template<class F>
    struct All {
        F function;
        explicit All(F&& func) : function(func) {}

        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::All(std::forward<T>(range), function);
        }
    };

    struct Reverse {
        explicit Reverse() {}
        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::Reverse(std::forward<T>(range));
        }
    };

    struct ToArray {
        explicit ToArray() {}
        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::ToArray(std::forward<T>(range));
        }
    };
    struct ToVector {
        explicit ToVector() {}
        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::ToVector(std::forward<T>(range));
        }
    };
    struct Count {
        explicit Count() {}
        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::Count(std::forward<T>(range));
        }
    };
    struct ToList {
        explicit ToList() {}
        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            return Sombrero::Linq::ToList(std::forward<T>(range));
        }
    };
    template<class T, class R>
    requires (Sombrero::Linq::range<T>)
    auto operator|(T&& inp, R&& rhs) {
        return rhs.transform(inp);
    }
}