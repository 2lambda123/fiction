//
// Created by marcel on 08.07.22.
//

#ifndef PYFICTION_WRITE_SVG_LAYOUT_HPP
#define PYFICTION_WRITE_SVG_LAYOUT_HPP

#include "pybind11/pybind11.h"
#include "pyfiction/types.hpp"

#include <fiction/io/write_svg_layout.hpp>

namespace pyfiction
{

inline void write_svg_layout(pybind11::module& m)
{
    namespace py = pybind11;
    using namespace pybind11::literals;

    py::class_<fiction::write_qca_layout_svg_params>(m, "write_qca_layout_svg_params")
        .def(py::init<>())
        .def_readwrite("simple", &fiction::write_qca_layout_svg_params::simple)

        ;

    void (*write_svg_layout_function_pointer)(const py_qca_layout&, const std::string&,
                                              fiction::write_qca_layout_svg_params) =
        &fiction::write_qca_layout_svg<py_qca_layout>;

    m.def("write_qca_layout_svg", write_svg_layout_function_pointer, "layout"_a, "filename"_a,
          "params"_a = fiction::write_qca_layout_svg_params{});
}

}  // namespace pyfiction

#endif  // PYFICTION_WRITE_SVG_LAYOUT_HPP
