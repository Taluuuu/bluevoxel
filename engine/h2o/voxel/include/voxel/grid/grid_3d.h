#pragma once

#include "core/events.h"
#include "core/types.h"
#include "voxel/voxel_utils.h"

#include <array>
#include <functional>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace h2o
{
    enum class EViewRelativeTo { ViewCenter, ViewCorner, World };
    struct CellsUpdatedEvent { const std::unordered_set<v3i>& updated_cells{}; };
    struct CellsDeletedEvent { const std::unordered_set<v2i>& deleted_cell_columns{}; };

    class IGrid3DCell
    {
    public:

        virtual ~IGrid3DCell() = default;

        virtual void init(const v3i& position) = 0;

    };

    template<u32 VerticalCellCount, class... CellTypes>
    class Grid3D
    {
    protected:

        template<class CellType>
        struct CellData
        {
            CellType cell{};
            std::shared_mutex mutex{};
        };

        // TODO: Replace this with a tuple of arrays for better cache performance
        using CellTuple = std::tuple<CellData<CellTypes>...>;
        using CellTupleColumn = std::array<CellTuple, VerticalCellCount>;

    public:

        template<class CellType>
        class View
        {
        public:

            View(const v3i& view_min, const v3i& view_size);

            // Cell, cell position in world space
            void for_each_cell(const std::function<void(CellType&, const v3i&)>& function);
            void for_each_cell(const std::function<void(const CellType&, const v3i&)>& function) const;
            bool any_matches(const std::function<bool(const CellType*)>& condition) const;

            [[nodiscard]] v3i center_cell_pos() const { return m_view_min + v3i{ m_view_size } / 2; }
            [[nodiscard]] v3i corner_cell_pos() const { return m_view_min; }
            [[nodiscard]] v3i size() const { return m_view_size; }

            [[nodiscard]] CellType* get(const v3i& position, EViewRelativeTo relative_to = EViewRelativeTo::World);
            [[nodiscard]] const CellType* get(const v3i& position, EViewRelativeTo relative_to = EViewRelativeTo::World) const;

        private:

            friend class Grid3D;
            void add_cell(const v3i& cell_pos, CellType& cell, const std::shared_ptr<CellTupleColumn>& cell_column);

            [[nodiscard]] v3i get_relative_to_cell_pos(EViewRelativeTo relative_to) const;
            [[nodiscard]] bool in_bounds(const v3i& local_cell_pos) const;
            [[nodiscard]] size_t to_index(const v3i& local_cell_pos) const;

        private:

            std::vector<CellType*> m_cells{};

            std::unordered_set< std::shared_ptr<CellTupleColumn> > m_cell_columns{};

            v3i m_view_min{}, m_view_size{};

        };

        template<class CellType>
        void fetch(
            const v3i& position,
            const std::function<void(const CellType*)>& function) const;

        template<class CellType>
        void fetch_mut(
            const v3i& position,
            const std::function<void(CellType*)>& function,
            bool create_if_missing = false);

        template<class CellType>
        void view(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<void(const View<CellType>&)>& function) const;

        template<class CellType>
        void view_mut(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<void(View<CellType>&)>& function,
            bool create_if_missing = false);

        template<class CellType>
        void view_column(
            v2i cell_column_pos,
            const std::function<void(const View<CellType>&)>& function) const;

        template<class CellType>
        void view_column_mut(
            v2i cell_column_pos,
            const std::function<void(View<CellType>&)>& function,
            bool create_if_missing = false);

        [[nodiscard]] bool cell_column_exists(v2i cell_column_pos) const;
        [[nodiscard]] bool cell_exists(const v3i& cell_pos) const;

        void remove_all(const std::function<bool(v2i)>& condition);

        void broadcast_events();
        Event<CellsUpdatedEvent> on_cells_updated{}; // First update is creation
        Event<CellsDeletedEvent> on_cells_deleted{};

    protected:

        static constexpr bool is_valid_cell_y(const i32 cell_y)
        { return cell_y >= 0 && cell_y < VerticalCellCount; }

        template<class CellType>
        void view_impl(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<bool(const v3i&)>& should_add_cell,
            const std::function<void(View<CellType>&)>& function) const;

        template<class CellType>
        void view_mut_impl(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<bool(const v3i&)>& should_add_cell,
            const std::function<void(View<CellType>&)>& function,
            bool create_if_missing = false);

    private:

        [[nodiscard]] std::shared_ptr<CellTupleColumn> find_cell_column(v2i cell_column_pos) const;
        [[nodiscard]] std::shared_ptr<CellTupleColumn> find_or_create_cell_column(v2i cell_column_pos);

        template<class T>
        static void init_cell_element(T& element, const v3i& cell_pos);

        [[nodiscard]] static std::shared_ptr<CellTupleColumn> create_cell_column(v2i cell_column_pos);

        void add_to_updated_cells_list(const std::vector<v3i>& updated_cells);
        void add_to_deleted_cells_list(const std::vector<v2i>& deleted_cells);

    private:

        std::unordered_map<v2i, std::shared_ptr<CellTupleColumn>> m_cells{};
        mutable std::shared_mutex m_cells_mutex{};

        // These are stored to ensure events are called on the correct thread.
        // TODO: Make sure nothing bad happens if a cell is updated and deleted on the same frame
        // TODO: Make an instance of this per type in the tuple
        std::mutex m_updated_cells_mutex{};
        std::unordered_set<v3i> m_updated_cells{};
        std::mutex m_deleted_cell_columns_mutex{};
        std::unordered_set<v2i> m_deleted_cell_columns{};

    };

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::View(const v3i& view_min, const v3i& view_size)
        : m_view_min(view_min), m_view_size(view_size)
    {
        assert(view_size.x > 0 && view_size.y > 0 && view_size.z > 0);
        m_cells.resize(view_size.x * view_size.y * view_size.z, nullptr);
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::for_each_cell(
        const std::function<void(CellType&, const v3i&)>& function)
    {
        voxel_utils::for_v3i(m_view_min, m_view_min + m_view_size,
            [&](const v3i& cell_pos)
            {
                if (CellType* cell = get(cell_pos))
                    function(*cell, cell_pos);
            }
        );
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::for_each_cell(
        const std::function<void(const CellType&, const v3i&)>& function) const
    {
        voxel_utils::for_v3i(m_view_min, m_view_min + m_view_size,
            [&](const v3i& cell_pos)
            {
                if (const CellType* cell = get(cell_pos))
                    function(*cell, cell_pos);
            }
        );
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    bool Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::any_matches(
        const std::function<bool(const CellType*)>& condition) const
    {
        for (const auto cell : m_cells)
        {
            if (condition(cell))
                return true;
        }

        return false;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    CellType* Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::get(
        const v3i& position,
        const EViewRelativeTo relative_to)
    {
        const v3i offset = get_relative_to_cell_pos(relative_to) - m_view_min;
        const v3i local_pos = position + offset;
        return in_bounds(local_pos) ? m_cells[to_index(local_pos)] : nullptr;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    const CellType* Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::get(
        const v3i& position,
        const EViewRelativeTo relative_to) const
    {
        const v3i offset = get_relative_to_cell_pos(relative_to) - m_view_min;
        const v3i local_pos = position + offset;
        return in_bounds(local_pos) ? m_cells[to_index(local_pos)] : nullptr;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::add_cell(
        const v3i& cell_pos, CellType& cell,
        const std::shared_ptr<CellTupleColumn>& cell_column)
    {
        const v3i local_cell_pos = cell_pos - m_view_min;
        assert(in_bounds(local_cell_pos));

        m_cells[to_index(local_cell_pos)] = &cell;
        m_cell_columns.insert(cell_column);
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    v3i Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::get_relative_to_cell_pos(
        const EViewRelativeTo relative_to) const
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

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    bool Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::in_bounds(const v3i& local_cell_pos) const
    {
        return
            local_cell_pos.x >= 0 && local_cell_pos.x < m_view_size.x &&
            local_cell_pos.y >= 0 && local_cell_pos.y < m_view_size.y &&
            local_cell_pos.z >= 0 && local_cell_pos.z < m_view_size.z;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    size_t Grid3D<VerticalCellCount, CellTypes...>::View<CellType>::to_index(const v3i& local_cell_pos) const
    {
        assert(in_bounds(local_cell_pos));

        return
            local_cell_pos.z * m_view_size.x * m_view_size.y +
            local_cell_pos.y * m_view_size.x +
            local_cell_pos.x;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::fetch(
        const v3i& position,
        const std::function<void(const CellType*)>& function) const
    {
        view<CellType>(position, v3i{1},
            [&](const View<CellType>& view)
            {
                function(view.get(position));
            }
        );
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::fetch_mut(
        const v3i& position,
        const std::function<void(CellType*)>& function,
        const bool create_if_missing)
    {
        view_mut<CellType>(position, v3i{1},
            [&](View<CellType>& view)
            {
                function(view.get(position));
            }, create_if_missing
        );
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view(
        const v3i& view_min,
        const v3i& view_size,
        const std::function<void(const View<CellType>&)>& function) const
    {
        view_impl<CellType>(view_min, view_size, [](const v3i&) { return true; }, function);
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_mut(
        const v3i& view_min,
        const v3i& view_size,
        const std::function<void(View<CellType>&)>& function,
        const bool create_if_missing)
    {
        view_mut_impl<CellType>(view_min, view_size, [](const v3i&) { return true; }, function, create_if_missing);
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_column(
        const v2i cell_column_pos,
        const std::function<void(const View<CellType>&)>& function) const
    {
        view<CellType>(
            v3i{ cell_column_pos.x, 0, cell_column_pos.y },
            v3i{ 1, VerticalCellCount, 1 },
            function
        );
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_column_mut(
        const v2i cell_column_pos,
        const std::function<void(View<CellType>&)>& function,
        const bool create_if_missing)
    {
        view_mut<CellType>(
            v3i{ cell_column_pos.x, 0, cell_column_pos.y },
            v3i{ 1, VerticalCellCount, 1 },
            function,
            create_if_missing
        );
    }

    template<u32 VerticalCellCount, class... CellTypes>
    bool Grid3D<VerticalCellCount, CellTypes...>::cell_column_exists(const v2i cell_column_pos) const
    {
        return find_cell_column(cell_column_pos) != nullptr;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    bool Grid3D<VerticalCellCount, CellTypes...>::cell_exists(const v3i& cell_pos) const
    {
        if (cell_pos.y >= 0 && cell_pos.y < VerticalCellCount)
            return cell_column_exists({ cell_pos.x, cell_pos.z });

        return false;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    void Grid3D<VerticalCellCount, CellTypes...>::remove_all(const std::function<bool(v2i)>& condition)
    {
        std::vector<v2i> cell_columns_to_delete{};

        {
            const std::shared_lock loaded_cells_lock{ m_cells_mutex };
            for (const auto& [cell_col_pos, _] : m_cells)
            {
                if (condition(cell_col_pos))
                    cell_columns_to_delete.push_back(cell_col_pos);
            }
        }

        if (cell_columns_to_delete.empty())
            return;

        add_to_deleted_cells_list(cell_columns_to_delete);

        {
            const std::unique_lock loaded_cells_lock{ m_cells_mutex };
            for (const v2i cell_col_pos : cell_columns_to_delete)
                m_cells.erase(cell_col_pos);
        }
    }

    template<u32 VerticalCellCount, class... CellTypes>
    void Grid3D<VerticalCellCount, CellTypes...>::broadcast_events()
    {
        {
            const std::unique_lock lock{ m_deleted_cell_columns_mutex };
            if (!m_deleted_cell_columns.empty())
            {
                on_cells_deleted.broadcast({ m_deleted_cell_columns });
                m_deleted_cell_columns.clear();
            }
        }

        {
            const std::unique_lock lock{ m_updated_cells_mutex };
            if (!m_updated_cells.empty())
            {
                on_cells_updated.broadcast({ m_updated_cells });
                m_updated_cells.clear();
            }
        }
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_impl(
        const v3i& view_min,
        const v3i& view_size,
        const std::function<bool(const v3i&)>& should_add_cell,
        const std::function<void(View<CellType>&)>& function) const
    {
        std::vector< std::shared_lock<std::shared_mutex> > locks{};
        locks.reserve(view_size.x * view_size.y * view_size.z);

        View<CellType> view(view_min, view_size);

        for (i32 i = view_min.x; i < view_min.x + view_size.x; i++)
        for (i32 k = view_min.z; k < view_min.z + view_size.z; k++)
        {
            const auto cell_column = find_cell_column({ i, k });
            if (!cell_column)
                continue;

            for (i32 j = view_min.y; j < view_min.y + view_size.y; j++)
            {
                const v3i cell_pos{ i, j, k };
                if (!should_add_cell(cell_pos) || !is_valid_cell_y(j))
                    continue;

                auto& [cell, mutex] = std::get<CellData<CellType>>((*cell_column)[j]);
                view.add_cell(cell_pos, cell, cell_column);
                locks.emplace_back(mutex);
            }
        }

        function(view);
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_mut_impl(
        const v3i& view_min,
        const v3i& view_size,
        const std::function<bool(const v3i&)>& should_add_cell,
        const std::function<void(View<CellType>&)>& function,
        const bool create_if_missing)
    {
        std::vector< std::unique_lock<std::shared_mutex> > locks{};
        locks.reserve(view_size.x * view_size.y * view_size.z);

        View<CellType> view(view_min, view_size);

        for (i32 i = view_min.x; i < view_min.x + view_size.x; i++)
        for (i32 k = view_min.z; k < view_min.z + view_size.z; k++)
        {
            const auto cell_column = create_if_missing ?
                find_or_create_cell_column({ i, k }) :
                find_cell_column({ i, k });

            if (!cell_column)
                continue;

            for (i32 j = view_min.y; j < view_min.y + view_size.y; j++)
            {
                const v3i cell_pos{ i, j, k };
                if (!should_add_cell(cell_pos) || !is_valid_cell_y(j))
                    continue;

                auto& [cell, mutex] = std::get<CellData<CellType>>((*cell_column)[j]);
                view.add_cell(cell_pos, cell, cell_column);
                locks.emplace_back(mutex);
            }
        }

        function(view);

        std::vector<v3i> updated_cells{};
        view.for_each_cell(
            [&](const CellType&, const v3i& cell_pos)
            {
                updated_cells.push_back(cell_pos);
            }
        );
        add_to_updated_cells_list(updated_cells);
    }

    template<u32 VerticalCellCount, class... CellTypes>
    std::shared_ptr<typename Grid3D<VerticalCellCount, CellTypes...>::CellTupleColumn>
        Grid3D<VerticalCellCount, CellTypes...>::find_cell_column(const v2i cell_column_pos) const
    {
        const std::shared_lock lock { m_cells_mutex };
        if (const auto it = m_cells.find(cell_column_pos); it != m_cells.end())
            return it->second;

        return nullptr;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    std::shared_ptr<typename Grid3D<VerticalCellCount, CellTypes...>::CellTupleColumn>
        Grid3D<VerticalCellCount, CellTypes...>::find_or_create_cell_column(const v2i cell_column_pos)
    {
        if (const auto cell_column = find_cell_column(cell_column_pos))
            return cell_column;

        const std::unique_lock lock { m_cells_mutex };

        const auto [it, success] = m_cells.try_emplace(cell_column_pos, nullptr);
        if (success)
            it->second = create_cell_column(cell_column_pos);

        return it->second;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class T>
    void Grid3D<VerticalCellCount, CellTypes...>::init_cell_element(T& element, const v3i& cell_pos)
    {
        if constexpr (std::is_base_of_v<IGrid3DCell, T>)
        {
            auto& grid_cell = static_cast<IGrid3DCell&>(element);
            grid_cell.init(cell_pos);
        }
    }

    template<u32 VerticalCellCount, class... CellTypes>
    std::shared_ptr<typename Grid3D<VerticalCellCount, CellTypes...>::CellTupleColumn>
        Grid3D<VerticalCellCount, CellTypes...>::create_cell_column(const v2i cell_column_pos)
    {
        const auto cell_column = std::make_shared<CellTupleColumn>();
        for (i32 i = 0; i < VerticalCellCount; i++)
        {
            const v3i cell_pos = v3i{ cell_column_pos.x, i, cell_column_pos.y };
            auto& cell_tuple = (*cell_column)[i];

            std::apply(
                [&](auto&&... args)
                {
                    (init_cell_element(args.cell, cell_pos), ...);
                }, cell_tuple
            );
        }

        return cell_column;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    void Grid3D<VerticalCellCount, CellTypes...>::add_to_updated_cells_list(const std::vector<v3i>& updated_cells)
    {
        {
            const std::unique_lock lock{ m_updated_cells_mutex };
            for (const v3i cell_col_pos : updated_cells)
                m_updated_cells.insert(cell_col_pos);
        }

        {
            const std::unique_lock lock{ m_deleted_cell_columns_mutex };
            for (const v3i& cell_pos : updated_cells)
            {
                erase_if(m_deleted_cell_columns,
                    [&](const v2i cell_col_pos)
                    {
                        return cell_pos.x == cell_col_pos.x && cell_pos.z == cell_col_pos.y;
                    }
                );
            }
        }
    }

    template<u32 VerticalCellCount, class... CellTypes>
    void Grid3D<VerticalCellCount, CellTypes...>::add_to_deleted_cells_list(const std::vector<v2i>& deleted_cells)
    {
        {
            const std::unique_lock lock{ m_deleted_cell_columns_mutex };
            for (const v2i cell_col_pos : deleted_cells)
                m_deleted_cell_columns.insert(cell_col_pos);
        }

        {
            const std::unique_lock lock{ m_updated_cells_mutex };
            for (const v2i cell_col_pos : deleted_cells)
            {
                erase_if(m_updated_cells,
                    [&](const v3i& cell_pos)
                    {
                        return cell_pos.x == cell_col_pos.x && cell_pos.z == cell_col_pos.y;
                    }
                );
            }
        }
    }
}
