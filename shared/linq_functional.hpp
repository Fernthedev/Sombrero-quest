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
            return Sombrero::Linq::SelectIterable<decltype(range.begin()), F>(range, std::forward<F>(function));
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
    #ifdef USE_CODEGEN
    struct ToList {
        explicit ToList() {}
        template<class T>
        requires (Sombrero::Linq::range<T>)
        auto transform(T&& range) {
            // TODO: Have this do the right thing
        }
    };
    #endif
    template<class T, class R>
    requires (Sombrero::Linq::range<T>)
    auto operator|(T&& inp, R&& rhs) {
        return rhs.transform(inp);
    }
}