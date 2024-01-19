#pragma once

#include <string>
#include <unordered_set>

namespace h2o
{
    class BoolLock
    {
    public:

        void lock(const std::string& reason);
        void unlock(const std::string& reason);

        [[nodiscard]] operator bool() const { return !m_reasons.empty(); }

    private:

        std::unordered_set<std::string> m_reasons{};

    };
}