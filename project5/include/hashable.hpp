#ifndef HASHABLE_HPP
#define HASHABLE_HPP

#include <functional>
#include <vector>

template <typename... T>
struct HashablePack {
    std::tuple<T...> data;

    template <typename... Tp>
    HashablePack(Tp&&... data) : data(std::forward<Tp>(data)...) {
        // Do Nothing
    }

    std::size_t hash() const {
        return hash_proxy(std::make_index_sequence<sizeof...(T)>{});
    }

    template <std::size_t... Indices>
    std::size_t hash_proxy(std::index_sequence<Indices...>) const {
        return Hasher<Indices...>::run(data, std::vector<char>());
    }

    template <std::size_t... Indices>
    struct Hasher;

    template <std::size_t Idx, std::size_t... Indices>
    struct Hasher<Idx, Indices...> {
        static std::size_t run(std::tuple<T...> const& data, std::vector<char> res) {
            using current_t = std::tuple_element_t<Idx, std::tuple<T...>>;
            char const* ptr = reinterpret_cast<char const*>(&std::get<Idx>(data));
            res.insert(res.end(), ptr, ptr + sizeof(current_t));
            return Hasher<Indices...>::run(data, std::move(res));
        }
    };

    template <>
    struct Hasher<> {
        static std::size_t run(std::tuple<T...> const& data, std::vector<char> res) {
            std::string str(res.begin(), res.end());
            return std::hash<std::string>{}(str);
        }
    };
};

namespace std {
    template <typename... T>
    struct hash<HashablePack<T...>> {
        std::size_t operator()(HashablePack<T...> const& pack) const {
            return pack.hash();
        }
    };
}

#endif