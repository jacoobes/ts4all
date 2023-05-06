#include <napi.h>
#include "libjustlm/include/justlm.hpp"
#include "libjustlm/include/justlm_pool.hpp"

struct ParamsWrap {
  int seed = 0;
  unsigned n_threads = 0;
  unsigned n_ctx = 2012;
  unsigned n_ctx_window_top_bar = 0;
  unsigned n_batch = 8;
  unsigned n_repeat_last = 0;
  float scroll_keep = 0.0f;
  unsigned top_k = 40;
  float top_p = 0.9f;
  float temp = 0.72f;
  float repeat_penalty = 1.0f;
  unsigned eos_ignores = 0;
  bool use_mlock = true;
};

Napi::Object CreateParamsObject(const Napi::Env& env, const ParamsWrap& params) {
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("seed", params.seed);
  obj.Set("n_threads", params.n_threads);
  obj.Set("n_ctx", params.n_ctx);
  obj.Set("n_ctx_window_top_bar", params.n_ctx_window_top_bar);
  obj.Set("n_batch", params.n_batch);
  obj.Set("n_repeat_last", params.n_repeat_last);
  obj.Set("scroll_keep", params.scroll_keep);
  obj.Set("top_k", params.top_k);
  obj.Set("top_p", params.top_p);
  obj.Set("temp", params.temp);
  obj.Set("repeat_penalty", params.repeat_penalty);
  obj.Set("eos_ignores", params.eos_ignores);
  obj.Set("use_mlock", params.use_mlock);
  return obj;
}


Napi::Object ParamsWrap::Init(Napi::Env env, Napi::Object exports) {
    // This method is used to hook the accessor and method callbacks
    Napi::Function createParams = Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        ParamsWrap params;
        if (info.Length() > 0) {
           params = GetParamsFromObject(info[0].As<Napi::Object>());
        }
        Napi::Object paramsObj = CreateParamsObject(env, params);
        info.GetReturnValue().Set(paramsObj);
    });
    exports.Set("createParams", createParams);

    return exports;
}



class Inference : public Napi::ObjectWrap<Inference> {
   public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);   
    Inference(const Napi::CallbackInfo& info);
    static Napi::Value CreateNewItem(const Napi::CallbackInfo& info);
}



Napi::Object Inference::Init(Napi::Env env, Napi::Object exports) {
    // This method is used to hook the accessor and method callbacks
    Napi::Function func = DefineClass(env, "Inference", {
        //InstanceMethod<&Example::GetValue>("GetValue", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        //InstanceMethod<&Example::SetValue>("SetValue", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        StaticMethod<&Example::CreateNewItem>("CreateNewItem", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();

    // Create a persistent reference to the class constructor. This will allow
    // a function called on a class prototype and a function
    // called on instance of a class to be distinguished from each other.
    *constructor = Napi::Persistent(func);
    exports.Set("Inference", func);

    // Store the constructor as the add-on instance data. This will allow this
    // add-on to support multiple instances of itself running on multiple worker
    // threads, as well as multiple instances of itself running in different
    // contexts on the same thread.
    //
    // By default, the value set on the environment here will be destroyed when
    // the add-on is unloaded using the `delete` operator, but it is also
    // possible to supply a custom deleter.
    env.SetInstanceData<Napi::FunctionReference>(constructor);

    return exports;
}


Napi::Object Init (Napi::Env env, Napi::Object exports) {
    Inference::Init(env, exports);
    ParamsWrap::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
