#pragma once

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "attribute.hpp"
#include "base_relation.hpp"
#include "csv.hpp"
#include "json.hpp"
#include "memory_relation.hpp"
#include "relation.hpp"

template<
    typename R,
    typename Un = typename std::enable_if<std::is_base_of<Relation, R>::value>::type>
void relation_to_csv(R const& relation, std::ostream& out)
{
    auto writer = csv::make_csv_writer(out);

    std::vector<std::string> v_tuple;
    v_tuple.reserve(relation.m_attributes.size());

    for (Attribute const& a : relation.m_attributes)
    {
        v_tuple.push_back(a.name);
    }
    writer << v_tuple;

    for (nlohmann::json const& tuple : relation)
    {
        v_tuple.clear();
        for (nlohmann::json const& e : tuple)
        {
            if (e.is_string())
            {
                v_tuple.push_back(e);
            }
            else
            {
                v_tuple.push_back(e.dump());
            }
        }

        writer << v_tuple;
    }
}
