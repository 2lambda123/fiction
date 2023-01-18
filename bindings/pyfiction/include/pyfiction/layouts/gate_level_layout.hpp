//
// Created by marcel on 04.06.22.
//

#ifndef PYFICTION_GATE_LEVEL_LAYOUT_HPP
#define PYFICTION_GATE_LEVEL_LAYOUT_HPP

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pyfiction/types.hpp"

#include <fiction/io/print_layout.hpp>
#include <fiction/traits.hpp>

#include <fmt/format.h>

#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace pyfiction
{

namespace detail
{

template <typename LytBase, typename GateLyt>
void gate_level_layout(pybind11::module& m, const std::string& topology)
{
    namespace py = pybind11;
    using namespace pybind11::literals;

    py::class_<GateLyt, LytBase>(m, fmt::format("{}_gate_layout", topology).c_str())
        .def(py::init<>())
        .def(py::init<const fiction::aspect_ratio<GateLyt>&>(), "dimension"_a)
        .def(py::init(
                 [](const fiction::aspect_ratio<GateLyt>& dimension, const std::string& scheme_name,
                    const std::string& layout_name) -> GateLyt
                 {
                     if (const auto scheme = fiction::get_clocking_scheme<GateLyt>(scheme_name); scheme.has_value())
                     {
                         return GateLyt{dimension, *scheme, layout_name};
                     }

                     throw std::runtime_error("Given name does not refer to a supported clocking scheme");
                 }),
             "dimension"_a, "clocking_scheme"_a = "2DDWave", "layout_name"_a = "")

        .def("create_pi", &GateLyt::create_pi)
        .def("create_po", &GateLyt::create_po)
        .def("is_pi_tile", &GateLyt::is_pi_tile)
        .def("is_po_tile", &GateLyt::is_po_tile)

        .def("set_layout_name", &GateLyt::set_layout_name)
        .def("get_layout_name", &GateLyt::get_layout_name)
        .def("set_input_name", &GateLyt::set_input_name)
        .def("get_input_name", &GateLyt::get_input_name)
        .def("set_output_name", &GateLyt::set_output_name)
        .def("get_output_name", &GateLyt::get_output_name)

        .def("create_buf", &GateLyt::create_buf)
        .def("create_not", &GateLyt::create_not)
        .def("create_and", &GateLyt::create_and)
        .def("create_nand", &GateLyt::create_nand)
        .def("create_or", &GateLyt::create_or)
        .def("create_nor", &GateLyt::create_nor)
        .def("create_xor", &GateLyt::create_xor)
        .def("create_xnor", &GateLyt::create_xnor)
        .def("create_maj", &GateLyt::create_maj)

        .def("num_pis", &GateLyt::num_pis)
        .def("num_pos", &GateLyt::num_pos)
        .def("num_gates", &GateLyt::num_gates)
        .def("num_wires", &GateLyt::num_wires)
        .def("is_empty", &GateLyt::is_empty)

        .def("fanin_size", &GateLyt::fanin_size)
        .def("fanout_size", &GateLyt::fanout_size)

        .def("get_node",
             [](const GateLyt& layout, const py_coordinate& coordinate) { return layout.get_node(coordinate); })
        .def("get_tile", &GateLyt::get_tile)
        .def("make_signal", &GateLyt::make_signal)

        .def("clear_tile", &GateLyt::clear_tile)

        .def("is_gate_tile", &GateLyt::is_gate_tile)
        .def("is_wire_tile", &GateLyt::is_wire_tile)
        .def("is_empty_tile", &GateLyt::is_empty_tile)

        .def("pis",
             [](const GateLyt& lyt)
             {
                 std::vector<fiction::tile<GateLyt>> pis{};
                 pis.reserve(lyt.num_pis());
                 lyt.foreach_pi([&pis, &lyt](const auto& pi) { pis.push_back(lyt.get_tile(pi)); });
                 return pis;
             })
        .def("pos",
             [](const GateLyt& lyt)
             {
                 std::vector<fiction::tile<GateLyt>> pos{};
                 pos.reserve(lyt.num_pos());
                 lyt.foreach_po([&pos, &lyt](const auto& po) { pos.push_back(lyt.get_tile(lyt.get_node(po))); });
                 return pos;
             })
        .def("gates",
             [](const GateLyt& lyt)
             {
                 std::vector<fiction::tile<GateLyt>> gates{};
                 gates.reserve(lyt.num_gates());
                 lyt.foreach_gate([&gates, &lyt](const auto& g) { gates.push_back(lyt.get_tile(g)); });
                 return gates;
             })
        .def("wires",
             [](const GateLyt& lyt)
             {
                 std::vector<fiction::tile<GateLyt>> wires{};
                 wires.reserve(lyt.num_wires());
                 lyt.foreach_wire([&wires, &lyt](const auto& w) { wires.push_back(lyt.get_tile(w)); });
                 return wires;
             })

        .def("fanins", &GateLyt::incoming_data_flow)
        .def("fanouts", &GateLyt::outgoing_data_flow)

        .def("is_incoming_signal", &GateLyt::is_incoming_signal)
        .def("has_northern_incoming_signal", &GateLyt::has_northern_incoming_signal)
        .def("has_north_eastern_incoming_signal", &GateLyt::has_north_eastern_incoming_signal)
        .def("has_eastern_incoming_signal", &GateLyt::has_eastern_incoming_signal)
        .def("has_south_eastern_incoming_signal", &GateLyt::has_south_eastern_incoming_signal)
        .def("has_southern_incoming_signal", &GateLyt::has_southern_incoming_signal)
        .def("has_south_western_incoming_signal", &GateLyt::has_south_western_incoming_signal)
        .def("has_western_incoming_signal", &GateLyt::has_western_incoming_signal)
        .def("has_north_western_incoming_signal", &GateLyt::has_north_western_incoming_signal)

        .def("is_outgoing_signal", &GateLyt::is_outgoing_signal)
        .def("has_northern_outgoing_signal", &GateLyt::has_northern_outgoing_signal)
        .def("has_north_eastern_outgoing_signal", &GateLyt::has_north_eastern_outgoing_signal)
        .def("has_eastern_outgoing_signal", &GateLyt::has_eastern_outgoing_signal)
        .def("has_south_eastern_outgoing_signal", &GateLyt::has_south_eastern_outgoing_signal)
        .def("has_southern_outgoing_signal", &GateLyt::has_southern_outgoing_signal)
        .def("has_south_western_outgoing_signal", &GateLyt::has_south_western_outgoing_signal)
        .def("has_western_outgoing_signal", &GateLyt::has_western_outgoing_signal)
        .def("has_north_western_outgoing_signal", &GateLyt::has_north_western_outgoing_signal)

        .def("__repr__",
             [](const GateLyt& lyt) -> std::string
             {
                 std::stringstream stream{};
                 fiction::print_gate_level_layout(stream, lyt);
                 return stream.str();
             })

        ;
}

}  // namespace detail

inline void gate_level_layouts(pybind11::module& m)
{
    /**
     * Gate-level clocked Cartesian layout.
     */
    detail::gate_level_layout<py_cartesian_clocked_layout, py_cartesian_gate_layout>(m, "cartesian");
    /**
     * Gate-level clocked hexagonal layout.
     */
    detail::gate_level_layout<py_hexagonal_clocked_layout, py_hexagonal_gate_layout>(m, "hexagonal");
}

}  // namespace pyfiction

#endif  // PYFICTION_GATE_LEVEL_LAYOUT_HPP
