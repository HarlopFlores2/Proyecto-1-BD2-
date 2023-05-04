#include <cstdint>
#include <functional>
#include <istream>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include "attribute.hpp"
#include "json.hpp"

using json = nlohmann::json;

auto INTEGER::operator==(INTEGER const& other) const -> bool
{
    return true;
}

auto INTEGER::read(std::istream& in) const -> json
{
    type ret = 0;
    in.read(reinterpret_cast<char*>(&ret), sizeof(ret));
    return json(ret);
}

void INTEGER::write(std::ostream& out, json const& j) const
{
    type n = j.get<type>();
    out.write(reinterpret_cast<char const*>(&n), sizeof(n));
}

auto INTEGER::to_specifier() const -> std::string
{
    return {"INTEGER"};
}

auto INTEGER::valid_json(json const& j) const -> bool
{
    return j.type() == json::value_t::number_integer;
}

auto INTEGER::size() const -> uint64_t
{
    return sizeof(INTEGER::type);
}

auto INTEGER::from_specifier(std::string const& specifier) -> std::optional<INTEGER>
{
    if (specifier == "INTEGER")
    {
        return {INTEGER{}};
    }

    return {};
}

auto operator<<(std::ostream& out, INTEGER const& i) -> std::ostream&
{
    out << i.to_specifier();
    return out;
}

auto VARCHAR::operator==(VARCHAR const& other) const -> bool
{
    return n_chars == other.n_chars;
}

auto VARCHAR::read(std::istream& in) const -> json
{
    auto temp = new type[n_chars];
    in.read(reinterpret_cast<char*>(temp), n_chars * sizeof(type));
    json ret(temp);
    delete[] temp;

    return ret;
}

void VARCHAR::write(std::ostream& out, json const& j) const
{
    std::string s = j.get<std::string>();
    auto s_to_write = std::min(s.size(), n_chars);
    out.write(s.data(), s_to_write);

    char null = '\0';
    for (size_t i = 0; i < n_chars - s_to_write; i++)
    {
        out.write(&null, 1);
    }
}

auto VARCHAR::to_specifier() const -> std::string
{
    return {"VARCHAR(" + std::to_string(this->n_chars) + ")"};
}

auto VARCHAR::valid_json(json const& j) const -> bool
{
    return j.type() == json::value_t::string;
}

auto VARCHAR::size() const -> uint64_t
{
    return sizeof(VARCHAR::type) * n_chars;
}

auto VARCHAR::from_specifier(std::string const& specifier) -> std::optional<VARCHAR>
{
    std::regex re{R"(VARCHAR\((\d+)\))"};
    std::cmatch m;

    if (!std::regex_match(specifier.c_str(), m, re))
    {
        return {};
    }

    uint64_t n_digits = std::stoull(m[1]);

    return {VARCHAR{n_digits}};
}

auto operator<<(std::ostream& out, VARCHAR const& vc) -> std::ostream&
{
    out << vc.to_specifier();
    return out;
}

auto Attribute::operator==(Attribute const& other) const -> bool
{
    return name == other.name && type == other.type;
}

Attribute::Attribute(std::string name, attribute_type type)
    : name{std::move(name)},
      type{std::move(type)}
{
}

Attribute::Attribute(std::string name, std::string const& type_string)
    : name{std::move(name)}
{
    // NOTE: This somehow works
    using from_specifier_type =
        std::function<std::optional<attribute_type>(std::string const&)>;
    std::vector<from_specifier_type> functions_to_try = {
        INTEGER::from_specifier, VARCHAR::from_specifier};

    for (auto const& f : functions_to_try)
    {
        auto maybe_value = f(type_string);
        if (maybe_value.has_value())
        {
            type = {maybe_value.value()};
            return;
        }
    }

    throw std::runtime_error("Type specifier did not match any known type");
}

auto Attribute::read(std::istream& in) const -> json
{
    return std::visit([&](auto& a) { return a.read(in); }, type);
}

void Attribute::write(std::ostream& out, json const& j) const
{
    std::visit([&](auto& a) { return a.write(out, j); }, type);
}

auto Attribute::to_specifier() const -> std::string
{
    return std::visit([&](auto& a) { return a.to_specifier(); }, type);
}

auto Attribute::valid_json(json const& j) const -> bool
{
    return std::visit([&](auto& a) { return a.valid_json(j); }, type);
}

auto Attribute::size() const -> uint64_t
{
    return std::visit([&](auto& a) { return a.size(); }, type);
}

auto operator<<(std::ostream& out, Attribute const& vc) -> std::ostream&
{
    out << vc.name;
    out << "{";
    std::visit([&out](auto&& a) { out << a; }, vc.type);
    out << "}";
    return out;
}
