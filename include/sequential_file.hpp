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
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = IndexRecord;
        using difference_type = std::ptrdiff_t;
        using reference = IndexRecord const&;
        using pointer = IndexRecord const*;

    private:
        std::filesystem::path m_filename;
        mutable std::ifstream m_file;
        uint64_t m_index;

        mutable std::optional<IndexRecord> m_value_read;

        SequentialFile const* m_sf;

        RawIterator(std::filesystem::path filename, uint64_t index, SequentialFile const* sf);
        RawIterator(RawIterator const& it);

        uint64_t calculate_offset(uint64_t index) const;

        auto operator*() const -> reference;
        auto operator->() const -> pointer;

        auto operator+=(difference_type n) -> RawIterator&;
        auto operator+(difference_type n);
        auto operator-=(difference_type n) -> RawIterator&;
        auto operator-(difference_type n);
        auto operator-(RawIterator const& other);

        auto operator++() -> RawIterator&;
        auto operator++(int) -> RawIterator;

        auto operator==(RawIterator const& other) -> bool;
        auto operator!=(RawIterator const& other) -> bool;
    };

    using reverse_raw_iterator = std::reverse_iterator<RawIterator>;

    class Iterator
    {
        friend IndexLocation;

        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using reference = IndexRecord const&;
        using pointer = IndexRecord const*;

    private:
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

        uint64_t calculate_offset(uint64_t index, IndexLocation index_location) const;

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

    Iterator begin() const;
    Iterator end() const;

    void insert(nlohmann::json const& key, uint64_t relation_index);

    bool remove_record(nlohmann::json key, uint64_t relation_index);

    void merge_data();

    std::optional<Iterator> find_location_to_add(nlohmann::json const& key);

    std::pair<IndexLocation, uint64_t> get_header() const;
    void set_header(IndexLocation index_location, uint64_t next_position) const;

    uint64_t calculate_offset(uint64_t index, IndexLocation index_location);

    void
    write_record(IndexLocation index_location, uint64_t index, IndexRecord const& record) const;
    void write_record(IndexRecord const& ir, std::ostream& os) const;
    auto read_record(std::istream& in) const -> IndexRecord;
};
