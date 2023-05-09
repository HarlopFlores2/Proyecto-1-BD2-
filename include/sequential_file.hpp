#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#include "attribute.hpp"
#include "json.hpp"

class SequentialFile
{
private:
    enum class IndexLocation
    {
        data,
        aux,
        no_next
    };

public:
    struct IndexRecord
    {
        nlohmann::json key;
        uint64_t relation_index;

        IndexLocation next_file;
        uint64_t next_position;
        bool deleted = false;

        static constexpr uint64_t size_without_key = sizeof(relation_index) + sizeof(next_file)
                                                     + sizeof(next_position) + sizeof(deleted);
    };

public:
    std::filesystem::path m_data_filename;
    std::filesystem::path m_aux_filename;

    mutable std::fstream m_data_file;
    mutable std::fstream m_aux_file;

    Attribute m_key_attribute;
    uint64_t const m_record_size;

    uint64_t m_max_aux_size;

public:
    constexpr static uint64_t header_size =
        sizeof(IndexRecord::next_file) + sizeof(IndexRecord::next_position);

    class RawIterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = IndexRecord;
        using difference_type = std::ptrdiff_t;
        using reference = IndexRecord const&;
        using pointer = IndexRecord const*;

    public:
        std::filesystem::path m_filename;
        mutable std::ifstream m_file;
        uint64_t m_index;

        mutable std::optional<IndexRecord> m_value_read;

        SequentialFile const* m_sf;

        RawIterator(std::filesystem::path filename, uint64_t index, SequentialFile const* sf);
        RawIterator(RawIterator const& other);
        RawIterator(RawIterator&& other) noexcept;

        void swap(RawIterator& other);
        void swap(RawIterator&& other);

        auto operator=(RawIterator const& other) -> RawIterator&;
        auto operator=(RawIterator&& other) noexcept -> RawIterator&;

        auto calculate_offset(uint64_t index) const -> uint64_t;

        auto operator*() const -> reference;
        auto operator->() const -> pointer;

        auto operator+=(difference_type n) -> RawIterator&;
        auto operator+(difference_type n);
        auto operator-=(difference_type n) -> RawIterator&;
        auto operator-(difference_type n);
        auto operator-(RawIterator const& other);

        auto operator++() -> RawIterator&;
        auto operator++(int) -> RawIterator;
        auto operator--() -> RawIterator&;
        auto operator--(int) -> RawIterator;

        auto operator==(RawIterator const& other) -> bool;
        auto operator!=(RawIterator const& other) -> bool;
    };

    using reverse_raw_iterator = std::reverse_iterator<RawIterator>;

    class Iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using reference = IndexRecord const&;
        using pointer = IndexRecord const*;

    public:
        mutable std::ifstream m_data_file;
        mutable std::ifstream m_aux_file;

        std::filesystem::path m_data_filename;
        std::filesystem::path m_aux_filename;

        uint64_t m_index;
        IndexLocation m_index_location;

        SequentialFile const* m_sf;

        mutable std::optional<IndexRecord> m_value_read;
        bool m_end = false;

        Iterator(
            std::filesystem::path data_filename,
            std::filesystem::path aux_filename,
            uint64_t file_index,
            IndexLocation index_location,
            SequentialFile const* sf);
        Iterator(Iterator const& it);

        auto operator=(Iterator const& other) -> Iterator&;

        auto calculate_offset(uint64_t index, IndexLocation index_location) const -> uint64_t;

        auto operator*() const -> reference;
        auto operator->() const -> pointer;

        void advance();

        auto operator++() -> Iterator&;
        auto operator++(int) -> Iterator;

        auto operator==(Iterator const& other) -> bool;
        auto operator!=(Iterator const& other) -> bool;

        void advance_until_next_valid();
    };

    explicit SequentialFile(
        std::filesystem::path data_filename,
        std::filesystem::path aux_filename,
        Attribute key_attribute,
        uint64_t max_aux_size);

    SequentialFile(SequentialFile const& other);
    SequentialFile(SequentialFile&& other) = delete;

    auto begin() const -> Iterator;
    auto end() const -> Iterator;

    void insert(nlohmann::json const& key, uint64_t relation_index);

    auto remove_record(nlohmann::json key, uint64_t relation_index) -> bool;

    void merge_data();

    auto find_location_to_add(nlohmann::json const& key) const -> std::optional<Iterator>;
    auto find_location(nlohmann::json const& key) const -> Iterator;

    auto get_header() const -> std::pair<IndexLocation, uint64_t>;
    void set_header(IndexLocation index_location, uint64_t next_position) const;

    auto calculate_offset(uint64_t index, IndexLocation index_location) -> uint64_t;

    void
    write_record(IndexLocation index_location, uint64_t index, IndexRecord const& record) const;
    void write_record(IndexRecord const& ir, std::ostream& os) const;
    auto read_record(std::istream& in) const -> IndexRecord;
};
