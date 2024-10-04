#pragma once

#include <functional>
#include <memory>

namespace h2o
{
    class BlockTrait;
    class BlockType;

    using BlockTraitBuilder = std::function<std::shared_ptr<BlockTrait>(const BlockType&)>;

    class BlockTraitManager final
    {
    public:

        BlockTraitManager() = default;

        // Returns true on success
        bool add_trait_to_block_type(const std::string& trait_name, BlockType& block_type) const;

        // Templated version for easy registration of C++ types
        // Non-templated version for eventual traits which share the same class but with different names
        // and creation process
        template<class T>
        void register_trait(const std::string& name);
        void register_trait(const std::string& name, const BlockTraitBuilder& builder);

        [[nodiscard]] std::vector<std::string> trait_names() const;

    private:

        std::unordered_map<std::string, BlockTraitBuilder> m_trait_builders{};

    };

    template<class T>
    void BlockTraitManager::register_trait(const std::string& name)
    {
        register_trait(name,
            [name](const BlockType& block_type)
            {
                return std::make_shared<T>(name, block_type);
            }
        );
    }
}
