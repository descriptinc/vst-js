#ifndef VSTJS_PLUGINHOSTWRAPPER_H
#define VSTJS_PLUGINHOSTWRAPPER_H

#include <napi.h>
#include "PluginHost.h"
#include "IPCAudioIOBuffer.h"

class PluginHostWrapper : public Napi::ObjectWrap<PluginHostWrapper> {
public:
  static void Init(Napi::Env env, std::string moduleDirectory);
  static Napi::Value NewInstance(const Napi::CallbackInfo& info);
  void Start(const Napi::CallbackInfo &info);
  void Stop(const Napi::CallbackInfo &info);
  void ProcessAudioBlock(const Napi::CallbackInfo &info);
  PluginHostWrapper(const Napi::CallbackInfo &info);

private:
  // PluginHostWrapper(std::string _shmemFile, std::string pluginPath);
  // static Napi::FunctionReference constructor;
  static void New(const Napi::CallbackInfo &info);
  PluginHost host;
  static IPCAudioIOBuffer tempBuffer;
  static std::string moduleDirectory;
};

#endif //VSTJS_PLUGINHOSTWRAPPER_H
