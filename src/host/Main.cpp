/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "JuceLibraryCode/JuceHeader.h"
#include "IPCAudioIODevice.h"

//==============================================================================
class vstjshostApplication : public JUCEApplication {
public:
  //==============================================================================
  vstjshostApplication() {}

  const String getApplicationName() override {
    return ProjectInfo::projectName;
  }

  const String getApplicationVersion() override {
    return ProjectInfo::versionString;
  }

  bool moreThanOneInstanceAllowed() override { return true; }

  //==============================================================================
  void initialise(const String &commandLine) override {
    // This method is where you should put your application's initialisation
    // code..
    StringArray params = this->getCommandLineParameterArray();
    String pluginPath = params[0];
    String socketAddress = params[1];
    mainWindow = new MainWindow("Plugin Host", pluginPath, socketAddress);
  }

  void shutdown() override {
    // Add your application's shutdown code here..

    mainWindow = nullptr; // (deletes our window)
  }

  //==============================================================================
  void systemRequestedQuit() override {
    // This is called when the app is being asked to quit: you can ignore this
    // request and let the app carry on running, or call quit() to allow the app
    // to close.
    quit();
  }

  void anotherInstanceStarted(const String &commandLine) override {
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the commandLine parameter tells you what
    // the other instance's command-line arguments were.
  }

  //==============================================================================
  /*
      This class implements the desktop window that contains an instance of
      our MainContentComponent class.
  */
  class MainWindow : public DocumentWindow {
  public:
    MainWindow(String name, String pluginPath, String socketAddress)
        : DocumentWindow(name, Colours::lightgrey, DocumentWindow::allButtons) {
      formatManager.addDefaultFormats();
      setUsingNativeTitleBar(true);
      setResizable(true, true);

      centreWithSize(getWidth(), getHeight());
      setVisible(true);

      AudioDeviceManager::AudioDeviceSetup setup;
      deviceManager.getAudioDeviceSetup(setup);

      PropertiesFile::Options options;
      options.applicationName = "Juce Audio Plugin Host";
      options.filenameSuffix = "settings";
      options.osxLibrarySubFolder = "Preferences";

      appProperties = new ApplicationProperties();
      appProperties->setStorageParameters(options);

      //    ScopedPointer<XmlElement> savedAudioState
      //    (appProperties->getUserSettings() ->getXmlValue
      //    ("audioDeviceState"));
      std::unique_ptr<XmlElement> savedAudioState =
          appProperties->getUserSettings()->getXmlValue("audioDeviceState");

      auto ipcType = std::unique_ptr<AudioIODeviceType>(new IPCAudioIODeviceType(socketAddress));
      deviceManager.addAudioDeviceType(std::move(ipcType));
      if (deviceManager.getCurrentAudioDeviceType() != "IPC") {
        deviceManager.setCurrentAudioDeviceType("IPC", true);
      }

      deviceManager.initialise(256, 256, savedAudioState.get(), true);

      OwnedArray<juce::PluginDescription> foundPlugins;
      VST3PluginFormat format;
      format.findAllTypesForFile(foundPlugins, pluginPath);

      description = foundPlugins[0];
      auto instance = format.createInstanceFromDescription(
          *description, setup.sampleRate, setup.bufferSize);

      // i/o graph nodes
      auto inputProcessor = std::unique_ptr<AudioProcessor>(new AudioProcessorGraph::AudioGraphIOProcessor(
          AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
      auto outputProcessor = std::unique_ptr<AudioProcessor>(new AudioProcessorGraph::AudioGraphIOProcessor(
          AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));

      deviceManager.addAudioCallback(&graphPlayer);
      graphPlayer.setProcessor(&graph);

      // Add all nodes to graph
      inputNode = std::unique_ptr<AudioProcessorGraph::Node>(graph.addNode(std::move(inputProcessor)));
      pluginNode = std::unique_ptr<AudioProcessorGraph::Node>(graph.addNode(std::move(instance)));
      outputNode = std::unique_ptr<AudioProcessorGraph::Node>(graph.addNode(std::move(outputProcessor)));

      graph.addConnection({ { inputNode->nodeID, 0 }, { pluginNode->nodeID, 0 } });
      graph.addConnection({ { inputNode->nodeID, 1 }, { pluginNode->nodeID, 1 } });

      graph.addConnection({ { pluginNode->nodeID, 0 }, { outputNode->nodeID, 0 } });
      graph.addConnection({ { pluginNode->nodeID, 1 }, { outputNode->nodeID, 1 } });

      // Get UI and add it to main window
      editor = instance->createEditor();
      this->setContentOwned(editor, true);

    }

    void closeButtonPressed() override {
      // This is called when the user tries to close this window. Here, we'll
      // just
      // ask the app to quit when this happens, but you can change this to do
      // whatever you need.
      JUCEApplication::getInstance()->systemRequestedQuit();
    }

    /* Note: Be careful if you override any DocumentWindow methods - the base
       class uses a lot of them, so by overriding you might break its
       functionality.
       It's best to do all your work in your content component instead, but if
       you really have to override any DocumentWindow methods, make sure your
       subclass also calls the superclass's method.
    */

  private:
    AudioDeviceManager deviceManager;
    AudioPluginFormatManager formatManager;
    ScopedPointer<PluginDescription> description;
    ScopedPointer<AudioProcessorEditor> editor;
    AudioProcessorGraph graph;
    AudioProcessorPlayer graphPlayer;

    // std::unique_ptr<AudioProcessor> inputProcessor;
    // ScopedPointer<AudioProcessor> outputProcessor;

    std::unique_ptr<AudioProcessorGraph::Node> inputNode;
    std::unique_ptr<AudioProcessorGraph::Node> outputNode;

    std::unique_ptr<AudioProcessorGraph::Node> pluginNode;

    ScopedPointer<ApplicationProperties> appProperties;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  };

private:
  ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(vstjshostApplication)
