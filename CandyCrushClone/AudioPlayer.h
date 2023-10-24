#pragma once

#include <SDL_mixer.h>

#include <random>
#include <unordered_map>
#include <vector>

class AudioPlayer {
public:
    enum class SoundEffect { TileDisappear };

    AudioPlayer();
    ~AudioPlayer();

    bool Initialize();
    void ToggleIsMusicEnabled();
    void Update();

    void PlaySoundEffect(SoundEffect effect);

private:
    std::vector<Mix_Music*> _backgroundTracks;
    int _lastPlayedMusicIndex = 0;

    std::random_device _randomDevice;
    std::mt19937 _randomEngine;
    std::uniform_int_distribution<int> _randomDistribution;

    std::unordered_map<SoundEffect, Mix_Chunk*> _soundEffects;

    void LoadBackgroundMusicTracks();
    void LoadSoundEffects();

    bool _isInitialized = false;
    bool _isMusicEnabled = true;
};
