//
// Created by David Ramirez on 9/12/16.
//

#ifndef VST_JS_HOST_IPCAUDIOIODEVICE_H
#define VST_JS_HOST_IPCAUDIOIODEVICE_H

#include "../../shared/JuceLibraryCode/JuceHeader.h"
#include "iobuffer.grpc.pb.h"
#include "zhelpers.hpp"
#include <grpc++/grpc++.h>

class IPCAudioIODevice : public AudioIODevice, public vstjs::RpcAudioIO::Service {
public:
  IPCAudioIODevice(const String &deviceName, const String _socketAddress);
  ~IPCAudioIODevice() {}
  StringArray getOutputChannelNames() override { return *inputChannelNames; };
  StringArray getInputChannelNames() override { return *outputChannelNames; };
  Array<double> getAvailableSampleRates() override { return *sampleRates; };
  Array<int> getAvailableBufferSizes() { return *bufferSizes; };
  int getDefaultBufferSize() { return bufferSizes->getFirst(); };
  String open(const BigInteger &inputChannels, const BigInteger &outputChannels,
              double sampleRate, int bufferSizeSamples) override;
  void close() override;
  bool isOpen() override;
  void start(AudioIODeviceCallback *callback) override;
  void stop() override;
  bool isPlaying() override;
  String getLastError() override;
  int getCurrentBufferSizeSamples() override;
  double getCurrentSampleRate() override;
  int getCurrentBitDepth() override;
  BigInteger getActiveOutputChannels() const override;
  BigInteger getActiveInputChannels() const override;
  int getOutputLatencyInSamples() override;
  int getInputLatencyInSamples() override;

  bool hasControlPanel() const override { return false; }
  bool showControlPanel() override { return false; }
  bool setAudioPreprocessingEnabled(bool shouldBeEnabled) override {
    return false;
  }

  grpc::Status ProcessAudioBlock (grpc::ServerContext* context, const vstjs::AudioBlock* request, vstjs::AudioBlock* reply) override;

private:
  ScopedPointer<StringArray> inputChannelNames;
  ScopedPointer<StringArray> outputChannelNames;
  ScopedPointer<Array<double>> sampleRates;
  ScopedPointer<Array<int>> bufferSizes;
  ScopedPointer<Array<int>> bitDepths;
  ScopedPointer<AudioIODeviceCallback> callback;

  ScopedPointer<grpc::ServerBuilder> serverBuilder;
  ScopedPointer<RpcAudioService> rpcAudioService;
  std::unique_ptr<grpc::Server> serviceInstance;

  const String socketAddress;

  zmq::context_t context;
  void prepareInputData(vstjs::AudioBlock *buffer, float **dest);
  void prepareOutputData(vstjs::AudioBlock *buffer, float **dest);

  bool deviceIsOpen;
  bool deviceIsPlaying;
};

class IPCAudioIODeviceType : public AudioIODeviceType {
public:
  IPCAudioIODeviceType(const String _socketAddress)
      : socketAddress(_socketAddress), AudioIODeviceType("IPC") {
    deviceNames = new StringArray();
    deviceNames->add("default");
  }
  void scanForDevices() override { /* todo: implement stub */
  }
  StringArray getDeviceNames(bool wantInputNames = false) const override {
    return *deviceNames;
  }
  int getDefaultDeviceIndex(bool forInput) const override { return 0; }
  int getIndexOfDevice(AudioIODevice *device, bool asInput) const override {
    return 0;
  }
  bool hasSeparateInputsAndOutputs() const override { return false; }
  AudioIODevice *createDevice(const String &outputDeviceName,
                              const String &inputDeviceName) override {
    return new IPCAudioIODevice(deviceNames->getReference(0), socketAddress);
  }

private:
  ScopedPointer<StringArray> deviceNames;
  const String socketAddress;
};

#endif // VST_JS_HOST_IPCAUDIOIODEVICE_H
