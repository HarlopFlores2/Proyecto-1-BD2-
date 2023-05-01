#pragma once

#include <map>
#include <vector>

#include "base_relation.hpp"
#include "json.hpp"

struct MemoryRelation : public Relation
{
    struct Iterator
    {
    public:
        // XXX: Because of the possibility of adding deleted mask, the iterator can only be
        // bidirectional
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using reference = nlohmann::json const&;
        using pointer = nlohmann::json const*;

    private:
        MemoryRelation const* m_mr;
        uint64_t m_index;

    public:
        Iterator(MemoryRelation const* mr, uint64_t index);

        Iterator(Iterator const& it);

        ~Iterator() = default;

        auto operator*() const -> reference;
        auto operator->() const -> pointer;

        auto operator++() -> Iterator&;
        auto operator++(int) -> Iterator;
        auto operator--() -> Iterator&;
        auto operator--(int) -> Iterator;

        friend auto operator==(Iterator const& a, Iterator const& b) -> bool;
        friend auto operator!=(Iterator const& a, Iterator const& b) -> bool;
    };

    std::vector<nlohmann::json> m_tuples;

    MemoryRelation(
        std::vector<Attribute> attributes,
        std::map<int, int> indexes,
        std::vector<nlohmann::json> tuples);

    auto begin() const -> Iterator;
    auto end() const -> Iterator;

    void insert(nlohmann::json tuple);
};
