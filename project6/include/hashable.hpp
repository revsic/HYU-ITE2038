#ifndef HASHABLE_HPP
#define HASHABLE_HPP

#include <functional>
#include <vector>

#include "utils.hpp"

/// Variadic template based hash pack.
/// \tparam typename TupleType, type of the data pack.
/// \tparam std::size_t... Indices, index sequence.
template <typename TupleType, std::size_t... Indices>
struct PackHasher;

/// Variadic template based hash pack.
/// \tparam typename TupleType, type of the data pack.
/// \tparam std::size_t Idx, current index.
/// \tparam std::size_t... Indices, index sequence.
template <typename TupleType, std::size_t Idx, std::size_t... Indices>
struct PackHasher<TupleType, Idx, Indices...> {
    /// Run hash function.
    /// \param data TupleType const&, data pack.
    /// \param res std::string, packed data.
    /// \return std::size_t, hash.
    static std::size_t run(TupleType const& data, std::string res) {
        using current_t = std::tuple_element_t<Idx, TupleType>;
        char const* ptr = reinterpret_cast<char const*>(&std::get<Idx>(data));
        res.insert(res.end(), ptr, ptr + sizeof(current_t));
        return PackHasher<TupleType, Indices...>::run(data, std::move(res));
    }
};

/// Variadic template based hash pack.
/// \tparam typename TupleType, type of the data pack.
template <typename TupleType>
struct PackHasher<TupleType> {
    /// Run hash function.
    /// \param data TupleType const&, data pack.
    /// \param res std::string, totally packed data.
    /// \return std::size_t, hash.
    static std::size_t run(TupleType const& data, std::string res) {
        return std::hash<std::string>{}(res);
    }
};

/// Hashable data pack.
/// \tparam typename... T, data types.
template <typename... T>
struct HashablePack {
    /// data pack.
    std::tuple<T...> data;

    /// Construct data with tuple based data pack.
    /// \tparam typename... Tp, types.
    /// \param _ utils::token_t, data based constructor token.
    /// \param data Tp&&..., data parameter pack.
    template <typename... Tp>
    HashablePack(utils::token_t, Tp&&... data) : data(std::forward<Tp>(data)...) {
        // Do Nothing
    }
    /// Copy constructor.
    HashablePack(HashablePack const& pack) : data(pack.data) {
        // Do Nothing
    }
    /// Move constructor.
    HashablePack(HashablePack&& pack) : data(std::move(pack.data)) {
        // Do Nothing
    }
    /// Equality comparable.
    bool operator==(HashablePack const& other) const {
        return data == other.data;
    }
    /// Hash data pack.
    std::size_t hash() const {
        return hash_proxy(std::make_index_sequence<sizeof...(T)>{});
    }
    /// Proxy method for hashing data pack.
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