#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::AudioAppComponent, private juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    bool loadFile(const juce::File& file);
    void startRecording();
    void stop();
    void startPlayback();

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread backgroundThread{"Audio Recorder Thread"};
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    std::unique_ptr<juce::FileOutputStream> fileOutputStream;
    std::unique_ptr<juce::AudioFormatWriter> audioWriter;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioTransportSource anotherTransportSource;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioFormatReader* reader = nullptr;
    juce::AudioDeviceManager deviceManager;

    bool isRecording = false;
    bool isPlaybackReady = false;
    juce::File recordedFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
