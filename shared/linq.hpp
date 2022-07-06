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

    template<class T>
    concept has_size_member = requires (T t) {
        {t.size} -> Sombrero::Linq::convertible_to<il2cpp_array_size_t>;
    };

    template<class T>
    concept has_size_call = requires (T t) {
        {t.size()} -> Sombrero::Linq::convertible_to<il2cpp_array_size_t>;
    };

    template<class I1, class I2>
    concept has_distance = requires (I1 i1, I2 i2) {
        {i1 - i2} -> Sombrero::Linq::convertible_to<il2cpp_array_size_t>;
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
            // ptrdiff_t can't be used for this type, since we can't know the distance.
            using difference_type = std::ptrdiff_t;
            using value_type = typename std::iterator_traits<I>::value_type;
            using pointer = typename std::iterator_traits<I>::pointer;
            using reference = typename std::iterator_traits<I>::reference;
            using iterator_category = std::forward_iterator_tag;
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
            // Actually has value for this type
            // TODO: Create - operators
            using difference_type = std::ptrdiff_t;
            using value_type = std::invoke_result<F, I>;
            using pointer = void;
            using reference = void;
            using iterator_category = std::forward_iterator_tag;
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

    template<class T>
    requires (range<T>)
    auto ToArray(T&& range) {
        // TODO: Improve this using better concepts
        // We can block copy if we satisfy contiguous iterator
        // So, this is actually kind of interesting.
        // If we can compute the distance (ex, we can easily tell how large our collection is)
        // Then we should do that
        // Otherwise, we kinda have to make a vector then shrink it and be good to go
        using ItemT = std::remove_reference_t<decltype(*range.begin())>;
        if constexpr (has_size_call<T>) {
            ArrayW<ItemT> arr(static_cast<il2cpp_array_size_t>(range.size()));
            // With a size method call, there's no guarantee we can block copy
            std::copy(range.begin(), range.end(), arr.begin());
            return arr;
        } else if constexpr (has_size_member<T>) {
            ArrayW<ItemT> arr(static_cast<il2cpp_array_size_t>(range.size));
            // With a size member, we can be FAIRLY confident we can block copy
            // but lets assume we can't
            std::copy(range.begin(), range.end(), arr.begin());
            return arr;
        } else if constexpr (has_distance<decltype(range.end()), decltype(range.begin())>) {
            // With a iterator distance, we can be even more confident we can block copy
            // let's actually do the block copy here.
            ArrayW<ItemT> arr(static_cast<il2cpp_array_size_t>(range.end() - range.begin()));
            std::copy(range.begin(), range.end(), arr.begin());
            return arr;
        } else {
            // We don't know how to make an array of the correct size.
            // Lets make a vector and fill it, instead.
            std::vector<ItemT> vec(range.begin(), range.end());
            ArrayW<ItemT> arr(vec.size());
            std::copy(vec.begin(), vec.end(), arr.begin());
            return arr;
        }
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