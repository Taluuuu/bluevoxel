#pragma once

#include <memory>

namespace engine
{
    class InstanceVk;

    class PhysicalDeviceVk
    {
    public:

        [[nodiscard]] static PhysicalDeviceVk* create(const std::shared_ptr<InstanceVk>& instance);

    private:

        // PhysicalDeviceVk()

    private:



        const std::shared_ptr<InstanceVk> m_instance;

    };
}