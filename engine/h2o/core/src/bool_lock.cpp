#include "core/bool_lock.h"

namespace h2o
{
    void BoolLock::lock(const std::string& reason)
    {
        m_reasons.insert(reason);
    }

    void BoolLock::unlock(const std::string& reason)
    {
        m_reasons.erase(reason);
    }
}