//
// Created by marcel on 07.03.22.
//

#ifndef FICTION_SIDB_SURFACE_HPP
#define FICTION_SIDB_SURFACE_HPP

#include "fiction/technology/cell_technologies.hpp"
#include "fiction/technology/sidb_defects.hpp"
#include "fiction/traits.hpp"

#include <phmap.h>

#include <cassert>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace fiction
{

struct sidb_surface_params
{
    /**
     * Specifies which defects are to be ignored, e.g., when they are not relevant for the current analysis.
     *
     * @note Ignored defects are not stored in the surface instance!
     */
    std::unordered_set<sidb_defect_type> ignore{};
};

/**
 * A layout type to layer on top of any SiDB cell-level layout. It implements an interface to store and access
 * fabrication defects on the H-Si(100) 2x1 surface.
 *
 * @tparam Lyt SiDB cell-level layout.
 * @tparam has_sidb_defect_interface Automatically determines whether a defect interface is already present.
 */
template <typename Lyt,
          bool has_sidb_defect_interface = std::conjunction_v<has_assign_sidb_defect<Lyt>, has_get_sidb_defect<Lyt>>>
class sidb_surface : public Lyt
{};

template <typename Lyt>
class sidb_surface<Lyt, true> : public Lyt
{
  public:
    explicit sidb_surface(const Lyt& lyt, [[maybe_unused]] const sidb_surface_params& ps = {}) : Lyt(lyt) {}
};

template <typename Lyt>
class sidb_surface<Lyt, false> : public Lyt
{
  public:
    struct sidb_surface_storage
    {
        explicit sidb_surface_storage(sidb_surface_params ps = {}) : params(std::move(ps)) {}

        sidb_surface_params params{};

        phmap::parallel_flat_hash_map<typename Lyt::coordinate, sidb_defect> defective_coordinates{};
    };

    using storage = std::shared_ptr<sidb_surface_storage>;

    /**
     * Standard constructor for empty layouts.
     */
    explicit sidb_surface(const sidb_surface_params& ps = {}) : Lyt(), strg{std::make_shared<sidb_surface_storage>(ps)}
    {
        static_assert(is_cell_level_layout_v<Lyt>, "Lyt is not a cell-level layout");
        static_assert(has_sidb_technology_v<Lyt>, "Lyt is not an SiDB layout");

        assert(strg->params.ignore.count(sidb_defect_type::NONE) == 0 && "The defect type 'NONE' cannot be ignored");
    }

    /**
     * Standard constructor that layers the SiDB defect interface onto a layout with aspect ratio ar as input.
     *
     * @param ar aspect ratio of the layout.
     */
    explicit sidb_surface(const typename Lyt::aspect_ratio& ar, const sidb_surface_params& ps = {}) :
            Lyt(ar),
            strg{std::make_shared<sidb_surface_storage>(ps)}
    {
        static_assert(is_cell_level_layout_v<Lyt>, "Lyt is not a cell-level layout");
        static_assert(has_sidb_technology_v<Lyt>, "Lyt is not an SiDB layout");

        assert(strg->params.ignore.count(sidb_defect_type::NONE) == 0 && "The defect type 'NONE' cannot be ignored");
    }

    /**
     * Standard constructor that layers the SiDB defect interface onto an existing layout.
     *
     * @param lyt Existing layout that is to be extended by an SiDB defect interface.
     */
    explicit sidb_surface(const Lyt& lyt, const sidb_surface_params& ps = {}) :
            Lyt(lyt),
            strg{std::make_shared<sidb_surface_storage>(ps)}
    {
        static_assert(is_cell_level_layout_v<Lyt>, "Lyt is not a cell-level layout");
        static_assert(has_sidb_technology_v<Lyt>, "Lyt is not an SiDB layout");

        assert(strg->params.ignore.count(sidb_defect_type::NONE) == 0 && "The defect type 'NONE' cannot be ignored");
    }
    /**
     * Assigns a given defect type to the given coordinate.
     *
     * @param c Coordinate to assign defect d to.
     * @param d Defect to assign to coordinate c.
     */
    void assign_sidb_defect(const typename Lyt::coordinate& c, const sidb_defect& d) noexcept
    {
        if (d.type == sidb_defect_type::NONE)  // delete defect
        {
            strg->defective_coordinates.erase(c);
        }
        else if (strg->params.ignore.count(d.type) == 0)  // add defect if this type is not ignored
        {
            strg->defective_coordinates.insert({c, d});
        }
    }
    /**
     * Returns the given coordinate's assigned defect type. If no defect type has been assigned, NONE is returned.
     *
     * @param c Coordinate to check.
     * @return Defect type previously assigned to c or NONE if no defect was yet assigned.
     */
    [[nodiscard]] sidb_defect get_sidb_defect(const typename Lyt::coordinate& c) const noexcept
    {
        if (auto it = strg->defective_coordinates.find(c); it != strg->defective_coordinates.cend())
        {
            return it->second;
        }

        return sidb_defect{sidb_defect_type::NONE};  // TODO what are default values for NONE sidb_defects?
    }
    /**
     * Returns the number of defective coordinates on the surface.
     *
     * @return Number of defective coordinates.
     */
    [[nodiscard]] uint64_t num_defects() const noexcept
    {
        return strg->defective_coordinates.size();
    }
    /**
     * Applies a function to all defects on the surface. Since the defects are fetched directly from the storage map,
     * the given function has to receive a pair of a coordinate and a defect type as its parameter.
     *
     * @tparam Fn Functor type that has to comply with the restrictions imposed by mockturtle::foreach_element.
     * @param fn Functor to apply to each defect.
     */
    template <typename Fn>
    void foreach_sidb_defect(Fn&& fn) const
    {
        mockturtle::detail::foreach_element(strg->defective_coordinates.cbegin(), strg->defective_coordinates.cend(),
                                            std::forward<Fn>(fn));
    }
    /**
     * Returns all SiDB positions affected by the defect at the given coordinate. This function relies on the
     * defect_extent function defined in sidb_defects.hpp that computes the extent of charged and neutral defect types.
     *
     * If the given coordinate is defect-free, the empty set is returned.
     *
     * @param c Coordinate whose defect extent is to be determined.
     * @return All SiDB positions affected by the defect at coordinate c.
     */
    [[nodiscard]] std::unordered_set<typename Lyt::coordinate>
    affected_sidbs(const typename Lyt::coordinate& c) const noexcept
    {
        std::unordered_set<typename Lyt::coordinate> influenced_sidbs{};

        if (const auto d = get_sidb_defect(c); d.type != sidb_defect_type::NONE)
        {
            const auto [horizontal_extent, vertical_extent] = defect_extent(d);

            for (auto y = static_cast<int64_t>(c.y - vertical_extent); y <= static_cast<int64_t>(c.y + vertical_extent);
                 ++y)
            {
                for (auto x = static_cast<int64_t>(c.x - horizontal_extent);
                     x <= static_cast<int64_t>(c.x + horizontal_extent); ++x)
                {
                    if (const auto affected = typename Lyt::coordinate{x, y, c.z}; Lyt::is_within_bounds(affected))
                    {
                        influenced_sidbs.insert(affected);
                    }
                }
            }
        }

        return influenced_sidbs;
    }
    /**
     * Returns all SiDB positions affected by any defect on the surface. This function relies on the defect_extent
     * function defined in sidb_defects.hpp that computes the extent of charged and neutral defect types.
     *
     * If the given surface is defect-free, the empty set is returned.
     *
     * @return All SiDB positions affected by any defect on the surface.
     */
    [[nodiscard]] std::unordered_set<typename Lyt::coordinate> all_affected_sidbs() const noexcept
    {
        std::unordered_set<typename Lyt::coordinate> influenced_sidbs{};

        foreach_sidb_defect([&influenced_sidbs, this](const auto& it)
                            { influenced_sidbs.merge(affected_sidbs(it.first)); });

        return influenced_sidbs;
    }

  private:
    storage strg;
};

template <class T>
sidb_surface(const T&) -> sidb_surface<T>;

}  // namespace fiction

#endif  // FICTION_SIDB_SURFACE_HPP
