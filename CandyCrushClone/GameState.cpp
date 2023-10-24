#include "GameState.h"

#include "GameWorld.h"

namespace {
std::string ToStringWith2FractionalDigits(uint64_t timeMs)
{
    auto fractionalSeconds = timeMs % 1000;
    fractionalSeconds = uint64_t(fractionalSeconds / 10.0);

    auto fractionalPartString = std::to_string(fractionalSeconds);

    // Always have 2 fractional digits
    if (fractionalPartString.size() < 2) {
        fractionalPartString += "0";
    }

    return std::to_string(int(timeMs / 1000.0)) + "." + fractionalPartString;
}
}

void ClassicGameState::UpdateScore(const CellDestructionData& data)
{
    // The player gets 20 points for each cell
    // 5 extra points are given for each cell after each destroyed tile in the longest streak
    // So if there is a row of 5 and a total of 8 cells are destroyed, that's 8 x 30 = 240 points

    auto highestCombo = std::max(data.HighestColumnCombo, data.HighestRowCombo);
    auto pointsForEachCell = 20 + (highestCombo - 3) * 5;

    _score += pointsForEachCell * int(data.DestroyedCells.size());
}

std::vector<std::string> ClassicGameState::GetUIText()
{
    return {
        "Current score:",
        std::to_string(_score),
        "Goal: ",
        std::to_string(ScoreToReach),
        "Time passed: ",
        ToStringWith2FractionalDigits(_timePassedMs),
    };
}

std::vector<std::string> ClassicGameState::GetResult()
{
    return { std::string("You reached: ") + std::to_string(ScoreToReach) + " points in " + ToStringWith2FractionalDigits(_timePassedMs) + " seconds!" };
}

int ClassicGameState::GetScore() const
{
    return int(_timePassedMs);
}

GameMode ClassicGameState::GetGameMode() const
{
    return GameMode::Classic;
}

bool ClassicGameState::IsGameOver() const
{
    return _score >= ScoreToReach;
}

bool QuickDeathGameState::IsGameOver() const
{
    return _timeLeft <= 0;
}

void QuickDeathGameState::Update(int deltaTime)
{
    IGameState::Update(deltaTime);

    _timeLeft -= deltaTime;
}

void QuickDeathGameState::UpdateScore(const CellDestructionData& data)
{
    auto highestCombo = std::max(data.HighestColumnCombo, data.HighestRowCombo);
    auto timeForEachCellMs = 300 + (highestCombo - 3) * 300; // extra 200 ms for each cell above 3 in the highest streak

    _timeLeft += int(timeForEachCellMs * data.DestroyedCells.size());
}

std::vector<std::string> QuickDeathGameState::GetUIText()
{
    return {
        "Survive as long as you can!",
        "Time left",
        ToStringWith2FractionalDigits(_timeLeft),
    };
}

std::vector<std::string> QuickDeathGameState::GetResult()
{
    return { std::string("You have survived for ") + ToStringWith2FractionalDigits(_timePassedMs) + " seconds!" };
}

int QuickDeathGameState::GetScore() const
{
    return int(_timePassedMs);
}

GameMode QuickDeathGameState::GetGameMode() const
{
    return GameMode::QuickDeath;
}

void IGameState::Update(int deltaTime)
{
    _timePassedMs += deltaTime;
}
