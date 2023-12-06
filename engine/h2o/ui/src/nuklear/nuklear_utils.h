#pragma once

#include "nuklear_headers.h"
#include "ui/ui_types.h"

namespace h2o::ui
{
    constexpr struct nk_rect to_nk_rect(const ui::Rect& rect)
    {
        return { rect.position.x, rect.position.y, rect.size.x, rect.size.y };
    }
}