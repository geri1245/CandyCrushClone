#include "AudioPlayer.h"

#include <SDL.h>

#include <cassert>
#include <filesystem>
#include <iostream>

namespace {
static constexpr const char* const BackgroundMusicFolder = "Assets/Sounds/BackgroundTracks/";
static constexpr const char* const TileDisappearEffectPath = "./Assets/Sounds/disappear.mp3";
}

AudioPlayer::AudioPlayer()
    : _randomEngine(_randomDevice())
{
}

AudioPlayer::~AudioPlayer()
{
    for (auto* music : _backgroundTracks) {
        Mix_FreeMusic(music);
    }

    Mix_CloseAudio();
}

bool AudioPlayer::Initialize()
{

    int flags = MIX_INIT_MP3;

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL audio" << std::endl;
    }

    if (flags != Mix_Init(flags)) {
        std::cerr << "Failed to initialize SDL mixer: " << Mix_GetError() << std::endl;
    }

    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);

    Mix_AllocateChannels(1);

    _isInitialized = true;

    LoadBackgroundMusicTracks();

    if (_backgroundTracks.size() < 2) {
        _isInitialized = false;
        return false;
    }

    LoadSoundEffects();

    _randomDistribution = std::uniform_int_distribution<int>(0, int(_backgroundTracks.size() - 1));

    return true;
}

void AudioPlayer::ToggleIsMusicEnabled()
{
    _isMusicEnabled = !_isMusicEnabled;

    if (!_isMusicEnabled) {
        Mix_HaltMusic();
    }
}

void AudioPlayer::Update()
{
    if (!_isInitialized || !_isMusicEnabled)
        return;

    if (Mix_PlayingMusic() == 0) {
        auto newTrackIndex = _randomDistribution(_randomEngine);
        if (newTrackIndex == _lastPlayedMusicIndex) {
            newTrackIndex = (newTrackIndex + 1) % _backgroundTracks.size();
        }

        Mix_FadeInMusic(_backgroundTracks[_lastPlayedMusicIndex], 1, 5000);
        _lastPlayedMusicIndex = newTrackIndex;
    }
}

void AudioPlayer::PlaySoundEffect(SoundEffect effect)
{
    if (!_isInitialized)
        return;

    auto effectIt = _soundEffects.find(effect);

    if (effectIt == _soundEffects.end()) {
        assert(false && "Trying to play a sound effect that wasn't loaded.");
        return;
    }

    Mix_PlayChannel(0, effectIt->second, 0);
}

void AudioPlayer::LoadBackgroundMusicTracks()
{
    auto assetPath = std::filesystem::current_path() / BackgroundMusicFolder;
    for (auto const& dirEntry : std::filesystem::directory_iterator { assetPath }) {
        Mix_Music* music = Mix_LoadMUS(dirEntry.path().string().c_str());
        if (music != nullptr) {
            _backgroundTracks.push_back(music);
        } else {
            _isInitialized = false;
        }
    }
}

void AudioPlayer::LoadSoundEffects()
{
    if (Mix_Chunk* chunk = Mix_LoadWAV(TileDisappearEffectPath)) {
        _soundEffects[SoundEffect::TileDisappear] = chunk;
    } else {
        _isInitialized = false;
    }
}
