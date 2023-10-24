#pragma once

#include <string>
#include <vector>

#include "GameMode.h"

struct CellDestructionData;

class IGameState {
public:
    virtual void UpdateScore(const CellDestructionData& datas) = 0;
    virtual std::vector<std::string> GetUIText() = 0;
    virtual std::vector<std::string> GetResult() = 0;

    virtual void Update(int deltaTime);
    virtual bool IsGameOver() const = 0;
    virtual GameMode GetGameMode() const = 0;

    virtual int GetScore() const = 0;

    virtual ~IGameState() = default;

protected:
    uint64_t _timePassedMs = 0;
};

class ClassicGameState : public IGameState {
public:
    void UpdateScore(const CellDestructionData& datas) override;
    std::vector<std::string> GetUIText() override;
    std::vector<std::string> GetResult() override;
    virtual int GetScore() const override;
    virtual GameMode GetGameMode() const override;
    virtual bool IsGameOver() const override;

private:
    static constexpr int ScoreToReach = 3000;
    int _score;
};

class QuickDeathGameState : public IGameState {
public:
    bool IsGameOver() const override;
    void Update(int deltaTime) override;

    void UpdateScore(const CellDestructionData& datas) override;
    std::vector<std::string> GetUIText() override;
    std::vector<std::string> GetResult() override;
    virtual int GetScore() const override;
    virtual GameMode GetGameMode() const override;

private:
    static constexpr int InitialTimeLeft = 15000; // Start with 10 seconds

    int _timeLeft = InitialTimeLeft;
};
