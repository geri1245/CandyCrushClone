#include "Player.h"

Player::Player(InputProcessor& inputProcessor, GameWorld& gameWorld)
    : _gameWorld(&gameWorld)
    , _mouseMoveToken(inputProcessor.MouseClicked.Subscribe([this](Vec2 position) {
        if (_gameWorld->IsInteractionEnabled()) {
            OnMouseClicked(position);
        }
    }))
    , _mouseDragStartedToken(inputProcessor.MouseDragStarted.Subscribe([this](Vec2 position) {
        if (_gameWorld->IsInteractionEnabled()) {
            OnMouseDragStarted(position);
        }
    }))
    , _mouseDragMovedToken(inputProcessor.MouseDragMoved.Subscribe([this](Vec2 position) {
        if (_gameWorld->IsInteractionEnabled()) {
            OnMouseDragMoved(position);
        }
    }))
    , _mouseDragEndedToken(inputProcessor.MouseDragEnded.Subscribe([this](Vec2 position) {
        if (_gameWorld->IsInteractionEnabled()) {
            OnMouseDragEnded(position);
        }
    }))
    , _tileDragCompletedToken(gameWorld.TileDragCompleted.Subscribe([this](Vec2 source) {
        OnTileDragCompleted(source);
    }))
{
}

void Player::OnMouseClicked(Vec2 clickedCoordinates)
{
    _draggedCell.reset();

    if (_selectedCell) {
        auto newSelectedCell = _gameWorld->GetTileIndicesAtPoint(clickedCoordinates);
        auto cell = _selectedCell->Index();

        _selectedCell.reset();

        if (newSelectedCell) {
            _gameWorld->TrySwitchCells(cell, *newSelectedCell);
        }

    } else if (auto newSelectedCell = _gameWorld->GetTileIndicesAtPoint(clickedCoordinates)) {
        _selectedCell.emplace(clickedCoordinates, *newSelectedCell, *_gameWorld, false);
    }
}

void Player::OnTileDragCompleted(Vec2 source)
{
    assert(_selectedCell->Index() == source);

    _selectedCell.reset();
}

void Player::OnMouseDragStarted(Vec2 clickedCoordinates)
{
    auto draggedCell = _gameWorld->GetTileIndicesAtPoint(clickedCoordinates);
    if (draggedCell) {
        _selectedCell.emplace(clickedCoordinates, *draggedCell, *_gameWorld, true);
    }
}

void Player::OnMouseDragMoved(Vec2 position)
{
    if (_selectedCell && _selectedCell->IsDragging()) {
        _selectedCell->UpdateBoardState(position);
    }
}

void Player::OnMouseDragEnded(Vec2 position)
{
    if (_selectedCell && _selectedCell->IsDragging()) {
        _selectedCell.reset();
    }
}

Player::SelectedCell::SelectedCell(Vec2 initialCoordinates, Vec2 selectedIndex, GameWorld& gameWorld, bool isDragging)
    : _initialCoordinates(initialCoordinates)
    , _selectedIndex(selectedIndex)
    , _gameWorld(gameWorld)
    , _isDragging(isDragging)
{
    _gameWorld.SetActiveCell(_selectedIndex);
}

Player::SelectedCell::~SelectedCell()
{
    _gameWorld.SetActiveCell(std::nullopt, _currentPosition);
}

void Player::SelectedCell::UpdateBoardState(Vec2 currentPosition)
{
    _gameWorld.SetActiveCell(_selectedIndex, currentPosition - _initialCoordinates);
    _currentPosition = currentPosition;
}

bool Player::SelectedCell::IsDragging() const
{
    return _isDragging;
}

Vec2 Player::SelectedCell::Index() const
{
    return _selectedIndex;
}
