#pragma once

#include "Event.h"
#include "InputProcessor.h"
#include "Screen.h"

#include <string>
#include <vector>

enum class ButtonType {
    Resume,
    Quit,
    Back,
    Classic,
    Leaderboard,
    QuickDeath,
    ToggleMusic,
};

class MainMenu {
public:
    MainMenu(const Screen& screen, InputProcessor& inputProcessor);

    void Draw();
    void Activate(bool needsResumeButton, const std::vector<std::string>& additionalText);
    void Deactivate();
    void ShowLeaderboard(const std::vector<int>& classicHighScores, const std::vector<int>& quickDeathHighScores);

    Event<std::function<void(ButtonType clickedButton)>> ButtonClicked;

private:
    static constexpr int ButtonHeight = 40;
    static constexpr int ButtonWidth = 280;
    static constexpr int TextBlockWidth = 460;
    static constexpr int ButtonSpacing = 20;
    static constexpr int LeaderboardEntryHeight = 25;
    static constexpr int LeaderboardSpacing = 5;

    struct Button {
        ButtonType Type;
        std::string Text;
        SDL_Rect Position;
    };

    struct TextBlock {
        std::string Text;
        SDL_Rect Position;
    };

    const Screen* _screen;
    InputProcessor* _inputProcessor;
    std::unique_ptr<EventToken> _userClickedEventToken;
    std::vector<Button> _buttons;
    std::vector<Button> _leaderboardButtons;

    std::vector<TextBlock> _additionalText;
    std::vector<TextBlock> _leaderboard;
    SDL_Rect _leaderboardBackground;

    std::vector<ButtonType> _buttonTypes;

    std::unique_ptr<EventToken> _mouseClickedEventToken;
    std::unique_ptr<EventToken> _mouseMovedEventToken;
    std::optional<ButtonType> _hoveredButton;

    bool _isShowingLeaderboard = false;
    bool _isPlayingMusic = true;

    void MakeMenuFromButtonTypes();
    int MakeTextBlocksFromTexts(const std::vector<std::string>& additionalText, std::vector<TextBlock>& resultTexts, int startingYPosition, int spacing, int height);
    int MakeTextBlocksFromMilliseconds(const std::vector<int>& data, std::vector<TextBlock>& resultTexts, int startingYPosition, int spacing);
    void TryClick(Vec2 position);
    void TryHover(Vec2 position);
    void GoBackFromLeaderboard();
    std::vector<Button>& CurrentButtons();
    Button GetMusicButton() const;

    void UpdateMusicButton();
};
