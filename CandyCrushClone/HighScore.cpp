#include "HighScore.h"

#include <exception>
#include <filesystem>
#include <fstream>

namespace {
static const char* const HighScoreFilePath = "./high.score";

std::vector<std::string> Split(const std::string& string, const std::string& delimiter)
{
    if (string.empty()) {
        return {};
    }

    size_t pos = 0;
    size_t nextPos = string.find(delimiter);
    std::vector<std::string> parts;

    while (nextPos != std::string::npos) {
        parts.push_back(string.substr(pos, nextPos - pos));

        pos = nextPos + 1;
        nextPos = string.find(delimiter, pos + 1);
    }

    if (pos < string.size()) {
        parts.push_back(string.substr(pos));
    }

    return parts;
}

bool ReadScore(std::ifstream& inStream, std::vector<int>& scoresToReadTo, int maxScoreCount, bool shouldBeSortedAscending)
{
    std::string scoresLine;
    if (std::getline(inStream, scoresLine)) {
        auto scores = Split(scoresLine, " ");
        std::transform(scores.begin(), scores.end(), std::back_inserter(scoresToReadTo), [](std::string score) { return std::stoi(score); });

        if (scoresToReadTo.size() > maxScoreCount) {
            // This should not happen. The config high score file was modified by someone else
            scoresToReadTo.erase(scoresToReadTo.begin() + maxScoreCount, scoresToReadTo.end());
        }

        // This shouldn't be necessary, but let's just make sure everything is in order
        if (shouldBeSortedAscending) {
            std::sort(scoresToReadTo.begin(), scoresToReadTo.end());
        } else {
            std::sort(scoresToReadTo.begin(), scoresToReadTo.end(), std::greater<int> {});
        }

        return true;
    }

    return false;
}
}

HighScore::HighScore()
{
    _classicScores.reserve(ScoresRemembered);
    _quickDeathScores.reserve(ScoresRemembered);
}

bool HighScore::ReadHighScore()
{
    _classicScores.clear();
    _quickDeathScores.clear();

    try {
        if (std::filesystem::exists(HighScoreFilePath)) {
            std::ifstream highScoreStream { HighScoreFilePath };

            if (ReadScore(highScoreStream, _classicScores, ScoresRemembered, false) && ReadScore(highScoreStream, _quickDeathScores, ScoresRemembered, true)) {
                return true;
            }
        }
    } catch (std::exception& e) {
        auto str = e.what();
    }

    return false;
}

bool HighScore::WriteHighScore()
{
    try {
        std::ofstream highScoreOut { HighScoreFilePath };

        for (auto score : _classicScores) {
            highScoreOut << score << " ";
        }

        highScoreOut << std::endl;

        for (auto score : _quickDeathScores) {
            highScoreOut << score << " ";
        }

        return true;

    } catch (const std::exception&) {
    }

    return false;
}

void HighScore::AddScore(GameMode mode, int newScore)
{
    auto& scores = mode == GameMode::Classic ? _classicScores : _quickDeathScores;

    auto elementToInsertBefore = mode == GameMode::Classic
        ? std::find_if(scores.begin(), scores.end(), [newScore](int score) { return newScore > score; })
        : std::find_if(scores.begin(), scores.end(), [newScore](int score) { return newScore < score; });

    if (scores.size() < ScoresRemembered) {
        scores.insert(elementToInsertBefore, newScore);
    } else if (elementToInsertBefore != scores.begin()) {
        // Shift all elements that are lowert than the current element down by 1
        for (auto shiftIt = scores.begin() + 1; shiftIt != elementToInsertBefore; ++shiftIt) {
            *(shiftIt - 1) = *shiftIt;
        }
        *(elementToInsertBefore - 1) = newScore;
    }
}

const std::vector<int>& HighScore::GetClassicScores() const
{
    return _classicScores;
}

const std::vector<int>& HighScore::GetQuickDeathScores() const
{
    return _quickDeathScores;
}
