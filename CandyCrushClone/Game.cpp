#include "Game.h"
#include "Event.h"
#include "GameState.h"
#include "InputProcessor.h"

#include <filesystem>
#include <fstream>
#include <iostream>

Game::Game()
    : _screen(Screen::GetScreen())
    , _inputProcessor(std::make_unique<InputProcessor>())
    , _highScore(std::make_unique<HighScore>())
{
    if (!_screen) {
        std::cerr << "Failed to initialize screen. Terminating..." << std::endl;
        std::terminate();
    }

    _audioPlayer = std::make_unique<AudioPlayer>();
    _gameWorld = std::make_unique<GameWorld>(8, 8, 5, *_screen, *_audioPlayer);
    _menu = std::make_unique<MainMenu>(*_screen, *_inputProcessor);
    _player = std::make_unique<Player>(*_inputProcessor, *_gameWorld);

    // This is not strictly necessary, the game can be played without sound as well, so we don't terminate here
    if (!_audioPlayer->Initialize()) {
        std::cerr << "Failed to initialize SDL Mixer" << std::endl;
    }

    _keyPressedToken = _inputProcessor->KeyPressed.Subscribe([this](Key key) { HandleKeyPress(key); });
    _mouseClickedToken = _menu->ButtonClicked.Subscribe([this](ButtonType button) { HandleButtonClicked(button); });

    _highScore->ReadHighScore();

    _menu->Activate(false, {});
}

void Game::RunMainLoop()
{
    auto previous = SDL_GetTicks64();

    while (!_shouldQuit) {
        auto now = SDL_GetTicks64();
        auto delta = now - previous;

        ProcessEvents();

        // Update it here, so we have background music in the main menu as well
        _audioPlayer->Update();

        _screen->BeginFrame();

        switch (_gameState) {
        case Game::GameState::Paused: {
            _menu->Draw();
        } break;
        case Game::GameState::Playing: {
            _gameWorld->Update(delta);

            if (_gameStateObject->IsGameOver()) {
                _highScore->AddScore(_gameStateObject->GetGameMode(), _gameStateObject->GetScore());
                _highScore->WriteHighScore();

                auto result = _gameStateObject->GetResult();
                _gameStateObject.reset();
                EndGame(false, result);
            } else {
                _gameWorld->Draw();
            }
        } break;
        }

        _screen->Present();

        if (delta <= FrameTime) {
            SDL_Delay(uint32_t(FrameTime - delta));
        }

        previous = now;
    }

    _highScore->WriteHighScore();
}

void Game::ProcessEvents()
{
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT: {
            _shouldQuit = true;
        } break;
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            _inputProcessor->ProcessMouseEvent(e);
        } break;
        case SDL_KEYDOWN: {
            _inputProcessor->ProcessKeyEvent(e);
        } break;
        default:
            break;
        }
    }
}

void Game::HandleKeyPress(Key key)
{
    switch (key) {
    case Key::Escape: {
        ToggleIsPlaying();
    } break;
    }
}

void Game::HandleButtonClicked(ButtonType button)
{
    switch (button) {
    case ButtonType::Resume: {
        ToggleIsPlaying();
    } break;
    case ButtonType::Quit: {
        _shouldQuit = true;
    } break;
    case ButtonType::Classic: {
        ResumeOrStartGame(GameMode::Classic);
    } break;
    case ButtonType::QuickDeath: {
        ResumeOrStartGame(GameMode::QuickDeath);
    } break;
    case ButtonType::Leaderboard: {
        _menu->ShowLeaderboard(_highScore->GetClassicScores(), _highScore->GetQuickDeathScores());
    } break;
    case ButtonType::ToggleMusic: {
        _audioPlayer->ToggleIsMusicEnabled();
    } break;
    }
}
void Game::ToggleIsPlaying()
{
    switch (_gameState) {
    case Game::GameState::Paused: {
        ResumeOrStartGame();
    } break;
    case Game::GameState::Playing: {
        EndGame(true, {});
    } break;
    }
}

void Game::EndGame(bool menuNeedsResumeButton, const std::vector<std::string>& additionalMenuText)
{
    _gameWorld->Deactivate();
    _menu->Activate(menuNeedsResumeButton, additionalMenuText);
    _gameState = GameState::Paused;
}

void Game::ResumeOrStartGame(std::optional<GameMode> gameMode)
{
    if (gameMode) {
        switch (*gameMode) {
        case GameMode::Classic: {
            if (auto currentStateObject = dynamic_cast<ClassicGameState*>(_gameStateObject.get()); !currentStateObject || currentStateObject != _gameStateObject.get()) {
                _gameStateObject = std::make_unique<ClassicGameState>();
            }
        } break;
        case GameMode::QuickDeath: {
            if (auto currentStateObject = dynamic_cast<QuickDeathGameState*>(_gameStateObject.get()); !currentStateObject || currentStateObject != _gameStateObject.get()) {
                _gameStateObject = std::make_unique<QuickDeathGameState>();
            }
        } break;
        }
    }

    _menu->Deactivate();
    _gameWorld->Activate(*_gameStateObject);
    _gameState = GameState::Playing;
}
