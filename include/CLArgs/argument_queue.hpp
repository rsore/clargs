#ifndef CLARGS_ARGUMENT_QUEUE_HPP
#define CLARGS_ARGUMENT_QUEUE_HPP

#include <stdexcept>
#include <string_view>

namespace CLArgs
{
    class ArgumentQueue
    {
    public:
        ArgumentQueue(int argc, char **argv);

        [[nodiscard]] std::size_t size() const noexcept;
        [[nodiscard]] bool        empty() const noexcept;

        [[nodiscard]] std::string_view front();
        [[nodiscard]] std::string_view dequeue();

    private:
        std::size_t cursor_{0};

        std::size_t number_of_arguments_{0};
        char      **arguments_{nullptr};
    };
} // namespace CLArgs

inline CLArgs::ArgumentQueue::ArgumentQueue(int argc, char **argv)
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
    arguments_           = argv;
}

inline std::size_t
CLArgs::ArgumentQueue::size() const noexcept
{
    return number_of_arguments_ - cursor_;
}

inline bool
CLArgs::ArgumentQueue::empty() const noexcept
{
    return size() == 0;
}

inline std::string_view
CLArgs::ArgumentQueue::front()
{
    if (empty())
    {
        throw std::logic_error("Cannot call front() on empty ArgumentQueue, consider checking emptiness first using empty()");
    }

    return {arguments_[cursor_]};
}

inline std::string_view
CLArgs::ArgumentQueue::dequeue()
{
    if (empty())
    {
        throw std::logic_error("Cannot call dequeue() on empty ArgumentQueue, consider checking emptiness first using empty()");
    }

    return {arguments_[cursor_++]};
}

#endif
