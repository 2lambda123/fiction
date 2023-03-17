//
// Created by marcel on 17.01.22.
//

#ifndef FICTION_WRITE_QLL_LAYOUT_HPP
#define FICTION_WRITE_QLL_LAYOUT_HPP

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
#include <vector>

namespace fiction
{

namespace detail
{

namespace qll
{

inline constexpr const char* VERSION_HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                              "<!--Generated by {} ({})-->\n";

inline constexpr const char* OPEN_QCA_LAYOUT  = "<qcalayout>\n";
inline constexpr const char* CLOSE_QCA_LAYOUT = "</qcalayout>\n";

inline constexpr const char* OPEN_TECHNOLOGIES  = "\t<technologies>\n";
inline constexpr const char* CLOSE_TECHNOLOGIES = "\t</technologies>\n";

inline constexpr const char* OPEN_SETTINGS = "\t\t<settings tech=\"{}\">\n";  // technology

inline constexpr const char* GENERAL_SETTINGS =
    "\t\t\t<property name=\"Layoutwidth\" value=\"{}\"/>\n"    // layout width
    "\t\t\t<property name=\"Layoutheight\" value=\"{}\"/>\n"   // layout height
    "\t\t\t<property name=\"layersEnabled\" value=\"{}\"/>\n"  // multi-layer design
    "\t\t\t<property name=\"PhaseNumber\" value=\"{}\"/>\n"    // num clocks
    "\t\t\t<property name=\"CZSequence\" value=\"4\"/>\n";

inline constexpr const char* INML_SETTINGS = "\t\t\t<property name=\"Width\" value=\"60\"/>\n"
                                             "\t\t\t<property name=\"Height\" value=\"90\"/>\n"
                                             "\t\t\t<property name=\"Thickness\" value=\"10\"/>\n"
                                             "\t\t\t<property name=\"VDistance\" value=\"10\"/>\n"
                                             "\t\t\t<property name=\"HDistance\" value=\"25\"/>\n"
                                             "\t\t\t<property name=\"DWEnabled\" value=\"true\"/>\n"
                                             "\t\t\t<property name=\"DWMaxSize\" value=\"6\"/>\n"
                                             "\t\t\t<property name=\"DWDefSize\" value=\"2\"/>\n"
                                             "\t\t\t<property name=\"InterlayerSpace\" value=\"10\"/>\n";

inline constexpr const char* MQCA_SETTINGS = "\t\t\t<property name=\"Intermolecular Distance\" value=\"1000\"/>\n";

inline constexpr const char* CLOSE_SETTINGS = "\t\t</settings>\n";

inline constexpr const char* OPEN_COMPONENTS  = "\t<components>\n";
inline constexpr const char* CLOSE_COMPONENTS = "\t</components>\n";
inline constexpr const char* OPEN_LAYOUT      = "\t<layout>\n";
inline constexpr const char* CLOSE_LAYOUT     = "\t</layout>\n";

inline constexpr const char* INML_COMPONENT_ITEM = "\t\t<item tech=\"{}\" name=\"{}\"/>\n";
inline constexpr const char* MQCA_COMPONENT_ITEM = "\t\t<item tech=\"MolFCN\" name=\"Bisferrocene\"/>\n";

inline constexpr const char* OPEN_INML_LAYOUT_ITEM = "\t\t<item comp=\"{}\" id=\"{}\" x=\"{}\" y=\"{}\">\n";
inline constexpr const char* OPEN_MQCA_LAYOUT_ITEM =
    "\t\t<item comp=\"{}\" id=\"{}\" x=\"{}\" y=\"{}\" layer=\"{}\">\n";

inline constexpr const char* CLOSE_LAYOUT_ITEM = "\t\t</item>\n";

inline constexpr const char* LAYOUT_ITEM_PROPERTY = "\t\t\t<property name=\"{}\" value=\"{}\"/>\n";
inline constexpr const char* PROPERTY_PHASE       = "phase";
inline constexpr const char* PROPERTY_LENGTH      = "length";

inline constexpr const char* PIN =
    "\t\t<pin tech=\"{}\" name=\"{}\" direction=\"{}\" id=\"{}\" x=\"{}\" y=\"{}\" layer=\"{}\"/>\n";

inline constexpr const std::array<const char*, 6> COMPONENTS{"Magnet", "Coupler",  "Cross Wire",
                                                             "And",    "Inverter", "Or"};

static const std::unordered_map<inml_technology::cell_type, uint8_t> INML_COMPONENT_SELECTOR{
    {inml_technology::cell_type::NORMAL, 0},           {inml_technology::cell_type::INPUT, 0},
    {inml_technology::cell_type::OUTPUT, 0},           {inml_technology::cell_type::FANOUT_COUPLER_MAGNET, 1},
    {inml_technology::cell_type::CROSSWIRE_MAGNET, 2}, {inml_technology::cell_type::SLANTED_EDGE_DOWN_MAGNET, 3},
    {inml_technology::cell_type::INVERTER_MAGNET, 4},  {inml_technology::cell_type::SLANTED_EDGE_UP_MAGNET, 5}};

}  // namespace qll

template <typename Lyt>
class write_qll_layout_impl
{
  public:
    write_qll_layout_impl(const Lyt& src, std::ostream& s) :
            lyt{src},
            bb{lyt},
            sorted_pi_list{sorted_pis()},
            sorted_po_list{sorted_pos()},
            os{s}
    {}

    void run()
    {
        if (has_inml_technology_v<Lyt> && !has_border_io_pins())
        {
            throw std::invalid_argument(
                "the layout does not fulfill all requirements to be written as a QLL file because it does not have "
                "designated I/O pins or they are not routed to the layout's borders");
        }

        write_header();

        os << qll::OPEN_QCA_LAYOUT;

        write_technology_settings();
        write_components();
        write_layout();

        // close component block and flush the file
        os << qll::CLOSE_QCA_LAYOUT << std::flush;
    }

  private:
    const Lyt& lyt;

    const bounding_box_2d<Lyt> bb;

    std::vector<cell<Lyt>> sorted_pi_list, sorted_po_list;

    std::ostream& os;

    uint64_t cell_id{1};

    const char* tech_name{has_inml_technology_v<Lyt> ? "iNML" : has_qca_technology_v<Lyt> ? "MolFCN" : "?"};

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

    void write_header()
    {
        os << fmt::format(qll::VERSION_HEADER, FICTION_VERSION, FICTION_REPO);
    }

    void write_technology_settings()
    {
        os << qll::OPEN_TECHNOLOGIES;

        os << fmt::format(qll::OPEN_SETTINGS, tech_name);

        os << fmt::format(qll::GENERAL_SETTINGS, lyt.x(), lyt.y(), (lyt.z() > 0 ? "true" : "false"), lyt.num_clocks());

        if constexpr (has_inml_technology_v<Lyt>)
        {
            os << qll::INML_SETTINGS;
        }
        else if constexpr (has_qca_technology_v<Lyt>)
        {
            os << qll::MQCA_SETTINGS;
        }

        os << qll::CLOSE_SETTINGS;
        os << qll::CLOSE_TECHNOLOGIES;
    }

    void write_components()
    {
        os << qll::OPEN_COMPONENTS;

        if constexpr (has_inml_technology_v<Lyt>)
        {
            for (const auto& comp : qll::COMPONENTS)
            {
                os << fmt::format(qll::INML_COMPONENT_ITEM, tech_name, comp);
            }
        }
        else if constexpr (has_qca_technology_v<Lyt>)
        {
            os << qll::MQCA_COMPONENT_ITEM;
        }

        os << qll::CLOSE_COMPONENTS;
    }

    void write_pins()
    {
        for (const auto& pi : sorted_pi_list)
        {
            os << fmt::format(qll::PIN, tech_name, lyt.get_cell_name(pi), 0, cell_id++, bb_x(pi), bb_y(pi), pi.z);
        }
        for (const auto& po : sorted_po_list)
        {
            os << fmt::format(qll::PIN, tech_name, lyt.get_cell_name(po), 1, cell_id++, bb_x(po), bb_y(po), po.z);
        }
    }

    void write_layout()
    {
        std::unordered_set<cell<Lyt>> skip{};

        os << qll::OPEN_LAYOUT;

        for (decltype(lyt.z()) layer = 0; layer <= lyt.z(); ++layer)
        {
            for (decltype(lyt.y()) row = 0; row <= lyt.y(); ++row)
            {
                for (decltype(lyt.x()) col = 0; col <= lyt.x(); ++col)
                {
                    const auto c    = cell<Lyt>{col, row, layer};
                    const auto type = lyt.get_cell_type(c);

                    // skip empty cells and cells marked as to be skipped as well (duh...)
                    if (lyt.is_empty_cell(c) || skip.count(c) > 0u)
                    {
                        continue;
                    }

                    // write iNML cell
                    if constexpr (has_inml_technology_v<Lyt>)
                    {
                        // if an AND or an OR structure is encountered, the next two magnets in southern direction need
                        // to be skipped
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

                        if (const auto it = qll::INML_COMPONENT_SELECTOR.find(type);
                            it != qll::INML_COMPONENT_SELECTOR.end())
                        {
                            os << fmt::format(qll::OPEN_INML_LAYOUT_ITEM, it->second, cell_id++, bb_x(c), bb_y(c));
                        }
                        else
                        {
                            std::cout << fmt::format("[w] cell at position {} has an unsupported type", c) << std::endl;
                        }

                        os << fmt::format(qll::LAYOUT_ITEM_PROPERTY, qll::PROPERTY_PHASE, lyt.get_clock_number(c));

                        if (type == inml_technology::cell_type::INVERTER_MAGNET)
                        {
                            os << fmt::format(qll::LAYOUT_ITEM_PROPERTY, qll::PROPERTY_LENGTH, 4);
                        }

                        os << qll::CLOSE_LAYOUT_ITEM;
                    }
                    // write mQCA cell
                    else if constexpr (has_qca_technology_v<Lyt>)
                    {
                        const auto mode = lyt.get_cell_mode(c);

                        // write normal cell
                        if (qca_technology::is_normal_cell(type))
                        {
                            os << fmt::format(qll::OPEN_MQCA_LAYOUT_ITEM, 0, cell_id++, bb_x(c), bb_y(c), c.z * 2);
                            os << fmt::format(qll::LAYOUT_ITEM_PROPERTY, qll::PROPERTY_PHASE, lyt.get_clock_number(c));
                            os << qll::CLOSE_LAYOUT_ITEM;
                        }
                        // constant cells are handled as input pins
                        else if (qca_technology::is_constant_cell(type))
                        {
                            const auto const_name = qca_technology::is_const_0_cell(type) ? "const0" : "const1";
                            os << fmt::format(qll::PIN, tech_name, const_name, 0, cell_id++, bb_x(c), bb_y(c), c.z * 2);
                        }

                        // write via cell
                        if (qca_technology::is_vertical_cell_mode(mode) && c.z != lyt.z())
                        {
                            os << fmt::format(qll::OPEN_MQCA_LAYOUT_ITEM, 0, cell_id++, bb_x(c), bb_y(c), c.z * 2 + 1);
                            os << fmt::format(qll::LAYOUT_ITEM_PROPERTY, qll::PROPERTY_PHASE, lyt.get_clock_number(c));
                            os << qll::CLOSE_LAYOUT_ITEM;
                        }
                    }
                }
            }
        }

        // I/O cells are not considered in the cases above because they need to be handled separately
        write_pins();

        os << qll::CLOSE_LAYOUT;
    }
};

}  // namespace detail

/**
 * Writes a cell-level QCA or iNML layout to a qll file that is used by ToPoliNano & MagCAD
 * (https://topolinano.polito.it/), an EDA tool and a physical simulator for the iNML technology platform as well as
 * SCERPA (https://ieeexplore.ieee.org/document/8935211), a physical simulator for the mQCA technology platform.
 *
 * This overload uses an output stream to write into.
 *
 * @tparam Lyt Cell-level QCA or iNML layout type.
 * @param lyt The layout to be written.
 * @param os The output stream to write into.
 * @param ps Parameters.
 */
template <typename Lyt>
void write_qll_layout(const Lyt& lyt, std::ostream& os)
{
    static_assert(is_cell_level_layout_v<Lyt>, "Lyt is not a cell-level layout");
    static_assert(has_inml_technology_v<Lyt> || has_qca_technology_v<Lyt>, "Lyt must be an iNML or a QCA layout");

    detail::write_qll_layout_impl p{lyt, os};

    p.run();
}
/**
 * Writes a cell-level QCA or iNML layout to a qll file that is used by ToPoliNano & MagCAD
 * (https://topolinano.polito.it/), an EDA tool and a physical simulator for the iNML technology platform as well as
 * SCERPA (https://ieeexplore.ieee.org/document/8935211), a physical simulator for the mQCA technology platform.
 *
 * This overload uses a file name to create and write into.
 *
 * @tparam Lyt Cell-level iNML layout type.
 * @param lyt The layout to be written.
 * @param filename The file name to create and write into. Should preferably use the `.qll` extension.
 * @param ps Parameters.
 */
template <typename Lyt>
void write_qll_layout(const Lyt& lyt, const std::string& filename)
{
    std::ofstream os{filename.c_str(), std::ofstream::out};

    if (!os.is_open())
    {
        throw std::ofstream::failure("could not open file");
    }

    write_qll_layout(lyt, os);
    os.close();
}

}  // namespace fiction

#endif  // FICTION_WRITE_QLL_LAYOUT_HPP
