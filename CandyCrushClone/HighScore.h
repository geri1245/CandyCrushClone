#pragma once

#include "GameMode.h"

#include <string>
#include <vector>

class HighScore {
public:
    HighScore();

    bool ReadHighScore();
    bool WriteHighScore();
    void AddScore(GameMode mode, int score);

    const std::vector<int>& GetClassicScores() const;
    const std::vector<int>& GetQuickDeathScores() const;

private:
    static constexpr int ScoresRemembered = 5;

    // Scores are stored in ascending order
    std::vector<int> _classicScores;
    std::vector<int> _quickDeathScores;
};
