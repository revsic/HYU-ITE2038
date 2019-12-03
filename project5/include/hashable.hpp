#ifndef HASHABLE_HPP
#define HASHABLE_HPP

#include <functional>
#include <vector>

#include "utils.hpp"

template <typename TupleType, std::size_t... Indices>
struct PackHasher;

template <typename TupleType, std::size_t Idx, std::size_t... Indices>
struct PackHasher<TupleType, Idx, Indices...> {
    static std::size_t run(TupleType const& data, std::string res) {
        using current_t = std::tuple_element_t<Idx, TupleType>;
        char const* ptr = reinterpret_cast<char const*>(&std::get<Idx>(data));
        res.insert(res.end(), ptr, ptr + sizeof(current_t));
        return PackHasher<TupleType, Indices...>::run(data, std::move(res));
    }
};

template <typename TupleType>
struct PackHasher<TupleType> {
    static std::size_t run(TupleType const& data, std::string res) {
        return std::hash<std::string>{}(res);
    }
};

template <typename... T>
struct HashablePack {
    std::tuple<T...> data;

    template <typename... Tp>
    HashablePack(utils::token_t, Tp&&... data) : data(std::forward<Tp>(data)...) {
        // Do Nothing
    }

    HashablePack(HashablePack const& pack) : data(pack.data) {
        // Do Nothing
    }

    HashablePack(HashablePack&& pack) : data(std::move(pack.data)) {
        // Do Nothing
    }

    bool operator==(HashablePack const& other) const {
        return data == other.data;
    }

    std::size_t hash() const {
        return hash_proxy(std::make_index_sequence<sizeof...(T)>{});
    }

    template <std::size_t... Indices>
    std::size_t hash_proxy(std::index_sequence<Indices...>) const {
        return PackHasher<std::tuple<T...>, Indices...>::run(data, std::string());
    }
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