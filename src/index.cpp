#include <cstdint>
#include <optional>
#include <vector>

#include "index.hpp"
#include "predicates.hpp"
#include "sequential_file.hpp"
#include "util.hpp"

auto operate_index(index_type const& index, predicate_type const& pred)
    -> std::optional<std::vector<uint64_t>>
{
    using ret_type = std::optional<std::vector<uint64_t>>;

    return std::visit(
        overloaded{[&](SequentialFile const& sf) -> ret_type {
            return std::visit(
                overloaded{
                    [&](P_A_GREATER_C const& p) -> ret_type {
                        if (p.m_a != sf.m_key_attribute.name)
                        {
                            return {};
                        }

                        auto it = sf.find_location(p.m_c);

                        std::vector<uint64_t> ret;
                        while (it != sf.end())
                        {
                            if (it->key > p.m_c)
                            {
                                ret.push_back(it->relation_index);
                            }

                            ++it;
                        }

                        return {ret};
                    },
                    [&](P_A_EQUAL_C const& p) -> ret_type {
                        if (p.m_a != sf.m_key_attribute.name)
                        {
                            return {};
                        }

                        auto it = sf.find_location(p.m_c);

                        std::vector<uint64_t> ret;
                        while (it != sf.end() && it->key == p.m_c)
                        {
                            ret.push_back(it->relation_index);
                            ++it;
                        }

                        return {ret};
                    },
                    [&](P_A_BETWEEN_C1_C2 const& p) -> ret_type {
                        if (p.m_a != sf.m_key_attribute.name)
                        {
                            return {};
                        }

                        auto it = sf.find_location(p.m_c1);

                        std::vector<uint64_t> ret;
                        while (it != sf.end() && it->key <= p.m_c2)
                        {
                            ret.push_back(it->relation_index);
                            ++it;
                        }

                        return {ret};
                    },
                    [](auto&&) -> ret_type { return {}; },
                },
                pred);
        }},
        index);
}
