/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
REversalDistortionAudioProcessor::REversalDistortionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
     , fDistortion(new juce::AudioParameterFloat("distortion", "Distortion",
                                                 0.0f, 1.0f, 0.5f))
     , fThreshold(new juce::AudioParameterFloat("threshold", "Threshold",
                                                0.0f, 0.005f, 0.0025f))
#endif
{
    addParameter(fDistortion);
    addParameter(fThreshold);
}

REversalDistortionAudioProcessor::~REversalDistortionAudioProcessor()
{
}

//==============================================================================
const juce::String REversalDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool REversalDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool REversalDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool REversalDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double REversalDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int REversalDistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int REversalDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void REversalDistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String REversalDistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void REversalDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void REversalDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void REversalDistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool REversalDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float REversalDistortionAudioProcessor::processDistortion(float in1)
{
  double resultSignal;

  if (std::fabs(in1) > *fThreshold)
  {
    if (in1 >= 0.0f)
      resultSignal = -M_PI_2 / 4;
    else
      resultSignal = M_PI_2 / 4;
  }
  else
    resultSignal = 0;
  
  assert(!std::isnan(resultSignal) &&
         std::abs(resultSignal) <= std::numeric_limits<float>::max());

  return in1 + resultSignal * *fDistortion;
}

void REversalDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* inBuffer = buffer.getReadPointer(channel % totalNumInputChannels);
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
        for (auto samples = 0; samples < buffer.getNumSamples(); samples++)
            *channelData++ = processDistortion(*inBuffer++);
    }
}

//==============================================================================
bool REversalDistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* REversalDistortionAudioProcessor::createEditor()
{
    return new REversalDistortionAudioProcessorEditor (*this);
}

//==============================================================================
void REversalDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void REversalDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new REversalDistortionAudioProcessor();
}
