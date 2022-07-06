#pragma once

#include <concepts>
#include <type_traits>
#include "Concepts.hpp"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"
#include <optional>
#include <iterator>
#include <functional>

namespace Sombrero::Linq {

    template <class From, class To>
    concept convertible_to =
        std::is_convertible_v<From, To> &&
        requires {
            static_cast<To>(std::declval<From>());
        };

    template<class T>
    concept input_iterator = requires (T t) {
        {*t};
        {++t} -> convertible_to<T>;
    };

    template<class T>
    concept range = requires (T& t) {
        t.begin();
        t.end();
    };

    template<class I, class F>
    requires (input_iterator<I>)
    struct WhereIterable {
        private:
        I start;
        I last;
        using T = decltype(*start);
        F function;
        public:
        struct WhereIterator {
            explicit WhereIterator(WhereIterable const& v, I iter) : iterable(v), iterator(iter) {
                // Start by finding the first match
                while (iterator != iterable.last && !iterable.function(*iterator)) {
                    ++iterator;
                }
                // If we walk all the way to the end, iterator == iterable.last
            }
            T& operator*() const {
                return *iterator;
            }
            WhereIterator& operator++() {
                // Move first, then compare.
                while (++iterator != iterable.last) {
                    if (iterable.function(*iterator)) {
                        break;
                    }
                }
                return *this;
            }
            WhereIterator operator++(int) {
                WhereIterator ret(iterable, iterator);
                this->operator++();
                return ret;
            }
            bool operator==(WhereIterator const& other) const {
                return iterator == other.iterator;
            }
            private:
            WhereIterable const& iterable;
            I iterator;
        };
        auto begin() const {
            return WhereIterator(*this, start);
        }
        auto end() const {
            return WhereIterator(*this, last);
        }
        template<class R>
        requires (range<R>)
        explicit WhereIterable(R&& range, F&& func) : start(range.begin()), last(range.end()), function(func) {}
    };

    template<class R, class F>
    requires (range<R>)
    WhereIterable(R&& r, F&&) -> WhereIterable<decltype(r.begin()), F>;

    template<class I, class F>
    requires (input_iterator<I>)
    struct SelectIterable {
        private:
        I start;
        I last;
        F function;
        public:
        struct SelectIterator {
            explicit SelectIterator(SelectIterable const& v, I iter) : iterable(v), iterator(iter) {}
            // TODO: Add support for function being invoked with index
            // TODO: Do we want to evaluate on * or have * always be post-evaluation?
            // Unclear which makes more sense.
            auto operator*() const {
                return iterable.function(*iterator);
            }
            SelectIterator& operator++() {
                ++iterator;
                return *this;
            }
            SelectIterator operator++(int) {
                return SelectIterator(iterable, iterator++);
            }
            bool operator==(SelectIterator const& other) const {
                return iterator == other.iterator;
            }
            private:
            SelectIterable const& iterable;
            I iterator;
        };
        auto begin() const {
            return SelectIterator(*this, start);
        }
        auto end() const {
            return SelectIterator(*this, last);
        }
        template<class Range>
        requires (range<Range>)
        explicit SelectIterable(Range&& range, F&& func) : start(range.begin()), last(range.end()), function(func) {}
    };

    template<class Range, class F>
    requires (range<Range>)
    SelectIterable(Range&& r, F&&) -> SelectIterable<decltype(r.begin()), F>;


    template<typename T, typename Iterator = typename T::iterator>
    concept ConstIterable = requires(T const& t, int length, Iterator it) {
        {T() -> SomberoConvertible<T>};
        {T(length) -> SomberoConvertible<T>};
        {T(it, it) -> SomberoConvertible<T>};
        {t.begin()};
        {t.end()};
        {t.size()};
    } && std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>;

    template<typename T>
    concept Iterable = ConstIterable<T> && requires(T& t) {
        {t.begin()};
        {t.end()};
    } && std::is_move_constructible_v<T> && std::is_move_assignable_v<T>;


    /**
     * Finds all elements that satisfy @param fn
     * @tparam T
     * @tparam F
     * @param list
     * @param fn
     * @return
     */
    template<class T, typename F>
    requires (range<T>)
    auto Where(T const& list, F&& fn) {
        return WhereIterable(list, fn);
    }

    template<class T, typename F>
    requires (range<T>)
    auto Select(T const& list, F&& fn) {
        return SelectIterable(list, fn);
    }

    template<Iterable T, typename V = typename T::value_type, typename F>
    std::optional<std::reference_wrapper<V>> First(T& list, F&& fn) {
        for (auto& v : list)
            if (fn(v)) return std::ref(v);

        return std::nullopt;
    }

    template<Iterable T, typename V = typename T::value_type, typename F>
    std::optional<std::reference_wrapper<const V>> First(T const& list, F&& fn) {
        for (auto& v : list)
            if (fn(v)) return std::cref(v);

        return std::nullopt;
    }

    template<typename... TArgs>
    auto FirstOrDefault(TArgs&&... args) {
        return First(std::forward<TArgs>(args)...).value_or({});
    }

    auto Last(auto& list, auto&& fn) {
        return First(std::span(std::rbegin(list), std::rend(list)), fn);
    }

    auto Last(auto const& list, auto&& fn) {
        return First(std::span(std::rbegin(list), std::rend(list)), fn);
    }

    template<typename... TArgs>
    auto LastOrDefault(TArgs&&... args) {
        return Last(std::forward<TArgs>(args)...).value_or({});
    }

    template<ConstIterable T, typename F>
    bool Any(T const& list, F&& fn) {
        for (auto it = list.begin(); it != list.end(); it++) {
            if (fn(*it)) return true;
        }

        return false;
    }

    template<ConstIterable T, typename F>
    bool All(T const& list, F&& fn) {
        for (auto it = list.begin(); it != list.end(); it++) {
            if (!fn(*it)) return false;
        }

        return true;
    }

}