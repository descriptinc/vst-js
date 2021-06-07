#include "PluginHostWrapper.h"
#include <node_api.h>

Napi::Value CreateObject(const Napi::CallbackInfo &info) {
  return PluginHostWrapper::NewInstance(info);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  // get the path of the .node file
  const char* filename;
  node_api_get_module_file_name(env, &filename);
  std::string modulePath(filename);
  unsigned long lastSlash = modulePath.find_last_of("/");
  // cut off file://
  std::string moduleDir = modulePath.substr(7,lastSlash - 7);

  PluginHostWrapper::Init(env, moduleDir);

  exports.Set("launchPlugin", Napi::Function::New(env, CreateObject));

  return exports;
}

NODE_API_MODULE(vstjs, InitAll)
