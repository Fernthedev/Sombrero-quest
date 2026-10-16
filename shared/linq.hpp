#pragma once
#include <ranges>

#include <concepts>
#include <type_traits>
#include "Concepts.hpp"
#include "beatsaber-hook/shared/arrayw.hpp"
#include "beatsaber-hook/shared/listw.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include <optional>
#include <iterator>
#include <functional>
#include <utility>
#include <vector>

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

    template<class T>
    concept can_get_size = has_size_member<T> || has_size_call<T> || requires (T t) {
        requires has_distance<decltype(t.end()), decltype(t.begin())>;
    };
    template<class T>
    requires (range<T> && can_get_size<T>)
    il2cpp_array_size_t get_size(T&& range) {
        if constexpr (has_distance<decltype(range.end()), decltype(range.begin())>) {
            // Try distance first
            return range.end() - range.begin();
        } else if constexpr (has_size_call<T>) {
            // Then try the size method call
            return range.size();
        } else if constexpr (has_size_member<T>) {
            // Then try the size member (TODO: Maybe not?)
            return range.size;
        } else {
            static_assert(has_size_call<T> || has_size_member<T> || has_distance<decltype(range.end()), decltype(range.begin())>, "Does not satisfy can_get_size!");
        }
    }

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
    auto Where(T&& list, F&& fn) {
        return std::views::filter(std::forward<T>(list), std::forward<F>(fn));
    }

    template<class T, typename F>
    requires (range<T>)
    auto Select(T&& list, F&& fn) {
        return std::views::transform(std::forward<T>(list), std::forward<F>(fn));
    }

    template<class T>
    requires (range<T>)
    auto Reverse(T&& list) {
        return std::views::reverse(std::forward<T>(list));
    }

    namespace detail {
        template<class ItemT>
        bool needs_managed_buffer() {
            if constexpr (i2c::type_check::ref_type<ItemT>) {
                return true;
            } else if constexpr (std::is_arithmetic_v<ItemT> || std::is_enum_v<ItemT>) {
                return false;
            } else {
                // Value types can contain managed references too. The runtime
                // query initializes field layout; cache its result per type.
                static const bool hasReferences = [] {
                    i2c::functions::initialize();
                    return i2c::functions::class_has_references(i2c::class_of<ItemT>());
                }();
                return hasReferences;
            }
        }
    }

    template<class T>
    requires (range<T>)
    auto ToArray(T&& range) {
        using ItemT = std::ranges::range_value_t<T>;
        if constexpr (can_get_size<T>) {
            ArrayW<ItemT> arr(get_size<T>(std::forward<T>(range)));
            // Projections can allocate or trigger GC between element writes.
            safe_ptr<ArrayW<ItemT>, false> outputRoot(arr);
            std::copy(range.begin(), range.end(), arr.begin());
            return arr;
        } else {
            if (!detail::needs_managed_buffer<ItemT>()) {
                std::vector<ItemT> buffer;
                for (auto&& value : range) {
                    buffer.emplace_back(std::forward<decltype(value)>(value));
                }
                ArrayW<ItemT> arr(buffer.size());
                std::copy(buffer.begin(), buffer.end(), arr.begin());
                return arr;
            }
            // A native buffer is not scanned by managed GC. Keep previously
            // produced values in a rooted managed list while evaluating the
            // rest of a single-pass/unknown-size range, then copy in managed code.
            using ListT = System::Collections::Generic::List_1<ItemT>;
            safe_ptr<ListT*, false> buffer(i2c::new_ctor<ListT*>());
            for (auto&& value : range) {
#ifdef HAS_CODEGEN
                buffer->Add(value);
#else
                i2c::run_method<void>(buffer.ptr(), "Add", static_cast<ItemT>(value));
#endif
            }
#ifdef HAS_CODEGEN
            return buffer->ToArray();
#else
            return i2c::run_method<ArrayW<ItemT>>(buffer.ptr(), "ToArray");
#endif
        }
    }

    template<class T>
    requires (range<T>)
    auto Count(T&& range) {
        return static_cast<il2cpp_array_size_t>(std::ranges::distance(range));
    }

    template<class T>
    requires (range<T>)
    auto ToVector(T&& range) {
        return std::vector(range.begin(), range.end());
    }

    template<class T>
    requires (range<T>)
    auto ToList(T&& range) {
        using ItemT = std::ranges::range_value_t<T>;
        auto lst = i2c::new_ctor<System::Collections::Generic::List_1<ItemT>*>();
        for (auto&& value : range) {
#ifdef HAS_CODEGEN
            lst->Add(value);
#else
            i2c::run_method<void>(lst, "Add", static_cast<ItemT>(value));
#endif
        }
        return lst;
    }

    template<range T, typename F>
    auto First(T const& list, F&& fn) {
        using ItemT = std::remove_reference_t<decltype(*list.begin())>;

        for (auto v : list)
            if (fn(v)) return std::make_optional<ItemT>(v);

        return (std::optional<ItemT>) std::nullopt;
    }

    template<range T, typename F>
    auto First(T&& list, F&& fn) {
        using ItemT = std::remove_reference_t<decltype(*list.begin())>;

        for (auto const& v : list)
            if (fn(v)) return std::make_optional<ItemT>(v);

        return (std::optional<ItemT>) std::nullopt;
    }

    template<typename... TArgs>
    auto FirstOrDefault(TArgs&&... args) {
        auto opt = First(std::forward<TArgs>(args)...);

        return opt.template value_or<typename decltype(opt)::value_type>({});
    }

    auto Last(auto& list, auto&& fn) {
        return First(std::ranges::subrange(std::rbegin(list), std::rend(list)), fn);
    }

    auto Last(auto const& list, auto&& fn) {
        return First(std::ranges::subrange(std::rbegin(list), std::rend(list)), fn);
    }

    template<typename... TArgs>
    auto LastOrDefault(TArgs&&... args) {
        auto opt = Last(std::forward<TArgs>(args)...);

        return opt.template value_or<typename decltype(opt)::value_type>({});
    }

    template<range T, typename F>
    bool Any(T&& list, F&& fn) {
        for (auto it = list.begin(); it != list.end(); it++) {
            if (fn(*it)) return true;
        }

        return false;
    }

    template<range T, typename F>
    bool All(T&& list, F&& fn) {
        for (auto it = list.begin(); it != list.end(); it++) {
            if (!fn(*it)) return false;
        }

        return true;
    }

}
