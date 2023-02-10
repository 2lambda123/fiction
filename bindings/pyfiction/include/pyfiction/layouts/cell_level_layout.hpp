//
// Created by marcel on 04.06.22.
//

#ifndef PYFICTION_CELL_LEVEL_LAYOUT_HPP
#define PYFICTION_CELL_LEVEL_LAYOUT_HPP

#include "pyfiction/types.hpp"

#include <fiction/technology/cell_technologies.hpp>
#include <fiction/traits.hpp>

#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <cctype>
#include <set>
#include <string>
#include <type_traits>

namespace pyfiction
{

namespace detail
{

template <typename Technology>
void fcn_technology_cell_level_layout(pybind11::module& m)
{
    namespace py = pybind11;
    using namespace pybind11::literals;

    // fetch technology name
    auto tech_name = std::string{fiction::tech_impl_name<Technology>};
    std::transform(tech_name.begin(), tech_name.end(), tech_name.begin(), ::tolower);

    /**
     * FCN cell technology.
     */
    const py::class_<Technology> tech(m, fmt::format("{}_technology", tech_name).c_str());

    py::enum_<typename Technology::cell_type> cell_type(tech, "cell_type");

    cell_type.value("EMPTY", Technology::cell_type::EMPTY);
    cell_type.value("NORMAL", Technology::cell_type::NORMAL);
    cell_type.value("INPUT", Technology::cell_type::INPUT);
    cell_type.value("OUTPUT", Technology::cell_type::OUTPUT);

    if constexpr (std::is_same_v<Technology, fiction::qca_technology>)
    {
        cell_type.value("CONST_0", Technology::cell_type::CONST_0);
        cell_type.value("CONST_1", Technology::cell_type::CONST_1);
    }
    else if constexpr (std::is_same_v<Technology, fiction::inml_technology>)
    {
        cell_type.value("SLANTED_EDGE_UP_MAGNET", Technology::cell_type::SLANTED_EDGE_UP_MAGNET);
        cell_type.value("SLANTED_EDGE_DOWN_MAGNET", Technology::cell_type::SLANTED_EDGE_DOWN_MAGNET);
        cell_type.value("INVERTER_MAGNET", Technology::cell_type::INVERTER_MAGNET);
        cell_type.value("CROSSWIRE_MAGNET", Technology::cell_type::CROSSWIRE_MAGNET);
        cell_type.value("FANOUT_COUPLER_MAGNET", Technology::cell_type::FANOUT_COUPLER_MAGNET);
    }
    else if constexpr (std::is_same_v<Technology, fiction::sidb_technology>)
    {
        // no additional cell types required
    }
    // NOTE: more technologies go here

    using py_cartesian_technology_cell_layout = py_cartesian_cell_layout<Technology>;

    /**
     * Cell-level clocked Cartesian layout.
     */
    py::class_<py_cartesian_technology_cell_layout,
               fiction::synchronization_element_layout<fiction::clocked_layout<
                   fiction::tile_based_layout<fiction::cartesian_layout<fiction::offset::ucoord_t>>>>>(
        m, fmt::format("{}_layout", tech_name).c_str())
        .def(py::init<>())
        .def(py::init<const fiction::aspect_ratio<py_cartesian_technology_cell_layout>&>(), "dimension"_a)
        .def(py::init(
                 [](const fiction::aspect_ratio<py_cartesian_technology_cell_layout>& dimension,
                    const std::string&                                                scheme_name,
                    const std::string& layout_name) -> py_cartesian_technology_cell_layout
                 {
                     if (const auto scheme =
                             fiction::get_clocking_scheme<py_cartesian_technology_cell_layout>(scheme_name);
                         scheme.has_value())
                     {
                         return py_cartesian_technology_cell_layout{dimension, *scheme, layout_name};
                     }

                     throw std::runtime_error("Given name does not refer to a supported clocking scheme");
                 }),
             "dimension"_a, "clocking_scheme"_a = "2DDWave", "layout_name"_a = "")

        .def("assign_cell_type", &py_cartesian_technology_cell_layout::assign_cell_type)
        .def("get_cell_type", &py_cartesian_technology_cell_layout::get_cell_type)
        .def("is_empty_cell", &py_cartesian_technology_cell_layout::is_empty_cell)
        .def("get_layout_name", &py_cartesian_technology_cell_layout::get_layout_name)
        .def("assign_cell_name", &py_cartesian_technology_cell_layout::assign_cell_name)
        .def("get_cell_name", &py_cartesian_technology_cell_layout::get_cell_name)
        .def("num_cells", &py_cartesian_technology_cell_layout::num_cells)
        .def("is_empty", &py_cartesian_technology_cell_layout::is_empty)
        .def("num_pis", &py_cartesian_technology_cell_layout::num_pis)
        .def("num_pos", &py_cartesian_technology_cell_layout::num_pos)
        .def("is_pi", &py_cartesian_technology_cell_layout::is_pi)
        .def("is_po", &py_cartesian_technology_cell_layout::is_po)

        .def("cells",
             [](const py_cartesian_technology_cell_layout& lyt)
             {
                 std::vector<fiction::coordinate<py_cartesian_technology_cell_layout>> cells{};
                 cells.reserve(lyt.num_cells());
                 lyt.foreach_cell([&cells](const auto& c) { cells.push_back(c); });
                 return cells;
             })
        .def("pis",
             [](const py_cartesian_technology_cell_layout& lyt)
             {
                 std::vector<fiction::coordinate<py_cartesian_technology_cell_layout>> pis{};
                 pis.reserve(lyt.num_pis());
                 lyt.foreach_pi([&pis](const auto& c) { pis.push_back(c); });
                 return pis;
             })
        .def("pos",
             [](const py_cartesian_technology_cell_layout& lyt)
             {
                 std::vector<fiction::coordinate<py_cartesian_technology_cell_layout>> pos{};
                 pos.reserve(lyt.num_pos());
                 lyt.foreach_po([&pos](const auto& c) { pos.push_back(c); });
                 return pos;
             })

        ;
}

}  // namespace detail

inline void cell_level_layouts(pybind11::module& m)
{
    detail::fcn_technology_cell_level_layout<fiction::qca_technology>(m);
    detail::fcn_technology_cell_level_layout<fiction::inml_technology>(m);
    detail::fcn_technology_cell_level_layout<fiction::sidb_technology>(m);
}

}  // namespace pyfiction

#endif  // PYFICTION_CELL_LEVEL_LAYOUT_HPP
