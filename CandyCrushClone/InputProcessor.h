#pragma once

#include "Event.h"
#include "Vec2.h"

#include <SDL.h>

#include <optional>

enum class Key {
    Escape,
};

class InputProcessor {
public:
    void ProcessKeyEvent(const SDL_Event& keyEvent);
    void ProcessMouseEvent(const SDL_Event& mouseEvent);

    Event<std::function<void(Vec2 position)>> MouseDragStarted;
    Event<std::function<void(Vec2 position)>> MouseDragMoved;
    Event<std::function<void(Vec2 position)>> MouseDragEnded;
    Event<std::function<void(Vec2 position)>> MouseClicked;
    Event<std::function<void(Vec2 position)>> MouseMoved;

    Event<std::function<void(Key key)>> KeyPressed;

private:
    bool _isDragging = false;
    std::optional<Vec2> _mouseDragStartPosition;
};
