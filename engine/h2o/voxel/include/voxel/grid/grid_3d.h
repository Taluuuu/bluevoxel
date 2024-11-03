#pragma once

#include "grid_view_3d.h"

namespace h2o
{
    template<u32 VerticalCellCount, class... CellTypes>
    class Grid3D
    {
    public:

        // template<class CellType>
        // CellType get()
        // {
        //     CellData<CellType>& temp = std::get<CellData<CellType>>(test);
        //     auto& mmm = temp.mutex;
        //     return temp.cell;
        // }

    private:

        template<class CellType>
        struct CellData
        {
            CellType cell{};
            std::shared_mutex mutex{};
        };

        using CellTuple = std::tuple<CellData<CellTypes>...>;
        using CellTupleColumn = std::array<CellTuple, VerticalCellCount>;

        std::unordered_map<v2i, std::shared_ptr<CellTupleColumn>> m_cells{};
        mutable std::shared_mutex m_cells_mutex{};

    };
}