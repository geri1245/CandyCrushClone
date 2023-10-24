#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <vector>

class EventToken {
public:
    explicit EventToken(std::function<void()> unsubAction)
        : _unsubAction(std::move(unsubAction))
    {
        assert(_unsubAction);
    }
    ~EventToken() { _unsubAction(); }

private:
    std::function<void()> _unsubAction;
};

template <class T>
class Event {
public:
    [[nodiscard]] std::unique_ptr<EventToken> Subscribe(T&& action)
    {
        auto initialSize = _subscribers.size();

        for (int i = 0; i < initialSize; ++i) {
            if (!_subscribers[i]) {
                _subscribers[i] = std::move(action);

                return std::make_unique<EventToken>([this, i]() { _subscribers[i] = nullptr; });
            }
        }

        _subscribers.push_back(std::move(action));
        return std::make_unique<EventToken>([this, initialSize]() { _subscribers[initialSize] = nullptr; });
    }

    template <class... Params>
    void Invoke(Params&&... params)
    {
        for (const auto& subscriber : _subscribers) {
            if (subscriber)
                subscriber(std::forward<Params>(params)...);
        }
    }

private:
    std::vector<T> _subscribers;
};