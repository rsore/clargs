#ifndef CLARGS_ARGUMENT_QUEUE_HPP
#define CLARGS_ARGUMENT_QUEUE_HPP

#include <algorithm>
#include <span>
#include <stdexcept>
#include <string_view>

namespace CLArgs
{
    class ArgumentQueue
    {
    public:
        ArgumentQueue(std::size_t argument_count, char **arguments);

        [[nodiscard]] std::size_t size() const noexcept;
        [[nodiscard]] bool        empty() const noexcept;

        [[nodiscard]] std::string_view front();
        [[nodiscard]] std::string_view dequeue();

    private:
        std::span<char *> arguments_{};
        std::size_t       cursor_{0};
    };
} // namespace CLArgs

inline CLArgs::ArgumentQueue::ArgumentQueue(const std::size_t argument_count, char **arguments) : arguments_(arguments, argument_count)
{
    if (arguments == nullptr)
    {
        throw std::invalid_argument("Cannot pass nullptr as arguments to ArgumentQueue constructor");
    }

    if (std::any_of(arguments_.begin(), arguments_.end(), [](const char *arg) { return arg == nullptr; }))
    {
        throw std::invalid_argument("None of the strings passed in arguments array to ArgumentQueue constructor can be nullptr");
    }
}

inline std::size_t
CLArgs::ArgumentQueue::size() const noexcept
{
    return arguments_.size() - cursor_;
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
