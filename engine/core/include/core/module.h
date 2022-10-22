#pragma once

// Typical use
//
// namespace engine
// {
//     class ExampleModule : public Module<ExampleModule>
//     {
//     protected:

//         friend class Module<ExampleModule>;

//         static bool init_impl();
//         static bool cleanup_impl();

//     };
// }

namespace engine
{
    template<typename T>
    class Module
    {
    public:
        
        static void init()
        {
            s_initialized = T::init_impl();
        }

        static void cleanup()
        {
            T::cleanup_impl();
            s_initialized = false;
        }

        bool is_initialized()
        {
            return s_initialized;
        }

    private:

        static inline bool s_initialized = false;

    };
}