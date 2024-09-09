#include <JuceHeader.h>
#include "MainComponent.h"

int main()
{
    juce::initialiseJuce_GUI();

    MainComponent mainComponent;

    // Manually specify the file path 
    std::cout << "Please enter the full path to the file: ";
    std::string filePath;
    std::getline(std::cin, filePath);

    juce::File file(filePath);

    if (file.existsAsFile())
    {
        if (mainComponent.loadFile(file))
        {
            mainComponent.startRecording();
        }
        else
        {
            std::cout << "Failed to load the selected file." << std::endl;
        }
    }
    else
    {
        std::cout << "File does not exist or invalid path." << std::endl;
    }

    juce::shutdownJuce_GUI();
    return 0;
}
