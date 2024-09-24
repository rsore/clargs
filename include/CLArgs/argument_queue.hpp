#ifndef CLARGS_ARGUMENT_QUEUE_HPP
#define CLARGS_ARGUMENT_QUEUE_HPP

#include <string_view>
#include <stdexcept>

namespace CLArgs
{
    class ArgumentQueue
    {
    public:
        ArgumentQueue(int argc, char **argv);

        [[nodiscard]] bool
            empty() const noexcept;

    private:
        std::size_t cursor_{0};

        std::size_t number_of_arguments_{0};
        char **arguments_{nullptr};
    };
}

inline
CLArgs::ArgumentQueue::ArgumentQueue(int argc, char **argv)
{
    if (argc < 0)
    {
        throw std::invalid_argument("Cannot parse value less than 0 to ArgumentQueue constructor");
    }

    if (argv == nullptr)
    {
        throw std::invalid_argument("Cannot pass nullptr as argv to ArgumentQueue constructor");
    }

    for (std::size_t i{}; i < static_cast<std::size_t>(argc); ++i)
    {
        if (argv[i] == nullptr)
        {
            throw std::invalid_argument("None of the strings passed in argv array to ArgumentQueue constructor can be nullptr");
        }
    }

    number_of_arguments_ = static_cast<std::size_t>(argc);
    arguments_ = argv;
}

inline bool
CLArgs::ArgumentQueue::empty() const noexcept
{
    return (cursor_ == number_of_arguments_);
}

#endif
