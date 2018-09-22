/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int)
{
}

const String PluginProcessor::getProgramName (int)
{
    return {};
}

void PluginProcessor::changeProgramName (int, const String&)
{
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    audioSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void PluginProcessor::releaseResources()
{
    audioSource.releaseResources ();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PluginProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    int totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int channel = totalNumInputChannels;
        channel < totalNumOutputChannels;
        ++channel)
    {
        buffer.clear (channel, 0, buffer.getNumSamples());
    }

    int intputBusesCount = getBusCount (true);

    for (int busIndex = 0; busIndex < intputBusesCount; ++busIndex)
    {
        AudioBuffer<float> inputBuffer = getBusBuffer (buffer, true, busIndex);
        audioSource.getNextAudioBlock (AudioSourceChannelInfo(inputBuffer));
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (MemoryBlock&)
{
}

void PluginProcessor::setStateInformation (const void*, int)
{
}

TenFtAudioSource& PluginProcessor::getAudioSource ()
{
    return audioSource;
}

//==============================================================================
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
