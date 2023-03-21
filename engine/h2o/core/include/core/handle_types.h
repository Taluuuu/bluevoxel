#pragma once

#include "core/types.h"

#include <vector>

namespace h2o
{
    template<class OwningType>
    class OwningHandle
    {
    public:

        OwningHandle(OwningType* value = nullptr)
            : m_value(value)
        {
            m_observer_count = new i32;
            (*m_observer_count) = 0;
        }

        template<class OtherType>
        OwningHandle(OwningHandle<OtherType>&& other)
        {
            static_assert(std::is_base_of_v<OwningType, OtherType>);

            m_value = other.m_value;
            m_observer_count = other.m_observer_count;

            other.m_value = nullptr;
            other.m_observer_count = nullptr;
        }

        OwningHandle(const OwningHandle&) = delete;

        ~OwningHandle()
        {
            if (!m_observer_count)
                return;

            if ((*m_observer_count) == 0)
            {
                delete m_observer_count;
            }
            else
            {
                // There are still weak handles.
                (*m_observer_count) = 0;
            }

            delete m_value;
        }

        [[nodiscard]] bool is_valid() const
        {
            return m_value != nullptr;
        }

        operator bool() const
        {
            return is_valid();
        }

        OwningType* operator->() const
        {
            assert(is_valid());
            return m_value;
        }

        OwningType& operator*() const
        {
            assert(is_valid());
            return *m_value;
        }

    private:

        template<class Any>
        friend class WeakHandle;

        template<class Any>
        friend class OwningHandle;

        // Lifetime is strictly inferior to this class'
        OwningType* m_value = nullptr;

        // Lifetime is dictated by the last WeakHandle to this handle, or
        // this handle's destruction if there are no weak handles remaining.
        i32* m_observer_count = nullptr;

    };

    template<class WeakType>
    class WeakHandle
    {
    public:

        WeakHandle() = default;

        template<class OwningType>
        WeakHandle(const OwningHandle<OwningType>& handle)
        {
            m_value = dynamic_cast<WeakType*>(handle.m_value);
            m_observer_count = handle.m_observer_count;

            if (handle.m_value && !m_value)
            {
                // TODO: Create a base object class for the engine where every type has a name
                //       for easier debugging.
                log::warn("Failed cast from WeakType to OwningType.");

                m_value = nullptr;
                m_observer_count = nullptr;

                return;
            }

            if (m_observer_count)
                (*m_observer_count)++;
        }

        WeakHandle(const WeakHandle<WeakType>& handle)
        {
            m_value = handle.m_value;
            m_observer_count = handle.m_observer_count;

            if (m_observer_count)
                (*m_observer_count)++;
        }

        WeakHandle(std::nullptr_t)
        {
            m_value = nullptr;
            m_observer_count = nullptr;
        }

        WeakHandle(WeakHandle&& other)
        {
            m_value = other.m_value;
            m_observer_count = other.m_observer_count;

            other.m_value = nullptr;
            other.m_observer_count = nullptr;

            // No need to increment the observer count
        }

        ~WeakHandle()
        {
            if (m_observer_count && *m_observer_count == 0)
                delete m_observer_count;
        }

        [[nodiscard]] bool is_valid() const
        {
            return
                m_observer_count != nullptr
                && (*m_observer_count) > 0;
        }

        WeakHandle<WeakType>& operator=(const WeakHandle<WeakType>& other)
        {
            if (this == &other || other.m_value == m_value)
                return *this;

            if (is_valid())
                (*m_observer_count)--;

            m_value = other.m_value;
            m_observer_count = other.m_observer_count;

            (*m_observer_count)++;

            return *this;
        }

        operator bool() const
        {
            return is_valid();
        }

        WeakType* operator->() const
        {
            assert(is_valid());
            return m_value;
        }

        WeakType& operator*() const
        {
            assert(is_valid());
            return *m_value;
        }

    private:

        WeakType* m_value = nullptr;

        i32* m_observer_count = nullptr;

    };
}