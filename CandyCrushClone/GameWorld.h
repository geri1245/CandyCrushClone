#pragma once

#include "AudioPlayer.h"
#include "Event.h"
#include "GameState.h"
#include "Screen.h"
#include "Vec2.h"

#include <array>
#include <optional>
#include <random>
#include <variant>
#include <vector>

struct Cell {
    enum class CellState {
        Destroyed,
        Normal,
        Active,
        WaitingForAnimationToComplete,
    };

    Cell(int cellTypeId);

    void Destroy();

    int Type = -1;
    CellState State = CellState::Normal;
};

struct CellDestructionData {
    CellDestructionData(std::vector<Vec2>&& destroyedCells, int highestRowCombo, int highestColCombo);

    std::vector<Vec2> DestroyedCells;
    int HighestRowCombo;
    int HighestColumnCombo;
};

class GameWorld {
public:
    using GameBoard = std::vector<std::vector<Cell>>;

    const int RowCount, ColCount, TileKindCount;

    Event<std::function<void(Vec2 source)>> TileDragCompleted;

    GameWorld(int rowCount, int colCount, int tileKindCount, Screen& screen, AudioPlayer& audioPlayer);

    void Activate(IGameState& gameState);
    void Deactivate();

    void Draw();
    void Update(uint64_t deltaTimeMs);
    bool IsInteractionEnabled() const;

    void SetActiveCell(std::optional<Vec2> index, Vec2 offset = Vec2 { 0, 0 });

    bool TrySwitchCells(Vec2 source, Vec2 destination, bool isDraggedCellTheSource = false);

    std::optional<Vec2> GetTileIndicesAtPoint(Vec2 position);

private:
    enum class EasingFunction {
        EaseOutBounce,
        EaseInCubic,
    };

    struct CellAnimationMoveData {
        Vec2 StartingPosition;
        Vec2 FinalPosition;
        int CellType;
        std::optional<Vec2> StartPositionOverride;
    };

    struct CellAnimationDestructionData {
        Vec2 CellIndex;
        int CellType;
    };

    struct AnimationState {
        std::variant<std::vector<CellAnimationMoveData>, std::vector<CellAnimationDestructionData>> AnimationData;
        std::function<void()> Completion;
        uint64_t AnimationTimePassed = 0;
        double AnimationDuration = 0;
        double AnimationProgress = 0.0;
        Cell::CellState FinalCellState = Cell::CellState::Normal;
        EasingFunction EasingFun = EasingFunction::EaseInCubic;
        std::optional<AudioPlayer::SoundEffect> EffectToPlay;
    };

    struct ActiveCellState {
        Vec2 Index;
        Vec2 Offset;
        uint64_t AnimationTimePassed = 0;
    };

    static constexpr int TileSize = 70; // The provided assets have this size, so for now just use it
    static constexpr int DragOffsetSuccessThreshold = int(TileSize * 0.8);
    static constexpr double CellSwitchAnimationDurationMs = 200.0;
    static constexpr double CellDestroyAnimationDurationMs = 400.0;
    static constexpr double BaseCellFallAnimationDurationMs = 800.0;

    Cell& At(Vec2 indices);
    const Cell& At(Vec2 indices) const;

    int GetRandomNumber(const std::array<int, 2>& excluding = { -1, -1 });
    Cell GenerateCellForIndex(int i, int j);
    void FillBoard();

    CellDestructionData GetCellsToDestroyFromCurrentState() const;
    void UpdateBoardState();
    void UpdateBoardState(CellDestructionData&& cellsToRemove);
    void MoveCellsAnimated(
        std::vector<CellAnimationMoveData>&& moveData,
        double animationDuration,
        std::function<void()> completion,
        EasingFunction easingFun = EasingFunction::EaseInCubic);
    void DestroyCellsAnimated(std::vector<Vec2>&& cellsToDestroy, double animationTime, std::function<void()> completion);
    void MoveDownCells();

    bool IsIndexOnTheBoard(Vec2 index) const;

    // The board is stored in a column major order. Columns are growing from left to right. Rows are growing from top to bottom.
    GameBoard _gameBoard;
    Screen* _screen = nullptr;
    bool _isActive = false;

    std::random_device _randomDevice;
    std::mt19937 _randomEngine;
    std::uniform_int_distribution<int> _randomDistribution;

    std::optional<AnimationState> _animationState;
    std::optional<ActiveCellState> _activeCellState;
    IGameState* _gameState;
    AudioPlayer* _audioPlayer;
};
