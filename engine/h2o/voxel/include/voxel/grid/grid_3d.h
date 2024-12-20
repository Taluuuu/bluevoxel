#pragma once

#include "core/events.h"
#include "core/types.h"
#include "voxel/grid/grid_view_3d.h"
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
    struct CellsUpdatedEvent { const std::unordered_set<v3i>& updated_cells{}; };
    struct CellsDeletedEvent { const std::unordered_set<v2i>& deleted_cell_columns{}; };

    template<typename CellType>
    concept HasViewType = requires { typename CellType::ViewType; };

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

        template<class CellType>
        using CellColumn = std::array<CellData<CellType>, VerticalCellCount>;
        using CellColumnTuple = std::tuple<CellColumn<CellTypes>...>;

        template<typename CellType, bool = HasViewType<CellType>>
        struct CellViewTypeHelper { using type = typename CellType::ViewType; };
        template<typename CellType> // Fallback for cell types without ViewType
        struct CellViewTypeHelper<CellType, false> { using type = View<CellType>; };
        template<typename CellType>
        using ViewType = typename CellViewTypeHelper<CellType>::type;

    public:

        Grid3D() = default;
        virtual ~Grid3D() = default;

        template<class CellType>
        void fetch(
            const v3i& position,
            const std::function<void(const CellType*)>& function) const;

        template<class CellType>
        void fetch_mut(
            const v3i& position,
            const std::function<void(CellType*)>& function,
            bool create_if_missing = false,
            bool broadcast_update_event = true);

        template<class CellType>
        void view(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<void(const ViewType<CellType>&)>& function) const;

        template<class CellType>
        void view_mut(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<void(ViewType<CellType>&)>& function,
            bool create_if_missing = false,
            bool broadcast_update_event = true);

        template<class CellType>
        void view_column(
            v2i cell_column_pos,
            const std::function<void(const ViewType<CellType>&)>& function) const;

        template<class CellType>
        void view_column_mut(
            v2i cell_column_pos,
            const std::function<void(ViewType<CellType>&)>& function,
            bool create_if_missing = false,
            bool broadcast_update_event = true);

        [[nodiscard]] bool cell_column_exists(v2i cell_column_pos) const;
        [[nodiscard]] bool cell_exists(const v3i& cell_pos) const;

        void remove_all(const std::function<bool(v2i)>& condition);

        template<class CellType>
        [[nodiscard]] Event<CellsUpdatedEvent>& cells_updated_event();
        [[nodiscard]] Event<CellsDeletedEvent>& cells_deleted_event();

        virtual void broadcast_events();

    protected:

        template<class CellType>
        void add_to_updated_cells_list(const std::vector<v3i>& updated_cells);
        void add_to_deleted_cells_list(const std::vector<v2i>& deleted_cells);

        static constexpr bool is_valid_cell_y(const i32 cell_y)
        { return cell_y >= 0 && cell_y < VerticalCellCount; }

        template<class CellType>
        void view_impl(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<bool(const v3i&)>& should_add_cell,
            const std::function<void(const ViewType<CellType>&)>& function) const;

        template<class CellType>
        void view_impl_mut(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<bool(const v3i&)>& should_add_cell,
            const std::function<void(ViewType<CellType>&)>& function,
            bool create_if_missing = false,
            bool broadcast_update_event = true);

        [[nodiscard]] virtual std::shared_ptr<CellColumnTuple> create_cell_column(v2i cell_column_pos);

    private:

        [[nodiscard]] std::shared_ptr<CellColumnTuple> find_cell_column(v2i cell_column_pos) const;
        [[nodiscard]] std::shared_ptr<CellColumnTuple> find_or_create_cell_column(v2i cell_column_pos);

        template<class T>
        static void init_cell_column(CellColumn<T>& cell_column, v2i column_pos);

        // These events are stored to ensure events are called on the correct thread.
        struct CellsUpdateEventData
        {
            Event<CellsUpdatedEvent> cells_update_event{};

            std::mutex updated_cells_mutex{};
            std::unordered_set<v3i> updated_cells{};
        };

        struct CellsDeletedEventData
        {
            Event<CellsDeletedEvent> cells_deleted_event{};

            std::mutex deleted_cell_columns_mutex{};
            std::unordered_set<v2i> deleted_cell_columns{};
        };

        template<class CellType>
        [[nodiscard]] CellsUpdateEventData& get_cells_update_event_data();

    private:

        std::unordered_map<v2i, std::shared_ptr<CellColumnTuple>> m_cells{};
        mutable std::shared_mutex m_cells_mutex{};

        std::array<CellsUpdateEventData, sizeof...(CellTypes)> m_cells_update_events_data;
        CellsDeletedEventData m_cells_deleted_event_data;

    };

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::fetch(
        const v3i& position,
        const std::function<void(const CellType*)>& function) const
    {
        view<CellType>(position, v3i{1},
            [&](const ViewType<CellType>& view)
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
        const bool create_if_missing,
        const bool broadcast_update_event)
    {
        view_mut<CellType>(position, v3i{1},
            [&](ViewType<CellType>& view)
            {
                function(view.get(position));
            }, create_if_missing, broadcast_update_event
        );
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view(
        const v3i& view_min,
        const v3i& view_size,
        const std::function<void(const ViewType<CellType>&)>& function) const
    {
        view_impl<CellType>(view_min, view_size, [](const v3i&) { return true; }, function);
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_mut(
        const v3i& view_min,
        const v3i& view_size,
        const std::function<void(ViewType<CellType>&)>& function,
        const bool create_if_missing,
        const bool broadcast_update_event)
    {
        view_impl_mut<CellType>(
            view_min,
            view_size,
            [](const v3i&) { return true; },
            function,
            create_if_missing,
            broadcast_update_event
        );
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_column(
        const v2i cell_column_pos,
        const std::function<void(const ViewType<CellType>&)>& function) const
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
        const std::function<void(ViewType<CellType>&)>& function,
        const bool create_if_missing,
        const bool broadcast_update_event)
    {
        view_mut<CellType>(
            v3i{ cell_column_pos.x, 0, cell_column_pos.y },
            v3i{ 1, VerticalCellCount, 1 },
            function,
            create_if_missing,
            broadcast_update_event
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
    template<class CellType>
    Event<CellsUpdatedEvent>& Grid3D<VerticalCellCount, CellTypes...>::cells_updated_event()
    {
        return get_cells_update_event_data<CellType>().cells_update_event;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    Event<CellsDeletedEvent>& Grid3D<VerticalCellCount, CellTypes...>::cells_deleted_event()
    {
        return m_cells_deleted_event_data.cells_deleted_event;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    void Grid3D<VerticalCellCount, CellTypes...>::broadcast_events()
    {
        for (auto& [event, mutex, cells] : m_cells_update_events_data)
        {
            const std::unique_lock lock { mutex };
            if (!cells.empty())
            {
                event.broadcast({ cells });
                cells.clear();
            }
        }

        {
            auto& [event, mutex, cells] = m_cells_deleted_event_data;

            const std::unique_lock lock { mutex };
            if (!cells.empty())
            {
                event.broadcast({ cells });
                cells.clear();
            }
        }
    }

    template<u32 VerticalCellCount, class... CellTypes>
    std::shared_ptr<typename Grid3D<VerticalCellCount, CellTypes...>::CellColumnTuple>
        Grid3D<VerticalCellCount, CellTypes...>::find_cell_column(const v2i cell_column_pos) const
    {
        const std::shared_lock lock { m_cells_mutex };
        if (const auto it = m_cells.find(cell_column_pos); it != m_cells.end())
            return it->second;

        return nullptr;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    std::shared_ptr<typename Grid3D<VerticalCellCount, CellTypes...>::CellColumnTuple>
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
    void Grid3D<VerticalCellCount, CellTypes...>::init_cell_column(CellColumn<T>& cell_column, const v2i column_pos)
    {
        if constexpr (std::is_base_of_v<IGrid3DCell, T>)
        {
            for (i32 i = 0; i < VerticalCellCount; i++)
            {
                auto& grid_cell = static_cast<IGrid3DCell&>(cell_column[i].cell);
                grid_cell.init(v3i{ column_pos.x, i, column_pos.y });
            }
        }
    }

    template<u32 VerticalCellCount, class... CellTypes>
    std::shared_ptr<typename Grid3D<VerticalCellCount, CellTypes...>::CellColumnTuple>
        Grid3D<VerticalCellCount, CellTypes...>::create_cell_column(const v2i cell_column_pos)
    {
        const auto cell_column_tuple = std::make_shared<CellColumnTuple>();
        std::apply(
            [&](auto&&... cell_columns)
            {
                (init_cell_column(cell_columns, cell_column_pos), ...);
            }, *cell_column_tuple
        );

        return cell_column_tuple;
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    typename Grid3D<VerticalCellCount, CellTypes...>::CellsUpdateEventData&
        Grid3D<VerticalCellCount, CellTypes...>::get_cells_update_event_data()
    {
        const size_t event_index = utils::index_in_template_list<CellType, CellTypes...>();
        return m_cells_update_events_data[event_index];
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::add_to_updated_cells_list(const std::vector<v3i>& updated_cells)
    {
        {
            CellsUpdateEventData& event_data = get_cells_update_event_data<CellType>();

            const std::unique_lock lock{ event_data.updated_cells_mutex };
            for (const v3i cell_col_pos : updated_cells)
                event_data.updated_cells.insert(cell_col_pos);
        }

        {
            const std::unique_lock lock{ m_cells_deleted_event_data.deleted_cell_columns_mutex };
            for (const v3i& cell_pos : updated_cells)
            {
                erase_if(m_cells_deleted_event_data.deleted_cell_columns,
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
            const std::unique_lock lock{ m_cells_deleted_event_data.deleted_cell_columns_mutex };
            for (const v2i cell_col_pos : deleted_cells)
                m_cells_deleted_event_data.deleted_cell_columns.insert(cell_col_pos);
        }

        {
            for (const v2i cell_col_pos : deleted_cells)
            {
                for (CellsUpdateEventData& event_data : m_cells_update_events_data)
                {
                    const std::unique_lock lock{ event_data.updated_cells_mutex };

                    erase_if(event_data.updated_cells,
                        [&](const v3i& cell_pos)
                        {
                            return cell_pos.x == cell_col_pos.x && cell_pos.z == cell_col_pos.y;
                        }
                    );
                }
            }
        }
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_impl(
        const v3i& view_min,
        const v3i& view_size,
        const std::function<bool(const v3i&)>& should_add_cell,
        const std::function<void(const ViewType<CellType>&)>& function) const
    {
        static_assert(std::is_base_of_v<View<CellType>, ViewType<CellType>>, "ViewType must inherit View<CellType>.");

        std::vector<std::shared_lock<std::shared_mutex>> locks{};
        locks.reserve(view_size.x * view_size.y * view_size.z);

        ViewType<CellType> view(view_min, view_size);

        // Make sure cell columns stay allocated while the view is in use
        std::vector< std::shared_ptr<CellColumnTuple> > cell_columns{};
        cell_columns.reserve(view_size.x * view_size.z);

        for (i32 i = view_min.x; i < view_min.x + view_size.x; i++)
        for (i32 k = view_min.z; k < view_min.z + view_size.z; k++)
        {
            const auto cell_column = find_cell_column({ i, k });
            if (!cell_column)
                continue;

            bool is_cell_column_used = false;
            for (i32 j = view_min.y; j < view_min.y + view_size.y; j++)
            {
                const v3i cell_pos{ i, j, k };
                if (!should_add_cell(cell_pos) || !is_valid_cell_y(j))
                    continue;

                auto& [cell, mutex] = std::get<CellColumn<CellType>>(*cell_column)[j];
                view.add_cell(cell_pos, cell);
                locks.emplace_back(mutex);

                is_cell_column_used = true;
            }

            if (is_cell_column_used)
                cell_columns.emplace_back(cell_column);
        }

        function(view);
    }

    template<u32 VerticalCellCount, class... CellTypes>
    template<class CellType>
    void Grid3D<VerticalCellCount, CellTypes...>::view_impl_mut(
        const v3i& view_min,
        const v3i& view_size,
        const std::function<bool(const v3i&)>& should_add_cell,
        const std::function<void(ViewType<CellType>&)>& function,
        const bool create_if_missing,
        const bool broadcast_update_event)
    {
        static_assert(std::is_base_of_v<View<CellType>, ViewType<CellType>>, "ViewType must inherit View<CellType>.");

        std::vector<v3i> updated_cells{};

        {
            std::vector<std::unique_lock<std::shared_mutex>> locks{};
            locks.reserve(view_size.x * view_size.y * view_size.z);

            ViewType<CellType> view(view_min, view_size);

            // Make sure cell columns stay allocated while the view is in use
            std::vector< std::shared_ptr<CellColumnTuple> > cell_columns{};
            cell_columns.reserve(view_size.x * view_size.z);

            for (i32 i = view_min.x; i < view_min.x + view_size.x; i++)
            for (i32 k = view_min.z; k < view_min.z + view_size.z; k++)
            {
                const auto cell_column = create_if_missing ?
                    find_or_create_cell_column({ i, k }) :
                    find_cell_column({ i, k });

                if (!cell_column)
                    continue;

                bool is_cell_column_used = false;
                for (i32 j = view_min.y; j < view_min.y + view_size.y; j++)
                {
                    const v3i cell_pos{ i, j, k };
                    if (!should_add_cell(cell_pos) || !is_valid_cell_y(j))
                        continue;

                    auto& [cell, mutex] = std::get<CellColumn<CellType> >(*cell_column)[j];
                    view.add_cell(cell_pos, cell);
                    locks.emplace_back(mutex);

                    is_cell_column_used = true;
                }

                if (is_cell_column_used)
                    cell_columns.emplace_back(cell_column);
            }

            function(view);

            if (broadcast_update_event)
            {
                view.for_each_cell(
                    [&](const CellType&, const v3i& cell_pos)
                    {
                        updated_cells.push_back(cell_pos);
                    }
                );
            }
        }

        if (broadcast_update_event)
            add_to_updated_cells_list<CellType>(updated_cells);
    }
}
