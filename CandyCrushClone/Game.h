#pragma once

#include "AudioPlayer.h"
#include "GameMode.h"
#include "GameWorld.h"
#include "HighScore.h"
#include "MainMenu.h"
#include "Player.h"
#include "Screen.h"

class Game {
public:
    Game();

    void RunMainLoop();

private:
    enum class GameState {
        Paused,
        Playing,
    };

    static constexpr int DesiredFPS = 60;
    static constexpr int FrameTime = int(1000.f / DesiredFPS);

    std::unique_ptr<Screen> _screen;
    std::unique_ptr<InputProcessor> _inputProcessor;
    std::unique_ptr<GameWorld> _gameWorld;
    std::unique_ptr<MainMenu> _menu;
    std::unique_ptr<Player> _player;
    std::unique_ptr<HighScore> _highScore;
    std::unique_ptr<AudioPlayer> _audioPlayer;

    bool _shouldQuit = false;
    GameState _gameState = GameState::Paused;

    std::unique_ptr<EventToken> _keyPressedToken;
    std::unique_ptr<EventToken> _mouseClickedToken;
    std::unique_ptr<IGameState> _gameStateObject;

    void ProcessEvents();
    void HandleKeyPress(Key key);
    void HandleButtonClicked(ButtonType button);
    void ToggleIsPlaying();
    void EndGame(bool menuNeedsResumeButton, const std::vector<std::string>& additionalMenuText);
    void ResumeOrStartGame(std::optional<GameMode> gameMode = std::nullopt);
};
