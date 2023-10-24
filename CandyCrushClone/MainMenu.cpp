#include <algorithm>
#include <iterator>

#include "MainMenu.h"

namespace {
bool Contains(const SDL_Rect& rect, Vec2 position)
{
    return position.x >= rect.x && position.y >= rect.y && position.x <= rect.x + rect.w && position.y <= rect.y + rect.h;
}

int GetCenteredPositionOfElement(int screenWidth, int elementWidth)
{
    return (screenWidth - elementWidth) / 2;
}
}

namespace {
std::string GetTextForButtonType(ButtonType buttonType)
{
    switch (buttonType) {
    case ButtonType::Resume:
        return "Resume Game";
    case ButtonType::Quit:
        return "Quit Game";
    case ButtonType::Classic:
        return "Play Classic";
    case ButtonType::QuickDeath:
        return "Play Quick Death";
    case ButtonType::Leaderboard:
        return "Show Leaderboard";
    case ButtonType::Back:
        return "Back";
    }

    return "ERROR - STRING NOT FOUND!!!";
}
}

MainMenu::MainMenu(const Screen& screen, InputProcessor& inputProcessor)
    : _screen(&screen)
    , _inputProcessor(&inputProcessor)
    , _buttonTypes {
        ButtonType::Classic,
        ButtonType::QuickDeath,
        ButtonType::Leaderboard,
        ButtonType::Quit,
    }
{
    MakeMenuFromButtonTypes();
    _leaderboardButtons = std::vector<Button> {
        GetMusicButton(),
        Button { ButtonType::Back, GetTextForButtonType(ButtonType::Back), SDL_Rect { GetCenteredPositionOfElement(screen.ScreenWidth, ButtonWidth), screen.ScreenHeight - 80, ButtonWidth, ButtonHeight } },
    };
}

void MainMenu::Draw()
{
    for (const auto& button : CurrentButtons()) {
        _screen->DrawButton(button.Text, button.Position, button.Type == _hoveredButton);
    }

    if (_isShowingLeaderboard) {
        _screen->DrawBackgroundRectangle(_leaderboardBackground);
        for (const auto& textBlock : _leaderboard) {
            _screen->DrawText(textBlock.Text, textBlock.Position, false);
        }
    } else {
        for (const auto& textBlock : _additionalText) {
            _screen->DrawText(textBlock.Text, textBlock.Position, true);
        }
    }
}

void MainMenu::Activate(bool needsResumeButton, const std::vector<std::string>& additionalText)
{
    _additionalText.clear();
    MakeTextBlocksFromTexts(additionalText, _additionalText, 100, ButtonSpacing, ButtonHeight);

    if (needsResumeButton && _buttonTypes[0] != ButtonType::Resume) {
        _buttonTypes.insert(_buttonTypes.begin(), ButtonType::Resume);
        MakeMenuFromButtonTypes();
    } else if (!needsResumeButton && _buttonTypes[0] == ButtonType::Resume) {
        _buttonTypes.erase(_buttonTypes.begin());
        MakeMenuFromButtonTypes();
    }

    _mouseClickedEventToken = _inputProcessor->MouseClicked.Subscribe([this](Vec2 position) { TryClick(position); });
    _mouseMovedEventToken = _inputProcessor->MouseMoved.Subscribe([this](Vec2 position) { TryHover(position); });
}

void MainMenu::Deactivate()
{
    _mouseClickedEventToken.reset();
    _mouseMovedEventToken.reset();

    _hoveredButton.reset();
}

void MainMenu::ShowLeaderboard(const std::vector<int>& classicHighScores, const std::vector<int>& quickDeathHighScores)
{
    _leaderboard.clear();
    _leaderboard.reserve(12);

    _leaderboard.push_back(TextBlock { "Classic:", SDL_Rect {
                                                       GetCenteredPositionOfElement(_screen->ScreenWidth, ButtonWidth),
                                                       50,
                                                       ButtonWidth,
                                                       LeaderboardEntryHeight,
                                                   } });

    int nextYPosition = MakeTextBlocksFromMilliseconds(classicHighScores, _leaderboard, 50 + LeaderboardSpacing + LeaderboardEntryHeight, LeaderboardSpacing);

    _leaderboard.push_back(TextBlock { "Quick death:", SDL_Rect { GetCenteredPositionOfElement(_screen->ScreenWidth, ButtonWidth), nextYPosition, ButtonWidth, LeaderboardEntryHeight } });

    MakeTextBlocksFromMilliseconds(quickDeathHighScores, _leaderboard, nextYPosition + LeaderboardEntryHeight + LeaderboardSpacing, LeaderboardSpacing);

    _leaderboardBackground = SDL_Rect { _leaderboard[0].Position.x - LeaderboardSpacing, _leaderboard[0].Position.y - LeaderboardSpacing, ButtonWidth + 2 * LeaderboardSpacing, int(_leaderboard.size()) * (LeaderboardSpacing + LeaderboardEntryHeight) + LeaderboardSpacing };

    _isShowingLeaderboard = true;
}

void MainMenu::GoBackFromLeaderboard()
{
    _isShowingLeaderboard = false;
}

std::vector<MainMenu::Button>& MainMenu::CurrentButtons()
{
    return _isShowingLeaderboard ? _leaderboardButtons : _buttons;
}

MainMenu::Button MainMenu::GetMusicButton() const
{
    return Button {
        ButtonType::ToggleMusic,
        _isPlayingMusic ? "Music is ON" : "Music is OFF",
        SDL_Rect { _screen->ScreenWidth - ButtonWidth - 20, _screen->ScreenHeight - ButtonHeight - 10, ButtonWidth, ButtonHeight }
    };
}

void MainMenu::UpdateMusicButton()
{
    auto& currentButtons = CurrentButtons();
    auto musicButtonIt = std::find_if(currentButtons.begin(), currentButtons.end(), [](const Button& button) { return button.Type == ButtonType::ToggleMusic; });
    if (musicButtonIt != currentButtons.end()) {
        *musicButtonIt = GetMusicButton();
    }
}

void MainMenu::MakeMenuFromButtonTypes()
{
    auto screenWidth = _screen->ScreenWidth;
    auto screenHeight = _screen->ScreenHeight;

    auto numOfButtons = int(_buttonTypes.size());

    auto menuSize = numOfButtons * ButtonHeight + (numOfButtons - 1) * ButtonSpacing;
    auto buttonYPosition = (screenHeight - menuSize) / 2;
    auto buttonXPosition = GetCenteredPositionOfElement(screenWidth, ButtonWidth);

    _buttons.reserve(numOfButtons);
    _buttons.clear();

    _buttons.push_back(GetMusicButton());

    for (auto buttonType : _buttonTypes) {
        _buttons.push_back(Button {
            buttonType,
            GetTextForButtonType(buttonType),
            SDL_Rect { buttonXPosition, buttonYPosition, ButtonWidth, ButtonHeight },
        });

        buttonYPosition += (ButtonHeight + ButtonSpacing);
    }
}

int MainMenu::MakeTextBlocksFromTexts(const std::vector<std::string>& additionalText, std::vector<TextBlock>& resultTexts, int startingYPosition, int spacing, int height)
{
    auto midPoint = GetCenteredPositionOfElement(_screen->ScreenWidth, TextBlockWidth);
    SDL_Rect textRect { midPoint, startingYPosition, TextBlockWidth, height };
    for (const auto& text : additionalText) {
        resultTexts.push_back(TextBlock { text, textRect });
        textRect.y += height + spacing;
    }

    return textRect.y;
}

int MainMenu::MakeTextBlocksFromMilliseconds(const std::vector<int>& data, std::vector<TextBlock>& resultTexts, int startingYPosition, int spacing)
{
    std::vector<std::string> stringVec;
    stringVec.reserve(data.size());

    std::transform(data.begin(), data.end(), std::back_inserter(stringVec), [](int num) { return std::to_string(int(num / 1000.0)); });
    return MakeTextBlocksFromTexts(stringVec, resultTexts, startingYPosition, spacing, LeaderboardEntryHeight);
}

void MainMenu::TryClick(Vec2 position)
{
    for (const auto& button : CurrentButtons()) {
        if (Contains(button.Position, position)) {
            // Handle some buttons' action here before forwarding it to the subscribers
            if (button.Type == ButtonType::Back) {
                GoBackFromLeaderboard();
            } else if (button.Type == ButtonType::ToggleMusic) {
                _isPlayingMusic = !_isPlayingMusic;
                UpdateMusicButton();
            }

            ButtonClicked.Invoke(button.Type);
        }
    }
}

void MainMenu::TryHover(Vec2 position)
{
    _hoveredButton.reset();

    for (const auto& button : CurrentButtons()) {
        if (Contains(button.Position, position)) {
            _hoveredButton = button.Type;
        }
    }
}
