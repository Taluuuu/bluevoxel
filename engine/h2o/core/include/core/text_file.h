#pragma once

#include "resources.h"

#include <vector>

namespace h2o
{
    class TextFile : public IResource
    {
    public:

        TextFile() = default;

        [[nodiscard]] const std::vector<char>& text() const { return m_text; }

        // IResource interface
        bool load(const fs::path& path) override;

    private:

        std::vector<char> m_text{};

    };
}
