#pragma once

#include <vector>

namespace h2o
{
    template<class T>
    class CachedRef
    {
        CachedRef(T& ref);
        CachedRef(const CachedRef&) = delete;
        CachedRef(CachedRef&&) = delete;
        ~CachedRef();
    };

    template<class T>
    class CachedRefOwner
    {
    private:

        std::vector<CachedRef<T>*> m_cached_refs;

    };
}