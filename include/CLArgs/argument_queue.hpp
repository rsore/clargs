#ifndef CLARGS_ARGUMENT_QUEUE_HPP
#define CLARGS_ARGUMENT_QUEUE_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string_view>

namespace CLArgs
{
    class ArgumentQueue
    {
        class Iterator;

    public:
        ArgumentQueue(int argc, const char *const *argv);

        [[nodiscard]] std::size_t size() const noexcept;
        [[nodiscard]] bool        empty() const noexcept;

        [[nodiscard]] std::string_view front() const;

        [[nodiscard]] std::string_view dequeue();
        template <std::size_t N>
        [[nodiscard]] std::array<std::string_view, N> dequeue();

        [[nodiscard]] Iterator begin() const;
        [[nodiscard]] Iterator end() const;

    private:
        std::span<const char *const> arguments_{};
        std::size_t                  cursor_{0};
    };

    class ArgumentQueue::Iterator
    {
    public:
        using value_type        = std::string_view;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type *; // Add pointer type
        using reference         = value_type &; // Add reference type
        using iterator_category = std::forward_iterator_tag;

        Iterator(const ArgumentQueue &queue, std::size_t cursor);

        [[nodiscard]] value_type operator*() const;

        Iterator &operator++();
        Iterator  operator++(int);

        [[nodiscard]] bool operator==(const Iterator &other) const;
        [[nodiscard]] bool operator!=(const Iterator &other) const;

    private:
        const CLArgs::ArgumentQueue &queue_;
        std::size_t                  cursor_;
    };
} // namespace CLArgs

inline CLArgs::ArgumentQueue::ArgumentQueue(const int argc, const char *const *const argv)
{
    if (argc < 0)
    {
        throw std::invalid_argument("Cannot pass a negative value as argc to ArgumentQueue destructor");
    }

    if (argv == nullptr)
    {
        throw std::invalid_argument("Cannot pass nullptr as argv to ArgumentQueue constructor");
    }

    arguments_ = std::span(argv, static_cast<std::size_t>(argc));

    if (std::any_of(arguments_.begin(), arguments_.end(), [](const char *const arg) { return arg == nullptr; }))
    {
        throw std::invalid_argument("None of the strings passed in argv array to ArgumentQueue constructor can be nullptr");
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
CLArgs::ArgumentQueue::front() const
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

template <std::size_t N>
std::array<std::string_view, N>
CLArgs::ArgumentQueue::dequeue()
{
    if (size() < N)
    {
        throw std::logic_error("Attempted to dequeue more elements in ArgumentQueue than there are in the queue, consider checking size "
                               "first using size()");
    }

    std::array<std::string_view, N> arr{};
    std::ranges::generate(arr, [this] { return dequeue(); });
    return arr;
}

CLArgs::ArgumentQueue::Iterator
CLArgs::ArgumentQueue::begin() const
{
    return {*this, cursor_};
}

CLArgs::ArgumentQueue::Iterator
CLArgs::ArgumentQueue::end() const
{
    return {*this, arguments_.size()};
}

CLArgs::ArgumentQueue::Iterator::Iterator(const ArgumentQueue &queue, const std::size_t cursor) : queue_(queue), cursor_{cursor}
{
}

CLArgs::ArgumentQueue::Iterator::value_type
CLArgs::ArgumentQueue::Iterator::operator*() const
{
    return {queue_.arguments_[cursor_]};
}

CLArgs::ArgumentQueue::Iterator &
CLArgs::ArgumentQueue::Iterator::operator++()
{
    ++cursor_;
    return *this;
}

CLArgs::ArgumentQueue::Iterator
CLArgs::ArgumentQueue::Iterator::operator++(int)
{
    Iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool
CLArgs::ArgumentQueue::Iterator::operator==(const Iterator &other) const
{
    return (&queue_ == &other.queue_) && (cursor_ == other.cursor_);
}

bool
CLArgs::ArgumentQueue::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

#endif
