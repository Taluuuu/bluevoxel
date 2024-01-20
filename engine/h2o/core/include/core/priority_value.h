#pragma once

#include <map>
#include <optional>

namespace h2o
{
    template<class PriorityType, class ValueType>
    class PriorityValue
    {
    public:

        void push(PriorityType priority, ValueType value);
        void remove(PriorityType priority);

        [[nodiscard]] std::optional<ValueType> get() const;

    private:

        std::map<PriorityType, ValueType, std::greater<PriorityType>> m_values{};

    };

    template<class PriorityType, class ValueType>
    void PriorityValue<PriorityType, ValueType>::push(PriorityType priority, ValueType value)
    {
        m_values[priority] = value;
    }

    template<class PriorityType, class ValueType>
    void PriorityValue<PriorityType, ValueType>::remove(PriorityType priority)
    {
        m_values.erase(priority);
    }

    template<class PriorityType, class ValueType>
    std::optional<ValueType> PriorityValue<PriorityType, ValueType>::get() const
    {
        const auto max_key_it = m_values.cbegin();
        if (max_key_it == m_values.end())
            return std::nullopt;

        return max_key_it->second;
    }
}