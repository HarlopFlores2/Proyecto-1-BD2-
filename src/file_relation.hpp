#pragma once

#include <cstddef>
#include <cstdint>
#include <experimental/filesystem>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <string>

#include "attribute.hpp"
#include "base_relation.hpp"
#include "json.hpp"
#include "memory_relation.hpp"

struct FileRelation : public Relation
{
    struct Iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using reference = nlohmann::json const&;
        using pointer = nlohmann::json const*;

    private:
        std::string const m_filename;
        mutable std::ifstream m_file;
        uint64_t m_file_offset;
        FileRelation const* m_fr;
        mutable std::optional<nlohmann::json> m_value_read;
        bool m_end = false;

    public:
        Iterator(std::string const& filename, uint64_t file_offset, FileRelation const* fr);
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

    private:
        void advance_until_next_valid();
    };

    constexpr static uint64_t const record_not_deleted = std::numeric_limits<uint64_t>::max();
    constexpr static uint64_t const record_deleted_and_last =
        std::numeric_limits<uint64_t>::max() - 1;

    uint64_t const m_record_size;

    std::string m_name;
    std::filesystem::path m_filename;

    FileRelation(
        std::vector<Attribute> attributes,
        std::map<int, int> indexes,
        std::string name,
        std::filesystem::path filename);

    auto to_memory() -> MemoryRelation;

    void insert(nlohmann::json const& tuple);
    auto remove(uint64_t index) -> bool;

    [[nodiscard]] auto calculate_offset(uint64_t index) const -> uint64_t;
};
