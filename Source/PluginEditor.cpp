/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processor (p), mainComponent(p.getAudioSource())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
	addAndMakeVisible(&mainComponent);
	setSize(1000, 800);
    //setOpaque (false);
}

PluginEditor::~PluginEditor()
{
}

//==============================================================================
//void PluginEditor::paint (Graphics& g)
//{
//    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    //g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
//
//    //g.setColour (Colours::white);
//    //g.setFont (15.0f);
//    //g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
//}

void PluginEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	mainComponent.setBounds(getLocalBounds());
}
