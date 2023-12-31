.. _ortho:

Scalable Orthogonal Physical Design
-----------------------------------

**Header:** ``fiction/algorithms/physical_design/orthogonal.hpp``

Utilizes approximations to the graph-theoretical problem of *Orthogonal Graph Drawing* to generate FCN gate-level
layouts from logic network specifications. This approach is scalable but requires that the input network is restricted
to a 3-graph. At the same time, the output layout will always be 2DDWave-clocked and has a large area overhead.

.. doxygenstruct:: fiction::orthogonal_physical_design_params
   :members:
.. doxygenfunction:: fiction::orthogonal(const Ntk& ntk, orthogonal_physical_design_params ps = {}, orthogonal_physical_design_stats* pst = nullptr)
