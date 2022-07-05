#pragma once

#include <concepts>

namespace Sombrero::Linq {



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
    template<ConstIterable T, typename V = typename T::value_type, typename F>
    T Where(T const& list, F&& fn) {
        std::vector<V> copy;
        copy.reserve(list.size());

        auto copyIt = copy.begin();

        for (auto it = list.begin(); it != list.end(); it++) {
            if (!fn(*it)) continue;

            *copyIt = *it;
            copyIt++;
        }

        copy.shrink_to_fit();

        return copy;
    }

    // TODO: Template deduct R
    template<ConstIterable T, typename R, Iterable ConstIterableResult = ArrayW<R>, typename V = typename T::value_type, typename F>
    Iterable auto Select(T const& list, F&& fn) {
        Iterable auto result(list.size());

        auto resultIt = result.begin();
        for (auto const& v : list) {
            *resultIt = fn(v);
            resultIt++;
        }

        return result;
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