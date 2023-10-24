#include "GameWorld.h"

namespace {
bool Contains(const std::array<int, 2>& arr, int value)
{
    return std::find(arr.begin(), arr.end(), value) != arr.end();
}
}

Cell::Cell(int type)
    : Type(type)
{
}

void Cell::Destroy()
{
    State = CellState::Destroyed;
}

int GameWorld::GetRandomNumber(const std::array<int, 2>& excluding)
{
    int randomNumber = _randomDistribution(_randomEngine);
    while (Contains(excluding, randomNumber)) {
        randomNumber = (randomNumber + 1) % TileKindCount;
    }

    return randomNumber;
}

Cell GameWorld::GenerateCellForIndex(int i, int j)
{
    std::array<int, 2> excludedNumbers = { -1, -1 };

    // If the current row's or column's previous 2 cells have the same type, then generate another kind
    if (i > 1 && _gameBoard[i - 1][j].Type == _gameBoard[i - 2][j].Type) {
        excludedNumbers[0] = _gameBoard[i - 1][j].Type;
    }
    if (j > 1 && _gameBoard[i][j - 1].Type == _gameBoard[i][j - 2].Type) {
        excludedNumbers[1] = _gameBoard[i][j - 1].Type;
    }

    return Cell(GetRandomNumber(excludedNumbers));
}

void GameWorld::FillBoard()
{
    _gameBoard.reserve(ColCount);
    _gameBoard.clear();

    for (int i = 0; i < RowCount; ++i) {
        auto& column = _gameBoard.emplace_back();
        column.reserve(RowCount);
        for (int j = 0; j < RowCount; ++j) {
            column.push_back(GenerateCellForIndex(i, j));
        }
    }
}

GameWorld::GameWorld(int rowCount, int colCount, int tileKindCount, Screen& screen, AudioPlayer& audioPlayer)
    : RowCount(rowCount)
    , ColCount(colCount)
    , TileKindCount(tileKindCount)
    , _screen(&screen)
    , _randomEngine(_randomDevice())
    , _randomDistribution(0, tileKindCount - 1) // Random distribution is inclusive on both ends, so the range [0, n - 1] will contain n possible values
    , _audioPlayer(&audioPlayer)
{
    FillBoard();
}

void GameWorld::Activate(IGameState& gameState)
{
    _isActive = true;

    if (_gameState != &gameState) {
        _gameState = &gameState;
        _animationState.reset();
        _activeCellState.reset();
        FillBoard();
    }
}

void GameWorld::Deactivate()
{
    _isActive = false;
}

void GameWorld::Draw()
{
    for (int i = 0; i < _gameBoard.size(); ++i) {
        auto& currentRow = _gameBoard[i];
        for (int j = 0; j < currentRow.size(); ++j) {
            if (_gameBoard[i][j].State == Cell::CellState::Normal) {
                _screen->DrawCell(Vec2 { i * TileSize, j * TileSize }, _gameBoard[i][j].Type, TileSize, TileSize);
            }
        }
    }

    if (_activeCellState) {
        // Make a periodic function with a period of 1 second and in the range [0, 0.2]
        auto scaleDiff = (sin(_activeCellState->AnimationTimePassed / 1000.f * 2 * M_PI)) * 0.1;
        auto newSize = TileSize * (1 + scaleDiff);
        auto halfDiff = int((newSize - TileSize) / 2.0);

        _screen->DrawCell(
            _activeCellState->Index * TileSize - Vec2 { halfDiff, halfDiff } + _activeCellState->Offset,
            At(_activeCellState->Index).Type,
            TileSize,
            int(newSize));
    }

    if (_animationState) {
        if (std::holds_alternative<std::vector<CellAnimationMoveData>>(_animationState->AnimationData)) {
            auto& animationData = std::get<std::vector<CellAnimationMoveData>>(_animationState->AnimationData);
            for (const auto& [startPosition, endPosition, cellType, startPositionOverride] : animationData) {
                auto realStartPosition = startPositionOverride.value_or(startPosition);
                _screen->DrawCell(realStartPosition.Lerp(endPosition, _animationState->AnimationProgress), cellType, TileSize, TileSize);
            }
        } else if (std::holds_alternative<std::vector<CellAnimationDestructionData>>(_animationState->AnimationData)) {
            auto& animationData = std::get<std::vector<CellAnimationDestructionData>>(_animationState->AnimationData);
            for (const auto& [cellIndex, cellType] : animationData) {
                double newSize = (1 - _animationState->AnimationProgress) * TileSize;
                auto halfDiff = int((TileSize - newSize) / 2);
                auto offset = Vec2 { halfDiff, halfDiff };

                _screen->DrawCell(cellIndex * TileSize + Vec2 { halfDiff, halfDiff }, cellType, TileSize, int(newSize));
                _screen->DrawDestroyAnimation(cellIndex * TileSize, TileSize, _animationState->AnimationProgress);
            }
        }
    }

    static constexpr int spacing = 50;
    static constexpr int textWidth = 240;
    static constexpr int textHeight = 40;

    auto textLines = _gameState->GetUIText();
    auto textPosition = 560 + (_screen->ScreenWidth - 560 - textWidth) / 2;
    SDL_Rect textRect { textPosition, 50, textWidth, textHeight };
    SDL_Rect uIBackgroundRect { textRect.x - spacing, textRect.y - spacing, textRect.w + 2 * spacing, int(textLines.size() + 2) * spacing };

    _screen->DrawBackgroundRectangle(uIBackgroundRect);

    for (const auto& line : textLines) {
        _screen->DrawText(line, textRect, true);
        textRect.y += spacing;
    }
}

void GameWorld::Update(uint64_t deltaTimeMs)
{
    _gameState->Update(int(deltaTimeMs));

    if (_animationState) {
        _animationState->AnimationTimePassed += deltaTimeMs;

        double rawProgress = _animationState->AnimationTimePassed / _animationState->AnimationDuration;

        if (_animationState->EffectToPlay) {
            _audioPlayer->PlaySoundEffect(*_animationState->EffectToPlay);
            _animationState->EffectToPlay.reset();
        } else if (rawProgress > 1.0) {
            auto completion = std::move(_animationState->Completion);

            for (auto& column : _gameBoard) {
                for (auto& cell : column) {
                    if (cell.State == Cell::CellState::WaitingForAnimationToComplete) {
                        cell.State = _animationState->FinalCellState;
                    }
                }
            }

            _animationState.reset();

            if (completion) {
                completion();
            }
        } else {
            switch (_animationState->EasingFun) {
            case EasingFunction::EaseInCubic: {
                _animationState->AnimationProgress = pow(rawProgress, 3);
            } break;
            case EasingFunction::EaseOutBounce: {
                // Taken from https://easings.net/#easeOutBounce
                static const double n1 = 7.5625;
                static const double d1 = 2.75;
                double x = rawProgress;
                double animationProgress;

                if (x < 1 / d1) {
                    animationProgress = n1 * x * x;
                } else if (x < 2 / d1) {
                    x -= 1.5 / d1;
                    animationProgress = n1 * x * x + 0.75;
                } else if (x < 2.5 / d1) {
                    x -= 2.25 / d1;
                    animationProgress = n1 * x * x + 0.9375;
                } else {
                    x -= 2.625 / d1;
                    animationProgress = n1 * x * x + 0.984375;
                }
                _animationState->AnimationProgress = animationProgress;
            } break;
            }
        }
    }

    if (_activeCellState) {
        _activeCellState->AnimationTimePassed += deltaTimeMs;
    }
}

bool GameWorld::IsInteractionEnabled() const
{
    return _isActive && !_animationState.has_value();
}

void GameWorld::SetActiveCell(std::optional<Vec2> index, Vec2 offset)
{
    if (index) {
        if (abs(offset.x) > DragOffsetSuccessThreshold) { // Successful drag in the x direction
            if (auto newCell = *index + Vec2 { offset.x > 0 ? 1 : -1, 0 }; IsIndexOnTheBoard(newCell)) {
                if (TrySwitchCells(*index, newCell, true)) {
                    TileDragCompleted.Invoke(*index);
                }
            }
        } else if (abs(offset.y) > DragOffsetSuccessThreshold) { // Successful drag in the y direction
            if (auto newCell = *index + Vec2 { 0, offset.y > 0 ? 1 : -1 }; IsIndexOnTheBoard(newCell)) {
                if (TrySwitchCells(*index, newCell, true)) {
                    TileDragCompleted.Invoke(*index);
                }
            }
        } else { // Just update the drag state (eg. cell position)
            if (_activeCellState && _activeCellState->Index == index) {
                _activeCellState->Offset = offset;
            } else {
                _activeCellState.emplace(
                    *index, offset, 0);
                At(*index).State = Cell::CellState::Active;
            }
        }
    } else {
        if (_activeCellState) {
            auto activeIndex = _activeCellState->Index;
            if (At(activeIndex).State == Cell::CellState::Active) {
                At(activeIndex).State = Cell::CellState::Normal;

                if (offset != Vec2 { 0, 0 }) {
                    MoveCellsAnimated({ CellAnimationMoveData { Vec2 {},
                                          activeIndex,
                                          At(activeIndex).Type,
                                          activeIndex * TileSize + _activeCellState->Offset } },
                        CellSwitchAnimationDurationMs, nullptr);
                }
            }
            _activeCellState.reset();
        }
    }
}

bool GameWorld::TrySwitchCells(Vec2 lhs, Vec2 rhs, bool isDraggedCellTheSource)
{
    assert(lhs.x >= 0 && lhs.x < RowCount);
    assert(lhs.y >= 0 && lhs.y < ColCount);
    assert(rhs.x >= 0 && rhs.x < RowCount);
    assert(rhs.y >= 0 && rhs.y < ColCount);
    assert(!isDraggedCellTheSource || _activeCellState);

    if (lhs.DistanceSquared(rhs) == 1) {
        auto tmp = At(lhs);
        At(lhs) = At(rhs);
        At(rhs) = tmp;

        // Check if we can destroy something in the new state
        auto cellsToDestroy = GetCellsToDestroyFromCurrentState();

        // Restore the original state
        tmp = At(lhs);
        At(lhs) = At(rhs);
        At(rhs) = tmp;

        if (!cellsToDestroy.DestroyedCells.empty()) {
            MoveCellsAnimated(
                {
                    CellAnimationMoveData { lhs, rhs, At(lhs).Type, isDraggedCellTheSource ? _activeCellState->Index * TileSize + _activeCellState->Offset : std::optional<Vec2>() },
                    CellAnimationMoveData { rhs, lhs, At(rhs).Type, std::nullopt },
                },
                CellSwitchAnimationDurationMs, [this]() { UpdateBoardState(); });

            return true;
        } else if (_activeCellState) { // Just move back the moved cell to its original position
            // This will only be invoked if we are dragging a cell, otherwise activeCellState is already reset
            auto activeIndex = _activeCellState->Index;
            MoveCellsAnimated({ CellAnimationMoveData { Vec2 {},
                                  activeIndex,
                                  At(activeIndex).Type,
                                  activeIndex * TileSize + _activeCellState->Offset } },
                CellSwitchAnimationDurationMs, nullptr);
            TileDragCompleted.Invoke(activeIndex);
        }
    }

    return false;
}

std::optional<Vec2> GameWorld::GetTileIndicesAtPoint(Vec2 position)
{
    Vec2 possibleResult = position / TileSize;
    if (possibleResult.x >= 0 && possibleResult.x < RowCount && possibleResult.y >= 0 && possibleResult.y < ColCount) {
        return possibleResult;
    }

    return std::nullopt;
}

Cell& GameWorld::At(Vec2 indices)
{
    assert(indices.x >= 0 && indices.x < ColCount);
    assert(indices.y >= 0 && indices.y < RowCount);

    return _gameBoard[indices.x][indices.y];
}

const Cell& GameWorld::At(Vec2 indices) const
{
    assert(indices.x >= 0 && indices.x < ColCount);
    assert(indices.y >= 0 && indices.y < RowCount);

    return _gameBoard[indices.x][indices.y];
}

CellDestructionData GameWorld::GetCellsToDestroyFromCurrentState() const
{
    std::vector<Vec2> cellsToRemove;

    int maxColStreak = 0;
    // Check the columns for at least 3 of the same cells next to each other
    for (int i = 0; i < ColCount; ++i) {
        int j = 0;
        while (j < RowCount - 2) {
            int k = j + 1;

            // Keep going until we find a cell that is different from the current one
            while (k < RowCount && _gameBoard[i][j].Type == _gameBoard[i][k].Type) {
                ++k;
            }

            // Check if we have at least 3 of the same cell types next to each other
            if (k - j > 2) {
                maxColStreak = std::max(maxColStreak, k - j);

                for (int copyInd = j; copyInd < k; ++copyInd) {
                    cellsToRemove.push_back(Vec2 { i, copyInd });
                }
            }

            j = k;
        }
    }

    int maxRowStreak = 0;
    // Exact same logic for the rows as well
    for (int j = 0; j < RowCount; ++j) {
        int i = 0;
        while (i < ColCount - 2) {
            int k = i + 1;

            while (k < ColCount && _gameBoard[i][j].Type == _gameBoard[k][j].Type) {
                ++k;
            }

            if (k - i > 2) {
                maxRowStreak = std::max(maxRowStreak, k - i);

                for (int copyInd = i; copyInd < k; ++copyInd) {
                    cellsToRemove.push_back(Vec2 { copyInd, j });
                }
            }

            i = k;
        }
    }

    if (!cellsToRemove.empty()) {
        std::sort(cellsToRemove.begin(), cellsToRemove.end(), std::greater<Vec2>());
        cellsToRemove.erase(std::unique(cellsToRemove.begin(), cellsToRemove.end()), cellsToRemove.end());
    }

    return CellDestructionData(std::move(cellsToRemove), maxRowStreak, maxColStreak);
}

void GameWorld::UpdateBoardState(CellDestructionData&& cellDestructionData)
{
    const auto& cellsToRemove = cellDestructionData.DestroyedCells;
    if (!cellsToRemove.empty()) {
        for (auto& cell : cellsToRemove) {
            At(cell).Destroy();
        }

        _gameState->UpdateScore(cellDestructionData);

        DestroyCellsAnimated(std::move(cellDestructionData.DestroyedCells), CellDestroyAnimationDurationMs, [this]() { MoveDownCells(); });
    }
}

void GameWorld::UpdateBoardState()
{
    auto cellsToRemove = GetCellsToDestroyFromCurrentState();

    UpdateBoardState(std::move(cellsToRemove));
}

void GameWorld::MoveCellsAnimated(
    std::vector<CellAnimationMoveData>&& moveData,
    double animationDuration,
    std::function<void()> completion,
    EasingFunction easingFun)
{
    assert(!_animationState);
    _animationState.emplace();

    for (auto& animationData : moveData) {
        auto& finalCell = At(animationData.FinalPosition);
        finalCell.State = Cell::CellState::WaitingForAnimationToComplete;
        finalCell.Type = animationData.CellType;

        animationData.FinalPosition = animationData.FinalPosition * TileSize;
        animationData.StartingPosition = animationData.StartPositionOverride.value_or(animationData.StartingPosition) * TileSize;
    }

    _animationState->AnimationData = std::move(moveData);

    _animationState->AnimationDuration = animationDuration;
    _animationState->Completion = std::move(completion);
    _animationState->EasingFun = easingFun;
}

void GameWorld::DestroyCellsAnimated(std::vector<Vec2>&& cellsToDestroy, double animationTime, std::function<void()> completion)
{
    _animationState.emplace();

    std::vector<CellAnimationDestructionData> animationData;
    animationData.reserve(cellsToDestroy.size());

    for (Vec2 cell : cellsToDestroy) {
        animationData.push_back(CellAnimationDestructionData { cell, At(cell).Type });
    }

    _animationState->AnimationData = std::move(animationData);

    _animationState->AnimationDuration = CellDestroyAnimationDurationMs;
    _animationState->Completion = std::move(completion);
    _animationState->FinalCellState = Cell::CellState::Destroyed;
    _animationState->EffectToPlay = AudioPlayer::SoundEffect::TileDisappear;
}

void GameWorld::MoveDownCells()
{
    std::vector<CellAnimationMoveData> cellMoveData;

    // Update the position of every cell that is above a destroyed cell and add them to be animated
    for (int i = 0; i < ColCount; ++i) {
        int destroyedCellCount = 0;

        for (int j = RowCount - 1; j >= 0; --j) {
            if (_gameBoard[i][j].State == Cell::CellState::Destroyed) {
                ++destroyedCellCount;
            } else if (destroyedCellCount > 0) {
                int newRow = j + destroyedCellCount;

                auto startPosition = Vec2 { i, j };
                auto finalPosition = Vec2 { i, newRow };

                cellMoveData.push_back(CellAnimationMoveData { startPosition, finalPosition, At(startPosition).Type });

                At(startPosition).State = Cell::CellState::Destroyed;
            }
        }

        // Fill the board again by generating destroyedCellCount number of new cells and animate them in from the top
        for (int cellInd = 0; cellInd < destroyedCellCount; ++cellInd) {
            auto finalPosition = Vec2 { i, cellInd };
            auto startPosition = Vec2 { i, cellInd - destroyedCellCount };
            auto newCellType = GetRandomNumber();

            cellMoveData.push_back(CellAnimationMoveData { startPosition, finalPosition, newCellType });
        }
    }

    MoveCellsAnimated(
        std::move(cellMoveData),
        BaseCellFallAnimationDurationMs,
        [this]() { UpdateBoardState(); },
        EasingFunction::EaseOutBounce);
}

bool GameWorld::IsIndexOnTheBoard(Vec2 index) const
{
    return !(index.x < 0 || index.x > ColCount - 1 || index.y < 0 || index.y > RowCount - 1);
}

CellDestructionData::CellDestructionData(std::vector<Vec2>&& destroyedCells, int highestRowCombo, int highestColCombo)
    : DestroyedCells(std::move(destroyedCells))
    , HighestRowCombo(highestRowCombo)
    , HighestColumnCombo(highestColCombo)
{
}
