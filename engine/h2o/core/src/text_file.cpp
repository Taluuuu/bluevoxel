#include "core/text_file.h"

#include "core/utils.h"

namespace h2o
{
    bool TextFile::load(const fs::path& path)
    {
        if (const auto text = utils::read_file(path))
        {
            m_text = *text;
            return true;
        }

        return false;
    }
}
