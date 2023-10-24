#pragma once

#include "GameWorld.h"
#include "InputProcessor.h"

class Player {
public:
    Player(InputProcessor& inputProcessor, GameWorld& gameWorld);

private:
    struct SelectedCell {
        SelectedCell(Vec2 initialCoordinates, Vec2 selectedIndex, GameWorld& gameWorld, bool isDragging);
        ~SelectedCell();

        void UpdateBoardState(Vec2 currentPosition);
        bool IsDragging() const;
        Vec2 Index() const;

    private:
        Vec2 _initialCoordinates;
        Vec2 _selectedIndex;
        GameWorld& _gameWorld;
        bool _isDragging;
        Vec2 _currentPosition = Vec2 { 0, 0 };
    };

    GameWorld* _gameWorld = nullptr;
    std::unique_ptr<EventToken> _mouseMoveToken;
    std::unique_ptr<EventToken> _mouseDragStartedToken;
    std::unique_ptr<EventToken> _mouseDragMovedToken;
    std::unique_ptr<EventToken> _mouseDragEndedToken;
    std::unique_ptr<EventToken> _tileDragCompletedToken;
    std::optional<Vec2> _draggedCell; // Used for mouse drag
    std::optional<SelectedCell> _selectedCell; // Used for mouse selection

    void OnMouseDragStarted(Vec2 position);
    void OnMouseDragMoved(Vec2 position);
    void OnMouseDragEnded(Vec2 position);
    void OnMouseClicked(Vec2 position);
    void OnTileDragCompleted(Vec2 source);
};
