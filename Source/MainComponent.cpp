#include "MainComponent.h"

MainComponent::MainComponent()
{
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);
    deviceManager.initialise(2, 2, nullptr, true); // Initialize with both input and output channels
    deviceManager.addAudioCallback(&audioSourcePlayer);
    backgroundThread.startThread();
}

MainComponent::~MainComponent()
{
    stop();
    backgroundThread.stopThread(1000);
    deviceManager.removeAudioCallback(&audioSourcePlayer);
    shutdownAudio();
}

bool MainComponent::loadFile(const juce::File& file)
{
    reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
        return false;

    readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
    transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
    //anotherTransportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
    audioSourcePlayer.setSource(&transportSource);
    return true;
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    juce::ignoreUnused(samplesPerBlockExpected);
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (isRecording)
    {
        // Fill the buffer with playback audio
        //transportSource.getNextAudioBlock(bufferToFill);
        //anotherTransportSource.getNextAudioBlock(bufferToFill);

        // Get the array of pointers to each channel's data
        auto* channelData = bufferToFill.buffer->getArrayOfReadPointers();

        // Write the buffer to the file
        if (threadedWriter != nullptr)
        {
            threadedWriter->write(channelData, bufferToFill.numSamples);
        }
    }
    else
    {
        // Clear the buffer if not recording
        bufferToFill.clearActiveBufferRegion();
    }
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::startRecording()
{
    setAudioChannels(2, 2); // Set channels for both playback and recording

    // Prepare file for recording
    recordedFile = juce::File::getCurrentWorkingDirectory().getChildFile("recordedAudio.wav");
    recordedFile.deleteFile(); // Remove old file

    fileOutputStream = std::make_unique<juce::FileOutputStream>(recordedFile);
    if (fileOutputStream && fileOutputStream->openedOk())
    {
        juce::WavAudioFormat wavFormat;
        audioWriter.reset(wavFormat.createWriterFor(fileOutputStream.release(), deviceManager.getCurrentAudioDevice()->getCurrentSampleRate(), 1, 16, {}, 0));

        if (audioWriter)
        {
            threadedWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(audioWriter.release(), backgroundThread, 32768);
            backgroundThread.startThread();
            isRecording = true;

            std::cout << "Recording playback started..." << std::endl;

            startPlayback(); // Start playback
        }
        else
        {
            std::cerr << "Error creating audio writer." << std::endl;
            stop(); // Ensure we stop everything if setup fails
        }
    }
    else
    {
        std::cerr << "Error opening file for recording." << std::endl;
        stop(); // Ensure we stop everything if file opening fails
    }
}


void MainComponent::startPlayback()
{
    if (readerSource != nullptr)
    {
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
        //anotherTransportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
        transportSource.start();
        //anotherTransportSource.start();
        std::cout << "Playback started..." << std::endl;

        //Keep checking until playback and recording are done
        while (transportSource.isPlaying() && isRecording)
        {
        //     juce::Thread::sleep(100); // Sleep for 100 ms to avoid high CPU usage
        }

        if (!isRecording)
        {
            std::cerr << "Recording stopped prematurely." << std::endl;
        }
        else
        {
            std::cout << "Playback finished." << std::endl;
        }
    }
    else
    {
        std::cerr << "No audio file loaded for playback." << std::endl;
    }
}

void MainComponent::stop()
{
    if(isRecording){
        isRecording = false;
        threadedWriter.reset();
    }
    transportSource.stop();
    //anotherTransportSource.stop();
    transportSource.setSource(nullptr);
    //anotherTransportSource.setSource(nullptr);

    setAudioChannels(0, 0); // No input or output channels for stopping
    //threadedWriter.reset();
    //fileOutputStream.reset();
    //isRecording = false;

    std::cout << "Playback and Recording stopped..." << std::endl;
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.hasStreamFinished())
        {
           transportSource.setSource(nullptr);
        }
    }
}
