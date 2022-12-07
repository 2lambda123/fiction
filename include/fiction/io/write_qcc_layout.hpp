//
// Created by marcel on 07.08.19.
//

#ifndef FICTION_WRITE_QCC_LAYOUT_HPP
#define FICTION_WRITE_QCC_LAYOUT_HPP

#include "fiction/layouts/bounding_box.hpp"
#include "fiction/technology/cell_technologies.hpp"
#include "fiction/technology/magcad_magnet_count.hpp"
#include "fiction/traits.hpp"
#include "fiction/types.hpp"
#include "utils/version_info.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fiction
{

struct write_qcc_layout_params
{
    bool use_filename_as_component_name = false;

    std::string filename{};
};

namespace detail
{

namespace qcc
{

inline constexpr const char* VERSION_HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                              "<!--Generated by {} ({})-->\n";

inline constexpr const char* OPEN_QCA_COMPONENT =
    "<qcacomponent Version=\"2\" tech=\"{}\" lib=\"{}\" name=\"{}\" "
    "ID=\"{}\" function=\"\" maxX=\"{}\" maxY=\"{}\" MagnetsCount=\"{}\">\n";
inline constexpr const char* CLOSE_QCA_COMPONENT = "</qcacomponent>\n";

inline constexpr const char* LIBRARY_NAME = "components_fiction";

inline constexpr const char* OPEN_ENTITY      = "\t<entity>\n";
inline constexpr const char* CLOSE_ENTITY     = "\t</entity>\n";
inline constexpr const char* OPEN_COMPONENTS  = "\t<components>\n";
inline constexpr const char* CLOSE_COMPONENTS = "\t</components>\n";
inline constexpr const char* OPEN_LAYOUT      = "\t<layout>\n";
inline constexpr const char* CLOSE_LAYOUT     = "\t</layout>\n";

inline constexpr const char* PIN            = "\t\t<pin tech=\"{}\" name=\"{}\" direction=\"{}\" x=\"{}\" y=\"{}\"/>\n";
inline constexpr const char* COMPONENT_ITEM = "\t\t<item tech=\"{}\" name=\"{}\"/>\n";

inline constexpr const char* OPEN_LAYOUT_ITEM  = "\t\t<item comp=\"{}\" x=\"{}\" y=\"{}\">\n";
inline constexpr const char* CLOSE_LAYOUT_ITEM = "\t\t</item>\n";

inline constexpr const char* LAYOUT_ITEM_PROPERTY = "\t\t\t<property name=\"{}\" value=\"{}\"/>\n";
inline constexpr const char* PROPERTY_PHASE       = "phase";
inline constexpr const char* PROPERTY_LENGTH      = "length";

inline constexpr const std::array<const char*, 6> COMPONENTS{"Magnet", "Coupler",  "Cross Wire",
                                                             "And",    "Inverter", "Or"};

static const std::unordered_map<inml_technology::cell_type, uint8_t> COMPONENT_SELECTOR{
    {inml_technology::cell_type::NORMAL, 0},           {inml_technology::cell_type::INPUT, 0},
    {inml_technology::cell_type::OUTPUT, 0},           {inml_technology::cell_type::FANOUT_COUPLER_MAGNET, 1},
    {inml_technology::cell_type::CROSSWIRE_MAGNET, 2}, {inml_technology::cell_type::SLANTED_EDGE_DOWN_MAGNET, 3},
    {inml_technology::cell_type::INVERTER_MAGNET, 4},  {inml_technology::cell_type::SLANTED_EDGE_UP_MAGNET, 5},
};

}  // namespace qcc

template <typename Lyt>
class write_qcc_layout_impl
{
  public:
    write_qcc_layout_impl(const Lyt& src, std::ostream& s, write_qcc_layout_params p) :
            lyt{src},
            bb{lyt},
            sorted_pi_list{sorted_pis()},
            sorted_po_list{sorted_pos()},
            num_magnets{magcad_magnet_count(lyt)},
            os{s},
            ps{std::move(p)}
    {}

    void run()
    {
        if (!has_border_io_pins())
        {
            throw std::invalid_argument(
                "the iNML layout does not fulfill all requirements to be written as a QCC file because it does not "
                "have designated I/O pins or they are not routed to the layout's borders");
        }

        write_header();
        write_entity();
        write_components();
        write_layout();

        // close component block and flush the file
        os << qcc::CLOSE_QCA_COMPONENT << std::flush;
    }

  private:
    const Lyt& lyt;

    const bounding_box_2d<Lyt> bb;

    std::vector<cell<Lyt>> sorted_pi_list, sorted_po_list;

    const uint64_t num_magnets;

    std::ostream& os;

    const write_qcc_layout_params ps;

    [[nodiscard]] std::vector<cell<Lyt>> sorted_pis() const noexcept
    {
        std::vector<cell<Lyt>> pi_list{};
        lyt.foreach_pi([&pi_list](const auto& pi) { pi_list.push_back(pi); });
        std::sort(pi_list.begin(), pi_list.end(), [](const auto& c1, const auto& c2) { return c1.y < c2.y; });

        return pi_list;
    }

    [[nodiscard]] std::vector<cell<Lyt>> sorted_pos() const noexcept
    {
        std::vector<cell<Lyt>> po_list{};
        lyt.foreach_po([&po_list](const auto& po) { po_list.push_back(po); });
        std::sort(po_list.begin(), po_list.end(), [](const auto& c1, const auto& c2) { return c1.y < c2.y; });

        return po_list;
    }

    [[nodiscard]] auto bb_x(const cell<Lyt>& c) const noexcept
    {
        return static_cast<decltype(c.x)>(c.x - bb.get_min().x);
    }

    [[nodiscard]] auto bb_y(const cell<Lyt>& c) const noexcept
    {
        return static_cast<decltype(c.y)>(c.y - bb.get_min().y);
    }

    [[nodiscard]] bool has_border_io_pins() const noexcept
    {
        auto all_border_pins = true;

        // check PI border cells
        lyt.foreach_pi(
            [this, &all_border_pins](const auto& pi)
            {
                if (bb_x(pi) != 0)
                {
                    all_border_pins = false;
                    return false;  // break iteration
                }
                return true;  // keep iterating
            });
        // check PO border cells
        lyt.foreach_po(
            [this, &all_border_pins](const auto& po)
            {
                if (bb_x(po) != lyt.x())
                {
                    all_border_pins = false;
                    return false;  // break iteration
                }
                return true;  // keep iterating
            });

        return all_border_pins;
    }

    [[nodiscard]] std::vector<std::string> get_pin_data() const
    {
        std::vector<std::string> pin_data{};

        auto store_pin_data = [this, &pin_data](const auto& io)
        { pin_data.push_back(fmt::format("{}{}{}{}", lyt.get_cell_name(io), bb_x(io), bb_y(io), io.z)); };

        for (const auto& pi : sorted_pi_list)
        {
            store_pin_data(pi);
        }
        for (const auto& po : sorted_po_list)
        {
            store_pin_data(po);
        }
        std::sort(pin_data.begin(), pin_data.end());

        return pin_data;
    }

    [[nodiscard]] std::string generate_layout_id_hash() const
    {
        std::stringstream ss{};

        ss << lyt.get_layout_name() << qcc::LIBRARY_NAME << tech_impl_name<technology<Lyt>> << num_magnets
           << bb.get_x_size() << bb.get_y_size();

        const auto pin_data = get_pin_data();
        std::for_each(pin_data.cbegin(), pin_data.cend(),
                      [&ss](auto&& pdata) { ss << std::forward<decltype(pdata)>(pdata); });

        const auto hash_fragment = std::hash<std::string>()(ss.str());

        return fmt::format("{0:<020}13{0:<020}37{0:<020}", hash_fragment);
    }

    void write_header()
    {
        os << fmt::format(qcc::VERSION_HEADER, FICTION_VERSION, FICTION_REPO);
        os << fmt::format(qcc::OPEN_QCA_COMPONENT, tech_impl_name<technology<Lyt>>, qcc::LIBRARY_NAME,
                          ps.use_filename_as_component_name ? std::filesystem::path{ps.filename}.stem().string() :
                                                              lyt.get_layout_name(),
                          generate_layout_id_hash(), bb.get_x_size(), bb.get_y_size(), num_magnets);
    }

    void write_entity()
    {
        os << qcc::OPEN_ENTITY;

        for (const auto& pi : sorted_pi_list)
        {
            os << fmt::format(qcc::PIN, tech_impl_name<technology<Lyt>>, lyt.get_cell_name(pi), 0, bb_x(pi), bb_y(pi));
        }
        for (const auto& po : sorted_po_list)
        {
            os << fmt::format(qcc::PIN, tech_impl_name<technology<Lyt>>, lyt.get_cell_name(po), 1, bb_x(po), bb_y(po));
        }
        os << qcc::CLOSE_ENTITY;
    }

    void write_components()
    {
        os << qcc::OPEN_COMPONENTS;
        for (const auto& comp : qcc::COMPONENTS)
        {
            os << fmt::format(qcc::COMPONENT_ITEM, tech_impl_name<technology<Lyt>>, comp);
        }
        os << qcc::CLOSE_COMPONENTS;
    }

    void write_layout()
    {
        std::unordered_set<cell<Lyt>> skip{};

        os << qcc::OPEN_LAYOUT;

        for (decltype(lyt.y()) row = 0; row <= lyt.y(); ++row)
        {
            for (decltype(lyt.x()) col = 0; col <= lyt.x(); ++col)
            {
                const auto c    = cell<Lyt>{col, row};
                const auto type = lyt.get_cell_type(c);

                // skip empty cells and cells marked as to be skipped as well (duh...)
                if (lyt.is_empty_cell(c) || skip.count(c) > 0u)
                {
                    continue;
                }

                // if an AND or an OR structure is encountered, the next two magnets in southern direction need to
                // be skipped
                if (type == inml_technology::cell_type::SLANTED_EDGE_UP_MAGNET ||
                    type == inml_technology::cell_type::SLANTED_EDGE_DOWN_MAGNET)
                {
                    skip.insert({c.x, c.y + 1});
                    skip.insert({c.x, c.y + 2});
                }
                // if a coupler is encountered, skip all magnets relating to the fan-out structure
                else if (type == inml_technology::cell_type::FANOUT_COUPLER_MAGNET)
                {
                    skip.insert({c.x, c.y + 1});
                    skip.insert({c.x, c.y + 2});
                    skip.insert({c.x + 1, c.y});
                    skip.insert({c.x + 1, c.y + 2});
                }
                // if a cross wire is encountered, skip all magnets relating to the crossing structure
                else if (type == inml_technology::cell_type::CROSSWIRE_MAGNET)
                {
                    skip.insert({c.x + 2, c.y});
                    skip.insert({c.x, c.y + 2});
                    skip.insert({c.x + 1, c.y + 1});
                    skip.insert({c.x + 2, c.y + 2});
                }
                // inverters are single structures taking up 4 magnets in the library, so skip the next 3 if
                // encountered one
                else if (type == inml_technology::cell_type::INVERTER_MAGNET)
                {
                    skip.insert({c.x + 1, c.y});
                    skip.insert({c.x + 2, c.y});
                    skip.insert({c.x + 3, c.y});
                }

                if (const auto it = qcc::COMPONENT_SELECTOR.find(type); it != qcc::COMPONENT_SELECTOR.end())
                {
                    os << fmt::format(qcc::OPEN_LAYOUT_ITEM, it->second, bb_x(c), bb_y(c));
                }
                else
                {
                    std::cout << fmt::format("[w] cell at position {} has an unsupported type", c) << std::endl;
                }

                os << fmt::format(qcc::LAYOUT_ITEM_PROPERTY, qcc::PROPERTY_PHASE, lyt.get_clock_number(c));

                if (type == inml_technology::cell_type::INVERTER_MAGNET)
                {
                    os << fmt::format(qcc::LAYOUT_ITEM_PROPERTY, qcc::PROPERTY_LENGTH, 4);
                }

                os << qcc::CLOSE_LAYOUT_ITEM;
            }
        }
        os << qcc::CLOSE_LAYOUT;
    }
};

}  // namespace detail

/**
 * Writes a cell-level iNML layout to a qcc file that is used by ToPoliNano & MagCAD (https://topolinano.polito.it/),
 * an EDA tool and a physical simulator for the iNML technology platform.
 *
 * This overload uses an output stream to write into.
 *
 * @tparam Lyt Cell-level iNML layout type.
 * @param lyt The layout to be written.
 * @param os The output stream to write into.
 * @param ps Parameters.
 */
template <typename Lyt>
void write_qcc_layout(const Lyt& lyt, std::ostream& os, write_qcc_layout_params ps = {})
{
    static_assert(is_cell_level_layout_v<Lyt>, "Lyt is not a cell-level layout");
    static_assert(has_inml_technology_v<Lyt>, "Lyt must be an iNML layout");

    detail::write_qcc_layout_impl p{lyt, os, ps};

    p.run();
}
/**
 * Writes a cell-level iNML layout to a qcc file that is used by ToPoliNano & MagCAD (https://topolinano.polito.it/),
 * an EDA tool and a physical simulator for the iNML technology platform.
 *
 * This overload uses a file name to create and write into.
 *
 * @tparam Lyt Cell-level iNML layout type.
 * @param lyt The layout to be written.
 * @param filename The file name to create and write into. Should preferably use the ".qcc" extension.
 * @param ps Parameters.
 */
template <typename Lyt>
void write_qcc_layout(const Lyt& lyt, const std::string& filename, write_qcc_layout_params ps = {})
{
    std::ofstream os{filename.c_str(), std::ofstream::out};

    if (!os.is_open())
    {
        throw std::ofstream::failure("could not open file");
    }

    ps.filename = filename;

    write_qcc_layout(lyt, os, ps);
    os.close();
}

}  // namespace fiction

#endif  // FICTION_WRITE_QCC_LAYOUT_HPP
