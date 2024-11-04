#pragma once

#include "grid_view_3d.h"

namespace h2o
{
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

        using CellTuple = std::tuple<CellData<CellTypes>...>;
        using CellTupleColumn = std::array<CellTuple, VerticalCellCount>;

    public:

        template<class CellType>
        class GridView3D
        {
        public:

            void add_cell(CellType& cell, const std::shared_ptr<CellTupleColumn>& cell_column);

        private:

            std::vector<CellType*> m_cells{};

            std::unordered_set< std::shared_ptr<CellTupleColumn> > m_cell_columns{};

            v3i m_view_min{}, m_view_size{};

        };

        // template<class CellType>
        // CellType get()
        // {
        //     CellData<CellType>& temp = std::get<CellData<CellType>>(test);
        //     auto& mmm = temp.mutex;
        //     return temp.cell;
        // }

        template<class CellType>
        void view(
            const v3i& view_min,
            const v3i& view_size,
            const std::function<void(const GridView3D<CellType>&)>& function);

        void view_mut();

    private:

        std::unordered_map<v2i, std::shared_ptr<CellTupleColumn>> m_cells{};
        mutable std::shared_mutex m_cells_mutex{};

    };
}