#pragma once
#include "Tako.hpp"
#include <unordered_map>

class AudioClips
{
public:
	static tako::AudioClip* Load(const char* path)
	{
		auto i = I();
		if (!i.m_clips.count(path))
		{
			i.m_clips.emplace(path, new tako::AudioClip(path));
		}
		return i.m_clips[path];
	}

	static void Play(const char* path)
	{
		tako::Audio::Play(*Load(path));
	}
private:
	AudioClips() = default;
	std::unordered_map<const char*, tako::AudioClip*> m_clips;

	static AudioClips& I()
	{
		static AudioClips i;
		return i;
	}
};