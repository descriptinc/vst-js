#include "PluginHostWrapper.h"

using namespace std;
using namespace Napi;

IPCAudioIOBuffer PluginHostWrapper::tempBuffer(SharedMemoryBuffer::NumChannels, SharedMemoryBuffer::BufferSize);
std::string PluginHostWrapper::moduleDirectory = "";

// PluginHostWrapper::PluginHostWrapper(string _shmemFile, string _pluginPath)
//   : Napi::ObjectWrap<PluginHostWrapper>(), 
//   host(_shmemFile, _pluginPath) {
  
//   Napi::Env env = info.Env();
// }
// Napi::FunctionReference PluginHostWrapper::constructor;

Napi::Value PluginHostWrapper::NewInstance(const Napi::CallbackInfo& info) {
  // Napi::Env env = info.Env();
  // Napi::EscapableHandleScope scope(env);

  // Retrieve the instance data we stored during `Init()`. We only stored the
  // constructor there, so we retrieve it here to create a new instance of the
  // JS class the constructor represents.
  Napi::FunctionReference* constructor =
      info.Env().GetInstanceData<Napi::FunctionReference>();
  auto instance = constructor->New({ info[0] });

  // Napi::Env env = arg.Env();
  // Napi::EscapableHandleScope scope(env);
  // // const unsigned argc = 1;
  // // Napi::Value argv[argc] = {arg};
  // // Napi::Function cons = Napi::Function::New(env, constructor);
  // // Napi::Object instance = cons->NewInstance(argc, argv);
  // auto instance = constructor.Call({arg});

  // return scope.Escape(instance);
  return instance;
}

void PluginHostWrapper::Init(Napi::Env env, std::string _moduleDirectory) {
  // Napi::HandleScope scope(env);

  moduleDirectory = _moduleDirectory;

  Napi::Function func = DefineClass(env, "PluginHost", {
    InstanceMethod<&PluginHostWrapper::Start>("start"),
    InstanceMethod<&PluginHostWrapper::Stop>("stop"),
    InstanceMethod<&PluginHostWrapper::ProcessAudioBlock>("processAudioBlock"),
  });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();

  *constructor = Napi::Persistent(func);

  // Napi::DefineClass(env, "PluginHost");
  // tpl->SetClassName(Napi::New("PluginHost"));


  // Napi::SetPrototypeTemplate(
  //   tpl, "start", Napi::New<Napi::FunctionReference>(Start));

  // Napi::SetPrototypeTemplate(
  //   tpl, "stop", Napi::New<Napi::FunctionReference>(Stop));

  // Napi::SetPrototypeTemplate(
  //   tpl, "processAudioBlock",
  //   Napi::New<Napi::FunctionReference>(ProcessAudioBlock));

  // constructor.Reset(tpl);

  // Store the constructor as the add-on instance data. This will allow this
  // add-on to support multiple instances of itself running on multiple worker
  // threads, as well as multiple instances of itself running in different
  // contexts on the same thread.
  //
  // By default, the value set on the environment here will be destroyed when
  // the add-on is unloaded using the `delete` operator, but it is also
  // possible to supply a custom deleter.
  env.SetInstanceData<Napi::FunctionReference>(constructor);
}

void PluginHostWrapper::Start(const Napi::CallbackInfo &info) {
  PluginHostWrapper *obj = ObjectWrap::Unwrap(info.This().As<Napi::Object>());
  obj->host.Start(moduleDirectory);
}

void PluginHostWrapper::Stop(const Napi::CallbackInfo &info) {
  PluginHostWrapper *obj = ObjectWrap::Unwrap(info.This().As<Napi::Object>());
  obj->host.Stop();
}

PluginHostWrapper::PluginHostWrapper(const Napi::CallbackInfo &info)
  : Napi::ObjectWrap<PluginHostWrapper>(info),
    host("test123", info[0].ToString())  {
  // if (info.Length() < 1) {
  //   Napi::Error::New(info.Env(), "Wrong number of arguments. Expected 1 argument").ThrowAsJavaScriptException();
  //   return;
  // }

  // if (!info[0].IsString()) {
  //   Napi::Error::New(info.Env(), "Incorrect Type for Argument 1. Expected String").ThrowAsJavaScriptException();
  //   return;
  // }
  // Napi::String param1(info.Env(), info[0].ToString());
  // string pluginPath = param1.Utf8Value();

  // host = PluginHost("test123", pluginPath);

  // PluginHostWrapper *obj = new PluginHostWrapper("test123", pluginPath);

  // obj->Wrap(info.This());
  // return info.This();
}

void PluginHostWrapper::ProcessAudioBlock(
  const Napi::CallbackInfo &info) {
  PluginHostWrapper *obj = ObjectWrap::Unwrap(info.This().As<Napi::Object>());

  if (info.Length() < 3) {
    Napi::Error::New(info.Env(), "Wrong number of arguments. Expected 3 arguments").ThrowAsJavaScriptException();
    return;
  }

  if (!info[0].IsNumber()) {
    Napi::Error::New(info.Env(), "Incorrect Type for Argument 1. Expected Number").ThrowAsJavaScriptException();
    return;
  }
  int numChannels = info[0].As<Napi::Number>().Int32Value();

  if (!info[1].IsNumber()) {
    Napi::Error::New(info.Env(), "Incorrect Type for Argument 2. Expected Number").ThrowAsJavaScriptException();
    return;
  }
  int numSamples = info[1].As<Napi::Number>().Int32Value();

  if (!info[2].IsArray()) {
    Napi::Error::New(info.Env(), "Incorrect Type for Argument 3. Expected Array").ThrowAsJavaScriptException();
    return;
  }

  // copy to buffer
  Napi::Array a = info[2].As<Napi::Array>();
  for (int channel = 0; channel < numChannels; channel++) {
    Napi::Value element = a.Get(channel);
    if (!element.IsTypedArray()) {
      Napi::Error::New(info.Env(), "Incorrect Type for channel data. Expected TypedArray").ThrowAsJavaScriptException();
      return;
    }
    TypedArray channel_handle = element.As<TypedArray>();
    if (channel_handle.TypedArrayType() != napi_float32_array) {
      Napi::Error::New(info.Env(), "Incorrect Type for channel data. Expected Float32Array").ThrowAsJavaScriptException();
      return;
    }
    float* dest = (float *)(channel_handle.ArrayBuffer().Data());
    tempBuffer.data[channel] = dest;
  }

  obj->host.ProcessAudioBlock(numChannels, numSamples, tempBuffer.data);
}

