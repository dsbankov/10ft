/*
  ==============================================================================

	AudioPlayer.h
	Created: 18 May 2018 9:53:34pm
	Author:  DBANKOV

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

class AudioFileTransportSource : public AudioTransportSource, private ChangeListener
{
public:

	enum AudioPlayerState
	{
		NoFileLoaded,
		Starting,
		Playing,
		Stopping,
		Stopped,
		Pausing,
		Paused
	};

	AudioFileTransportSource(AudioFormatManager& formatManager)
		: state(NoFileLoaded), formatManager(formatManager), AudioTransportSource()
	{
		formatManager.registerBasicFormats();
		addChangeListener(this);
	}

	~AudioFileTransportSource()
	{
		setSource(nullptr);
	}

	bool loadAudio(File& file)
	{
		auto* reader = formatManager.createReaderFor(file);
		if (reader != nullptr)
		{
			std::unique_ptr<AudioFormatReaderSource> tempReaderSource(new AudioFormatReaderSource(reader, true));
			setSource(tempReaderSource.get(), 0, nullptr, reader->sampleRate);
			readerSource.swap(tempReaderSource);
			changeState(Stopped);
			setPosition(0.0);
			return true;
		}
		else
		{
			unloadAudio();
			return false;
		}
	}

	bool isFileLoaded()
	{
		return state != NoFileLoaded;
	}

	void playAudio()
	{
		if (state == Playing)
			changeState(Pausing);
		else
			changeState(Starting);
	}

	void stopAudio()
	{
		if (state == Paused || state == NoFileLoaded)
			changeState(Stopped);
		else
			changeState(Stopping);
	}

	void pauseAudio()
	{
		changeState(Pausing);
	}

	void setLooping(bool shouldLoop) override
	{
		readerSource.get()->setLooping(shouldLoop);
	}

	std::function<void(AudioFileTransportSource::AudioPlayerState)> onStateChange;

private:

	void unloadAudio()
	{
		changeState(NoFileLoaded);
	}

	void changeState(AudioFileTransportSource::AudioPlayerState newState)
	{
		if (state != newState)
		{
			state = newState;

			switch (state)
			{
			case Starting:
				start();
				break;

			case Playing:
				break;

			case Stopping:
				stop();
				break;

			case Stopped:
				setPosition(0.0);
				break;

			case Pausing:
				stop();
				break;

			case Paused:
				break;

			case NoFileLoaded:
				setSource(nullptr);
				break;
			}

			onStateChange(newState);

		}
	}

	void changeListenerCallback(ChangeBroadcaster* broadcaster) override
	{
		if (broadcaster == this)
		{
			if (isPlaying())
				changeState(Playing);
			else if (state == Pausing)
				changeState(Paused);
			else
				changeState(Stopped);
		}
	}

	//==============================================================================================

	AudioFormatManager & formatManager;
	std::unique_ptr<AudioFormatReaderSource> readerSource;
	AudioPlayerState state;

};