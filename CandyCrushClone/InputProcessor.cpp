#include "InputProcessor.h"

namespace {
constexpr float mouseDragThresholdSquared = 4;
}

void InputProcessor::ProcessKeyEvent(const SDL_Event& keyEvent)
{
    assert(keyEvent.type == SDL_KEYDOWN);

    switch (keyEvent.key.keysym.sym) {
    case SDLK_ESCAPE: {
        KeyPressed.Invoke(Key::Escape);
    } break;
    }
}

void InputProcessor::ProcessMouseEvent(const SDL_Event& mouseEvent)
{
    switch (mouseEvent.type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (mouseEvent.button.button == SDL_BUTTON_LEFT) {
            _mouseDragStartPosition = Vec2 { mouseEvent.button.x, mouseEvent.button.y };
        }
    } break;
    case SDL_MOUSEMOTION: {
        if (mouseEvent.button.button == SDL_BUTTON_LEFT) {
            if (_mouseDragStartPosition) {
                if (_isDragging) {
                    MouseDragMoved.Invoke(Vec2 { mouseEvent.button.x, mouseEvent.button.y });
                } else if (_mouseDragStartPosition->DistanceSquared(Vec2 { mouseEvent.button.x, mouseEvent.button.y }) > mouseDragThresholdSquared) {
                    // If we are not yet dragging and the drag can be started, then let's start it
                    _isDragging = true;
                    MouseDragStarted.Invoke(*_mouseDragStartPosition);
                }
            }
        }

        MouseMoved.Invoke(Vec2 { mouseEvent.button.x, mouseEvent.button.y });
    } break;
    case SDL_MOUSEBUTTONUP: {
        if (mouseEvent.button.button == SDL_BUTTON_LEFT) {
            if (_mouseDragStartPosition) {
                if (_isDragging) {
                    MouseDragEnded.Invoke(Vec2 { mouseEvent.button.x, mouseEvent.button.y });
                    _isDragging = false;
                } else {
                    // Instead of the start position (mouse down even location), we invoke the event with the current position. Should be less confusing
                    MouseClicked.Invoke(Vec2 { mouseEvent.button.x, mouseEvent.button.y });
                }

                _mouseDragStartPosition.reset();
            }
        }
    } break;
    default:
        break;
    }
}
