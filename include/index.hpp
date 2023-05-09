#pragma once

#include "predicates.hpp"
#include "sequential_file.hpp"

using index_type = std::variant<SequentialFile>;

auto operate_index(index_type& index, predicate_type const& pred)
    -> std::optional<std::vector<uint64_t>>;
