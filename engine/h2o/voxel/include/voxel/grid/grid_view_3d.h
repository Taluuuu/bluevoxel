#pragma once

#include "voxel/voxel_utils.h"

namespace h2o
{
    enum class EViewRelativeTo { ViewCenter, ViewCorner, World };

    // TODO: Place this in another file
    class IGrid3DCell
    {
    public:

        virtual ~IGrid3DCell() = default;

        virtual void init(const v3i& position) = 0;

    };

    template<class CellType>
    class View
    {
    public:

        View(const v3i& view_min, const v3i& view_size);
        virtual ~View() = default;

        [[nodiscard]] v3i center_cell_pos() const { return m_view_min + v3i{ m_view_size } / 2; }
        [[nodiscard]] v3i corner_cell_pos() const { return m_view_min; }
        [[nodiscard]] v3i size() const { return m_view_size; }

        // Cell, cell position in world space
        void for_each_cell(const std::function<void(const CellType&, const v3i&)>& function) const;
        void for_each_cell(const std::function<void(CellType&, const v3i&)>& function);
        [[nodiscard]] bool any_matches(const std::function<bool(const CellType*, const v3i&)>& condition) const;

        [[nodiscard]] const CellType* get(const v3i& position, EViewRelativeTo relative_to = EViewRelativeTo::World) const;
        [[nodiscard]] CellType* get(const v3i& position, EViewRelativeTo relative_to = EViewRelativeTo::World);

        // Should not be used outside Grid3D
        virtual void add_cell(const v3i& cell_pos, CellType& cell);

        [[nodiscard]] v3i get_relative_to_cell_pos(EViewRelativeTo relative_to) const;
        [[nodiscard]] bool in_bounds(const v3i& local_cell_pos) const;

    private:

        [[nodiscard]] size_t to_index(const v3i& local_cell_pos) const;

    private:

        std::vector<CellType*> m_cells{};

        v3i m_view_min{}, m_view_size{};

    };

    template<class CellType>
    View<CellType>::View(const v3i& view_min, const v3i& view_size)
        : m_view_min(view_min), m_view_size(view_size)
    {
        assert(view_size.x > 0 && view_size.y > 0 && view_size.z > 0);
        m_cells.resize(view_size.x * view_size.y * view_size.z, nullptr);
    }

    template<class CellType>
    void View<CellType>::for_each_cell(const std::function<void(const CellType&, const v3i&)>& function) const
    {
        voxel_utils::for_v3i(m_view_min, m_view_min + m_view_size,
            [&](const v3i& cell_pos)
            {
                if (const CellType* cell = get(cell_pos))
                    function(*cell, cell_pos);
            }
        );
    }

    template<class CellType>
    void View<CellType>::for_each_cell(const std::function<void(CellType&, const v3i&)>& function)
    {
        voxel_utils::for_v3i(m_view_min, m_view_min + m_view_size,
            [&](const v3i& cell_pos)
            {
                if (CellType* cell = get(cell_pos))
                    function(*cell, cell_pos);
            }
        );
    }

    template<class CellType>
    bool View<CellType>::any_matches(const std::function<bool(const CellType*, const v3i&)>& condition) const
    {
        for (i32 i = m_view_min.x; i < m_view_min.x + m_view_size.x; i++)
        for (i32 j = m_view_min.y; j < m_view_min.y + m_view_size.y; j++)
        for (i32 k = m_view_min.z; k < m_view_min.z + m_view_size.z; k++)
        {
            const v3i cell_pos{ i, j, k };
            if (condition(get(cell_pos), cell_pos))
                return true;
        }

        return false;
    }

    template<class CellType>
    const CellType* View<CellType>::get(const v3i& position, const EViewRelativeTo relative_to) const
    {
        return const_cast<View*>(this)->get(position, relative_to);
    }

    template<class CellType>
    CellType* View<CellType>::get(const v3i& position, const EViewRelativeTo relative_to)
    {
        const v3i offset = get_relative_to_cell_pos(relative_to) - m_view_min;
        const v3i local_pos = position + offset;
        return in_bounds(local_pos) ? m_cells[to_index(local_pos)] : nullptr;
    }

    template<class CellType>
    void View<CellType>::add_cell(const v3i& cell_pos, CellType& cell)
    {
        const v3i local_cell_pos = cell_pos - m_view_min;
        assert(in_bounds(local_cell_pos));

        m_cells[to_index(local_cell_pos)] = &cell;
    }

    template<class CellType>
    v3i View<CellType>::get_relative_to_cell_pos(const EViewRelativeTo relative_to) const
    {
        switch (relative_to)
        {
        case EViewRelativeTo::ViewCenter:
            return center_cell_pos();
        case EViewRelativeTo::ViewCorner:
            return m_view_min;
        default:
        case EViewRelativeTo::World:
            return v3i{ 0 };
        }
    }

    template<class CellType>
    bool View<CellType>::in_bounds(const v3i& local_cell_pos) const
    {
        return
            local_cell_pos.x >= 0 && local_cell_pos.x < m_view_size.x &&
            local_cell_pos.y >= 0 && local_cell_pos.y < m_view_size.y &&
            local_cell_pos.z >= 0 && local_cell_pos.z < m_view_size.z;
    }

    template<class CellType>
    size_t View<CellType>::to_index(const v3i& local_cell_pos) const
    {
        assert(in_bounds(local_cell_pos));

        return
            local_cell_pos.z * m_view_size.x * m_view_size.y +
            local_cell_pos.y * m_view_size.x +
            local_cell_pos.x;
    }
}
