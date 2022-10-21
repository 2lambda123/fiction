//
// Created by sophia on 12/30/18.
//

#ifndef FICTION_WRITE_SVG_LAYOUT_HPP
#define FICTION_WRITE_SVG_LAYOUT_HPP

#include "fiction/layouts/coordinates.hpp"
#include "fiction/traits.hpp"
#include "utils/version_info.hpp"

#include <fmt/format.h>

#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fiction
{

struct write_qca_layout_svg_params
{
    bool simple = false;
};

template <typename Coordinate>
class unsupported_cell_type_exception : public std::exception
{
  public:
    explicit unsupported_cell_type_exception(const Coordinate& c) noexcept : std::exception(), coord{c} {}

    [[nodiscard]] Coordinate where() const noexcept
    {
        return coord;
    }

  private:
    const Coordinate coord;
};

namespace detail
{

namespace svg
{

// General layout
inline constexpr const double VIEWBOX_DISTANCE = 11;
inline constexpr const double TILE_DISTANCE    = 119;
inline constexpr const double CELL_DISTANCE    = 23;

// Generic tiles
inline constexpr const int    STARTING_OFFSET_TILE_X = -175;
inline constexpr const int    STARTING_OFFSET_TILE_Y = -58;
inline constexpr const double STARTING_OFFSET_CELL_X = 3.5;
inline constexpr const double STARTING_OFFSET_CELL_Y = -40.25;

// Latch tiles
inline constexpr const double STARTING_OFFSET_LATCH_X      = -483.5;
inline constexpr const int    STARTING_OFFSET_LATCH_Y      = -57;
inline constexpr const int    STARTING_OFFSET_LATCH_CELL_X = 312;
inline constexpr const int    STARTING_OFFSET_LATCH_CELL_Y = -41;

// Colors
inline constexpr const char* CLOCK_ZONE_1_CELL     = "86e291";
inline constexpr const char* CLOCK_ZONE_2_CELL     = "ffa5fa";
inline constexpr const char* CLOCK_ZONE_3_CELL     = "00c8bc";
inline constexpr const char* CLOCK_ZONE_4_CELL     = "ffffff";
inline constexpr const char* CLOCK_ZONE_LATCH_CELL = "ffe33a";
inline constexpr const char* CLOCK_ZONE_1_TILE     = "ffffff";
inline constexpr const char* CLOCK_ZONE_2_TILE     = "bfbfbf";
inline constexpr const char* CLOCK_ZONE_3_TILE     = "7f7f7f";
inline constexpr const char* CLOCK_ZONE_4_TILE     = "3f3f3f";
inline constexpr const char* CLOCK_ZONE_12_TEXT    = "000000";
inline constexpr const char* CLOCK_ZONE_34_TEXT    = "ffffff";
inline constexpr const char* PI_CELL               = "008dc8";
inline constexpr const char* PO_CELL               = "e28686";

// SVG parts with placeholders
inline constexpr const char* HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                                      "<!-- Generated by {} ({}) -->\n"
                                      "\n"
                                      "<svg\n"
                                      "xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
                                      "xmlns:cc=\"http://creativecommons.org/ns#\"\n"
                                      "xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
                                      "xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
                                      "xmlns=\"http://www.w3.org/2000/svg\"\n"
                                      "viewBox=\"0 0 {} {}\"\n"
                                      "version=\"1.1\">\n"
                                      "<metadata>\n"
                                      "<rdf:RDF>\n"
                                      "<cc:Work\n"
                                      "rdf:about=\"\">\n"
                                      "<dc:format>image/svg+xml</dc:format>\n"
                                      "<dc:type\n"
                                      "rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />\n"
                                      "<dc:title></dc:title>\n"
                                      "</cc:Work>\n"
                                      "</rdf:RDF>\n"
                                      "</metadata>\n"
                                      "<g>\n"
                                      "{}\n"
                                      "</g>\n"
                                      "</svg>";

inline constexpr const char* TILE =
    "<g\n"
    "transform=\"translate({0},{1})\">\n"
    "<g>\n"
    "<rect\n"
    "ry=\"1.4357216\"\n"
    "y=\"69.390663\"\n"
    "x=\"186.11467\"\n"
    "height=\"118.80357\"\n"
    "width=\"118.80357\"\n"
    "style=\"fill:#{2};stroke:#000000;\" />\n"
    "<text\n"
    "y=\"179.25412\"\n"
    "x=\"288.74234\"\n"
    "style=\"font-style:normal;font-weight:normal;font-size:14.5px;line-height:125%;font-"
    "family:'Noto Sans';fill:#{4};stroke:none;\"\n"
    "xml:space=\"preserve\"><tspan\n"
    "y=\"179.25412\"\n"
    "x=\"288.74234\">{5}</tspan></text>\n"
    "</g>\n"
    "<g>\n"
    "{3}\n"
    "</g>\n"
    "</g>";

inline constexpr const char* LATCH =
    "<g\n"
    "transform=\"translate({0},{1})\">\n"
    "<g>\n"
    "<g>\n"
    "<g>\n"
    "<path\n"
    "d=\"m 613.38686,68.413109 0,118.803571 -118.80358,0\"\n"
    "style=\"color:#000000;solid-color:#000000;solid-opacity:1;fill:#{2};fill-opacity:1;fill-rule:nonzero;stroke:none;"
    "stroke-width:1.19643486;stroke-opacity:1;enable-background:accumulate\" />\n"
    "<path\n"
    "d=\"m 494.58328,187.21668 0,-118.80357 118.80358,0\"\n"
    "style=\"color:#000000;solid-color:#000000;solid-opacity:1;fill:#{3};fill-opacity:1;fill-rule:nonzero;stroke:none;"
    "stroke-width:1.19643486;stroke-opacity:1;enable-background:accumulate\" />\n"
    "</g>\n"
    "<rect\n"
    "style=\"color:#000000;solid-color:#000000;solid-opacity:1;fill:none;fill-opacity:1;fill-rule:nonzero;stroke:#"
    "000000;stroke-width:1.19643486;stroke-opacity:1;enable-background:accumulate\"\n"
    "width=\"118.80357\"\n"
    "height=\"118.80357\"\n"
    "x=\"494.58328\"\n"
    "y=\"68.413109\"\n"
    "ry=\"1.4357216\" />\n"
    "</g>\n"
    "<text\n"
    "y=\"86.340652\"\n"
    "x=\"504.43588\"\n"
    "style=\"font-style:normal;font-weight:normal;font-size:12.5px;line-height:125%;font-family:'Noto "
    "Sans';letter-spacing:0px;word-spacing:0px;fill:#{5};fill-opacity:1;\"\n"
    "xml:space=\"preserve\"><tspan\n"
    "y=\"86.340652\"\n"
    "x=\"504.43588\">{6}</tspan></text>\n"
    "<text\n"
    "y=\"178.27962\"\n"
    "x=\"596.48468\"\n"
    "style=\"font-style:normal;font-weight:normal;font-size:12.5px;line-height:125%;font-family:'Noto "
    "Sans';letter-spacing:0px;word-spacing:0px;fill:#{7};fill-opacity:1;\"\n"
    "xml:space=\"preserve\"><tspan\n"
    "y=\"178.27962\"\n"
    "x=\"596.48468\">{8}</tspan></text>\n"
    "</g>\n"
    "<g>\n"
    "{4}\n"
    "</g>\n"
    "</g>";

inline constexpr const char* CELL = "<g\n"
                                    "style=\"fill:#{0};\"\n"
                                    "transform=\"translate({1},{2})\">\n"
                                    "<rect\n"
                                    "style=\"fill:#{0};stroke:#000000;\"\n"
                                    "width=\"20\"\n"
                                    "height=\"20\"\n"
                                    "x=\"185.86807\"\n"
                                    "y=\"112.92032\"\n"
                                    "ry=\"1.5\" />\n"
                                    "<g\n"
                                    "style=\"fill:#{0};\"\n"
                                    "transform=\"translate(-0.22642517,-0.09405518)\">\n"
                                    "<circle\n"
                                    "r=\"2.7314816\"\n"
                                    "cx=\"191.0945\"\n"
                                    "cy=\"118.01437\"\n"
                                    "style=\"opacity:1;fill:#{0};stroke:#000000;\" />\n"
                                    "<circle\n"
                                    "r=\"2.7314816\"\n"
                                    "cx=\"191.0945\"\n"
                                    "cy=\"128.01437\"\n"
                                    "style=\"opacity:1;fill:#{0};stroke:#000000;\" />\n"
                                    "<circle\n"
                                    "r=\"2.7314816\"\n"
                                    "cx=\"201.0945\"\n"
                                    "cy=\"128.01437\"\n"
                                    "style=\"opacity:1;fill:#{0};stroke:#000000;\" />\n"
                                    "<circle\n"
                                    "r=\"2.7314816\"\n"
                                    "cx=\"201.0945\"\n"
                                    "cy=\"118.01437\"\n"
                                    "style=\"opacity:1;fill:#{0};stroke:#000000;\" />\n"
                                    "</g>\n"
                                    "</g>";

inline constexpr const char* CONST0 = "<g\n"
                                      "style=\"fill:#000000;fill-opacity:1\"\n"
                                      "transform=\"translate({1},{2})\">\n"
                                      "<rect\n"
                                      "style=\"fill:#000000;stroke:#000000;\"\n"
                                      "width=\"20\"\n"
                                      "height=\"20\"\n"
                                      "x=\"185.86807\"\n"
                                      "y=\"112.92032\"\n"
                                      "ry=\"1.5\" />\n"
                                      "<g\n"
                                      "style=\"fill:#000000;fill-opacity:1\"\n"
                                      "transform=\"translate(-0.22642517,-0.09405518)\">\n"
                                      "<circle\n"
                                      "r=\"2.7314816\"\n"
                                      "cx=\"191.0945\"\n"
                                      "cy=\"118.01437\"\n"
                                      "style=\"fill:#ffffff;stroke:#ffffff;\" />\n"
                                      "<circle\n"
                                      "r=\"2.7314816\"\n"
                                      "cx=\"201.0945\"\n"
                                      "cy=\"128.01437\"\n"
                                      "style=\"fill:#ffffff;stroke:#ffffff;\" />\n"
                                      "</g>\n"
                                      "</g>";

inline constexpr const char* CONST1 = "<g\n"
                                      "style=\"fill:#000000;\"\n"
                                      "transform=\"translate({1},{2})\">\n"
                                      "<rect\n"
                                      "style=\"fill:#000000;stroke:#000000;\"\n"
                                      "width=\"20\"\n"
                                      "height=\"20\"\n"
                                      "x=\"185.86807\"\n"
                                      "y=\"112.92032\"\n"
                                      "ry=\"1.5\" />\n"
                                      "<g\n"
                                      "style=\"fill:#000000;\"\n"
                                      "transform=\"translate(-0.22642517,-0.09405518)\">\n"
                                      "<circle\n"
                                      "r=\"2.7314816\"\n"
                                      "cx=\"191.0945\"\n"
                                      "cy=\"128.01437\"\n"
                                      "style=\"fill:#ffffff;stroke:#ffffff;\" />\n"
                                      "<circle\n"
                                      "r=\"2.7314816\"\n"
                                      "cx=\"201.0945\"\n"
                                      "cy=\"118.01437\"\n"
                                      "style=\"fill:#ffffff;stroke:#ffffff;\" />\n"
                                      "</g>\n"
                                      "</g>";

inline constexpr const char* VIA = "<g\n"
                                   "style=\"fill:#{0};\"\n"
                                   "transform=\"translate({1},{2})\">\n"
                                   "<g>\n"
                                   "<rect\n"
                                   "transform=\"rotate(90)\"\n"
                                   "ry=\"1.5\"\n"
                                   "y=\"-205.86807\"\n"
                                   "x=\"112.92032\"\n"
                                   "height=\"20\"\n"
                                   "width=\"20\"\n"
                                   "style=\"fill:#{0};stroke:#000000;\" />\n"
                                   "<circle\n"
                                   "r=\"7.446404\"\n"
                                   "transform=\"rotate(90)\"\n"
                                   "cx=\"122.92032\"\n"
                                   "cy=\"-195.86807\"\n"
                                   "style=\"fill:none;stroke:#000000;\" />\n"
                                   "</g>\n"
                                   "</g>";

inline constexpr const char* CROSS =
    "<g\n"
    "style=\"fill:#{0};fill-opacity:1\"\n"
    "transform=\"translate({1},{2})\">\n"
    "<g>\n"
    "<rect\n"
    "transform=\"rotate(90)\"\n"
    "ry=\"1.5\"\n"
    "y=\"-205.86807\"\n"
    "x=\"112.92032\"\n"
    "height=\"20\"\n"
    "width=\"20\"\n"
    "style=\"fill:#{0};stroke:#000000;\" />\n"
    "<g\n"
    "style=\"stroke-width:1.05832684;stroke-miterlimit:4;stroke-dasharray:none\"\n"
    "transform=\"matrix(0.94695856,0,0,0.94341063,31.332271,8.4394994)\">\n"
    "<path\n"
    "style=\"stroke:#000000;\"\n"
    "d=\"m 165.8261,113.42213 c 15.83125,15.8311 15.8515,15.85136 15.8515,15.85136\"/>\n"
    "<path\n"
    "style=\"stroke:#000000;\"\n"
    "d=\"m 181.6776,113.42213 c -15.83124,15.83111 -15.85149,15.85137 -15.85149,15.85137\"/>\n"
    "</g>\n"
    "</g>\n"
    "</g>";

inline constexpr const char* SIMPLE_CELL = "<g\n"
                                           "transform=\"translate({1},{2})\">\n"
                                           "<rect\n"
                                           "ry=\"1.5\"\n"
                                           "y=\"112.92032\"\n"
                                           "x=\"185.86807\"\n"
                                           "height=\"20\"\n"
                                           "width=\"20\"\n"
                                           "style=\"fill:#{0};stroke:#000000;\" />\n"
                                           "</g>";

}  // namespace svg

template <typename Lyt>
class write_qca_layout_svg_impl
{
  public:
    write_qca_layout_svg_impl(const Lyt& layout, std::ostream& stream, write_qca_layout_svg_params p) :
            lyt{layout},
            os{stream},
            ps{p}
    {}

    void run()
    {
        if (lyt.get_tile_size_x() == 1 && lyt.get_tile_size_y() == 1)
        {
            generate_cell_based_svg();
        }
        else
        {
            generate_tile_based_svg();
        }
    }

  private:
    Lyt lyt;

    std::ostream& os;

    write_qca_layout_svg_params ps;

    /**
     * Alias for an SVG description of a tile containing also its clock zone.
     */
    using tile_description = std::pair<std::string, typename Lyt::clock_number_t>;
    /**
     * Maps coordinates of tiles to their string representation and their clock zone.
     */
    using coord_to_tile_mapping = std::unordered_map<coordinate<Lyt>, tile_description>;
    /**
     * Maps coordinates of tiles to a string representation of the cells contained within them.
     */
    using coord_to_cell_list_mapping = std::unordered_map<coordinate<Lyt>, std::string>;
    /**
     * Alias for an SVG description of a latch tile containing also its clock zone and its offset.
     */
    using svg_latch = std::tuple<std::string, typename Lyt::clock_number_t, uint32_t>;
    /**
     * Maps coordinates of latch tiles to tuples containing their string representation, their clock zone,
     * and their latch offset.
     */
    using coord_to_latch_mapping = std::unordered_map<coordinate<Lyt>, svg_latch>;

    std::pair<std::string, std::string> generate_description_color(const cell<Lyt>& c)
    {
        std::string cell_description, cell_color{};

        static constexpr const std::array<const char*, 4> cell_colors{
            {svg::CLOCK_ZONE_1_CELL, svg::CLOCK_ZONE_2_CELL, svg::CLOCK_ZONE_3_CELL, svg::CLOCK_ZONE_4_CELL}};

        bool is_sync_elem = false;

        if (lyt.is_empty_cell(c))
        {
            // skip empty cells
        }
        else if (const auto ct = lyt.get_cell_type(c); Lyt::technology::is_normal_cell(ct))
        {
            if constexpr (has_synchronization_elements_v<Lyt>)
            {
                if (lyt.is_synchronization_element(c))
                {
                    cell_color = svg::CLOCK_ZONE_LATCH_CELL;

                    is_sync_elem = true;
                }
            }
            if (!is_sync_elem)
            {
                cell_color = cell_colors[lyt.get_clock_number(c)];
            }

            if (ps.simple)
            {
                cell_description = svg::SIMPLE_CELL;
            }
            else if (c.z != 0 && Lyt::technology::is_vertical_cell_mode(lyt.get_cell_mode(c)))
            {
                cell_description = svg::VIA;
            }
            else if (const auto ac = lyt.above(c), bc = lyt.below(c);
                     ((c != ac) && !lyt.is_empty_cell(ac)) || ((c != bc) && lyt.is_empty_cell(bc)))
            {
                cell_description = svg::CROSS;
            }
            else if (c.z != 0)
            {
                // skip non-ground layer cells
            }
            else
            {
                cell_description = svg::CELL;
            }
        }
        else if (Lyt::technology::is_input_cell(ct))
        {
            cell_color       = svg::PI_CELL;
            cell_description = ps.simple ? svg::SIMPLE_CELL : svg::CELL;
        }
        else if (Lyt::technology::is_output_cell(ct))
        {
            cell_color       = svg::PO_CELL;
            cell_description = ps.simple ? svg::SIMPLE_CELL : svg::CELL;
        }
        else if (Lyt::technology::is_const_0_cell(ct))
        {
            cell_color       = "#000000";
            cell_description = ps.simple ? svg::SIMPLE_CELL : svg::CONST0;
        }
        else if (Lyt::technology::is_const_1_cell(ct))
        {
            cell_color       = "#000000";
            cell_description = ps.simple ? svg::SIMPLE_CELL : svg::CONST1;
        }
        else
        {
            throw unsupported_cell_type_exception(c);
        }

        return std::make_pair(cell_description, cell_color);
    }

    /**
     * Returns an SVG string representing the given cell-based clocked cell layout
     *
     * @param fcl The cell layout to generate an SVG representation for.
     * @param simple Flag to indicate that the SVG representation should be generated with less details. Recommended
     *               for large layouts.
     * @return The SVG string containing a visual representation of the given layout.
     */
    void generate_cell_based_svg()
    {
        std::stringstream cell_descriptions{};

        lyt.foreach_cell_position(
            [this, &cell_descriptions](const auto& c)
            {
                if (!lyt.is_empty_cell(c))
                {
                    // Determines cell type and color
                    const auto desc_col = generate_description_color(c);

                    bool is_sync_elem = false;
                    // Current cell-description can now be appended to the description of all cells
                    if constexpr (has_synchronization_elements_v<Lyt>)
                    {
                        if (lyt.is_synchronization_element(c))
                        {
                            cell_descriptions
                                << fmt::format(desc_col.first, desc_col.second,
                                               svg::STARTING_OFFSET_TILE_X + svg::STARTING_OFFSET_LATCH_CELL_X +
                                                   c.x * svg::CELL_DISTANCE,
                                               svg::STARTING_OFFSET_TILE_Y + svg::STARTING_OFFSET_LATCH_CELL_Y +
                                                   c.y * svg::CELL_DISTANCE);

                            is_sync_elem = true;
                        }
                    }
                    if (!is_sync_elem)
                    {
                        cell_descriptions << fmt::format(
                            desc_col.first, desc_col.second,
                            svg::STARTING_OFFSET_TILE_X + svg::STARTING_OFFSET_CELL_X + c.x * svg::CELL_DISTANCE,
                            svg::STARTING_OFFSET_TILE_Y + svg::STARTING_OFFSET_CELL_Y + c.y * svg::CELL_DISTANCE);
                    }
                }
            });

        const double viewbox_x = 2 * svg::VIEWBOX_DISTANCE + static_cast<double>(lyt.x() + 1) * svg::CELL_DISTANCE;
        const double viewbox_y = 2 * svg::VIEWBOX_DISTANCE + static_cast<double>(lyt.y() + 1) * svg::CELL_DISTANCE;

        os << fmt::format(svg::HEADER, FICTION_VERSION, FICTION_REPO, viewbox_x, viewbox_y, cell_descriptions.str());
    }

    /**
     * Returns an SVG string representing the given tile-based clocked cell layout.
     *
     * @param fcl The cell layout to generate an SVG representation for.
     * @param simple Flag to indicate that the SVG representation should be generated with less details. Recommended
     *               for large layouts.
     * @return The SVG string containing a visual representation of the given layout.
     */
    void generate_tile_based_svg()
    {
        // Collects ALL tile-descriptions
        std::stringstream tile_descriptions{};

        // Used for generating tile-descriptions with information about the tile's coordinates and clock zone
        // It is needed because cells may not be in "tile-order" when read from a cell layout
        coord_to_tile_mapping      coord_to_tile{};
        coord_to_cell_list_mapping coord_to_cells{};
        coord_to_latch_mapping     coord_to_latch_tile{};
        coord_to_cell_list_mapping coord_to_latch_cells{};

        // Used to determine the color of cells, tiles and text based on its clock zone
        static constexpr const std::array<const char*, 4> tile_colors{
            {svg::CLOCK_ZONE_1_TILE, svg::CLOCK_ZONE_2_TILE, svg::CLOCK_ZONE_3_TILE, svg::CLOCK_ZONE_4_TILE}},
            text_colors{
                {svg::CLOCK_ZONE_12_TEXT, svg::CLOCK_ZONE_12_TEXT, svg::CLOCK_ZONE_34_TEXT, svg::CLOCK_ZONE_34_TEXT}};

        // Adds all non-empty cells from the layout to their correct tiles; it generates the "body"
        // of all the tile-descriptions to be used later
        lyt.foreach_cell_position(
            [this, &coord_to_tile, &coord_to_cells, &coord_to_latch_cells, &coord_to_latch_tile](const auto& c)
            {
                const auto clock_zone = lyt.get_clock_number(c);
                const auto tile_coords =
                    coordinate<Lyt>{std::ceil(c.x / lyt.get_tile_size_x()), std::ceil(c.y / lyt.get_tile_size_y())};
                std::string current_cells{};

                bool is_sync_elem = false;

                if constexpr (has_synchronization_elements_v<Lyt>)
                {
                    if (const auto latch_delay = lyt.get_synchronization_element(c); latch_delay > 0)
                    {
                        if (auto latch_it = coord_to_latch_cells.find(tile_coords);
                            latch_it != coord_to_latch_cells.end())
                        {
                            current_cells = latch_it->second;
                        }
                        else
                        {
                            // If this is called then there is no tile for the current cell yet
                            // It also makes sure that all required tiles are created
                            coord_to_latch_tile[tile_coords] = {svg::LATCH, clock_zone,
                                                                static_cast<uint32_t>(latch_delay)};
                        }

                        is_sync_elem = true;
                    }
                }

                if (!is_sync_elem)
                {
                    if (auto cell_it = coord_to_cells.find(tile_coords); cell_it != coord_to_cells.end())
                    {
                        current_cells = cell_it->second;
                    }
                    else
                    {
                        // If this is called, then there is no tile for the current cell yet
                        // It also makes sure that all required tiles are created
                        coord_to_tile[tile_coords] = {svg::TILE, clock_zone};
                    }
                }

                // Represent the x- and y-coordinates inside the c's tile
                const coordinate<Lyt> in_tile{c.x % lyt.get_tile_size_x(), c.y % lyt.get_tile_size_y()};

                // Determines cell type and color
                const auto desc_col = generate_description_color(c);

                // Only add cell description if the cell is not empty
                if (!(lyt.is_empty_cell(c)))
                {
                    //  Current cell-description can now be appended to the description of all cells in the current tile
                    if constexpr (has_synchronization_elements_v<Lyt>)
                    {
                        if (const auto latch_delay = lyt.get_synchronization_element(c); latch_delay > 0)
                        {
                            coord_to_latch_cells[tile_coords] = current_cells.append(
                                fmt::format(desc_col.first, desc_col.second,
                                            svg::STARTING_OFFSET_LATCH_CELL_X + in_tile.x * svg::CELL_DISTANCE,
                                            svg::STARTING_OFFSET_LATCH_CELL_Y + in_tile.y * svg::CELL_DISTANCE));

                            is_sync_elem = true;
                        }
                    }

                    if (!is_sync_elem)
                    {
                        coord_to_cells[tile_coords] = current_cells.append(
                            fmt::format(desc_col.first, desc_col.second,
                                        svg::STARTING_OFFSET_CELL_X + in_tile.x * svg::CELL_DISTANCE,
                                        svg::STARTING_OFFSET_CELL_Y + in_tile.y * svg::CELL_DISTANCE));
                    }
                }
            });

        // All cell-descriptions are done and tiles have been created

        // Delete empty tiles in simple designs
        if (ps.simple)
        {
            std::vector<coordinate<Lyt>> empty_tiles{};

            // Find empty tiles via missing cell-descriptions for their coordinates
            for (const auto& [coord, tdscr] : coord_to_tile)
            {
                if (coord_to_cells.count(coord) == 0)
                {
                    empty_tiles.emplace_back(coord);
                }
            }

            // Delete empty tiles
            for (const auto& coord : empty_tiles)
            {
                coord_to_tile.erase(coord);
            }

            if constexpr (has_synchronization_elements_v<Lyt>)
            {
                std::vector<coordinate<Lyt>> empty_latches{};

                // Find empty latches via missing cell-descriptions for their coordinates
                for (const auto& [coord, ldscr] : coord_to_latch_tile)
                {
                    if (auto cell_it = coord_to_latch_cells.find(coord); cell_it == coord_to_latch_cells.end())
                    {
                        empty_latches.emplace_back(coord);
                    }
                }

                // Delete empty latches
                for (const auto& coord : empty_latches)
                {
                    coord_to_latch_tile.erase(coord);
                }
            }
        }

        // Associate tiles with cell-descriptions now; coordinates of tiles are used for tile- and cell-descriptions
        for (const auto& [coord, tdscr] : coord_to_tile)
        {
            const auto [descr, czone] = tdscr;

            const auto cell_descriptions = coord_to_cells[coord];

            const double x_pos = svg::STARTING_OFFSET_TILE_X + coord.x * svg::TILE_DISTANCE;
            const double y_pos = svg::STARTING_OFFSET_TILE_Y + coord.y * svg::TILE_DISTANCE;

            const auto c_descr =
                fmt::format(descr, x_pos, y_pos, tile_colors[czone], cell_descriptions,
                            ps.simple ? "" : text_colors[czone], ps.simple ? "" : std::to_string(czone + 1));

            tile_descriptions << c_descr;
        }

        if constexpr (has_synchronization_elements_v<Lyt>)
        {
            // Add the descriptions of latch-tiles to the whole image
            for (const auto& [coord, ldscr] : coord_to_latch_tile)
            {
                const auto [descr, czone_up, latch_delay] = ldscr;
                const auto czone_lo                       = czone_up + latch_delay % lyt.num_clocks();

                const auto cell_descriptions = coord_to_latch_cells[coord];

                const double x_pos = svg::STARTING_OFFSET_LATCH_X + coord.x * svg::TILE_DISTANCE;
                const double y_pos = svg::STARTING_OFFSET_LATCH_Y + coord.y * svg::TILE_DISTANCE;

                const auto t_descr =
                    fmt::format(descr, x_pos, y_pos, tile_colors[czone_lo], tile_colors[czone_up], cell_descriptions,
                                text_colors[czone_up], ps.simple ? "" : std::to_string(czone_up + 1),
                                text_colors[czone_lo], ps.simple ? "" : std::to_string(czone_lo + 1));

                tile_descriptions << t_descr;
            }
        }

        const coordinate<Lyt> length = {(lyt.x() + 1) / lyt.get_tile_size_x(), (lyt.y() + 1) / lyt.get_tile_size_y()};

        const double viewbox_x = 2 * svg::VIEWBOX_DISTANCE + length.x * svg::TILE_DISTANCE;
        const double viewbox_y = 2 * svg::VIEWBOX_DISTANCE + length.y * svg::TILE_DISTANCE;

        os << fmt::format(svg::HEADER, FICTION_VERSION, FICTION_REPO, viewbox_x, viewbox_y, tile_descriptions.str());
    }
};

}  // namespace detail

/**
 * Writes an SVG representation of a cell-level QCA layout into an output stream. Both tile- and cell-based layouts are
 * supported. For tile-based layouts, QCA layouts of tile size 5 x 5 are supported exclusively so far.
 *
 * The utilized color scheme is based on the standard scheme used in QCADesigner
 * (https://waluslab.ece.ubc.ca/qcadesigner/).
 *
 * May throw an 'unsupported_cell_type_exception'.
 *
 * @tparam Lyt Cell-level QCA layout type.
 * @param lyt The layout to be written.
 * @param os The output stream to write into.
 * @param ps Parameters.
 */
template <typename Lyt>
void write_qca_layout_svg(const Lyt& lyt, std::ostream& os, write_qca_layout_svg_params ps = {})
{
    static_assert(is_cell_level_layout_v<Lyt>, "Lyt is not a cell-level layout");
    static_assert(has_qca_technology_v<Lyt>, "Lyt must be a QCA layout");
    static_assert(std::is_same_v<coordinate<Lyt>, offset::ucoord_t>, "Lyt must use unsigned Cartesian coordinates");

    detail::write_qca_layout_svg_impl<Lyt> p{lyt, os, ps};

    p.run();
}
/**
 * Writes an SVG representation of a cell-level QCA layout into a file. Both tile- and cell-based layouts are supported.
 * For tile-based layouts, QCA layouts of tile size 5 x 5 are supported exclusively so far.
 *
 * The utilized color scheme is based on the standard scheme used in QCADesigner
 * (https://waluslab.ece.ubc.ca/qcadesigner/).
 *
 * May throw an 'unsupported_cell_type_exception'.
 *
 * @tparam Lyt Cell-level QCA layout type.
 * @param lyt The layout to be written.
 * @param filename The file name to create and write into. Should preferably use the ".svg" extension.
 * @param ps Parameters.
 */
template <typename Lyt>
void write_qca_layout_svg(const Lyt& lyt, const std::string& filename, write_qca_layout_svg_params ps = {})
{
    std::ofstream os{filename.c_str(), std::ofstream::out};
    write_qca_layout_svg(lyt, os, ps);
    os.close();
}

}  // namespace fiction

#endif  // FICTION_WRITE_SVG_LAYOUT_HPP
