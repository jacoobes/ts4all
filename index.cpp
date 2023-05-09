#include <napi.h>
#include "justlm.hpp"
#include "justlm_pool.hpp"
#include "justlm_llama.hpp"


class InferenceWrapper : public Napi::ObjectWrap<InferenceWrapper> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);

        static 
        InferenceWrapper *construct(const std::string& weights_path, const LM::Inference::Params& p);

        InferenceWrapper(const Napi::CallbackInfo& info);
        ~InferenceWrapper(){};
        void append(const Napi::CallbackInfo& info)
        {
            std::cout << "Hello";                  
        }
        void run(const Napi::CallbackInfo& info)
        {

        }
        void create_savestate(const Napi::CallbackInfo& info)
        {

        }
        void restore_savestate(const Napi::CallbackInfo& info)
        {

        }
        void serialize(const Napi::CallbackInfo& info)
        {


        }
        void deserialize(const Napi::CallbackInfo& info)
        {
           
        } 
        Napi::Value get_prompt(const Napi::CallbackInfo& info)
        {
            return Napi::Boolean::New(info.Env(), true);
        }
        Napi::Value get_context_size(const Napi::CallbackInfo& info)
        {
            return Napi::Value();
        }


    private:
        std::unique_ptr<LM::Inference> inference_;
};

LM::Inference *LM::Inference::construct(
    const std::string& weights_path,
    const LM::Inference::Params& p) {
    std::cout << "WtF";
    return new LLaMaInference(weights_path, p);
}


Napi::Object InferenceWrapper::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Inference", {
        InstanceMethod("append", &InferenceWrapper::append),
        InstanceMethod("run", &InferenceWrapper::run),
        InstanceMethod("create_savestate", &InferenceWrapper::create_savestate),
        InstanceMethod("restore_savestate", &InferenceWrapper::restore_savestate),
        InstanceMethod("serialize", &InferenceWrapper::serialize),
        InstanceMethod("deserialize", &InferenceWrapper::deserialize),
        InstanceMethod("get_prompt", &InferenceWrapper::get_prompt),
        InstanceMethod("get_context_size", &InferenceWrapper::get_context_size)
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();
        
    *constructor = Napi::Persistent(func);

    constructor->SuppressDestruct();

    env.SetInstanceData(constructor);

    exports.Set("Inference", func);
    return exports;
};

InferenceWrapper::InferenceWrapper(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<InferenceWrapper>(info) {
    // parse the arguments to get the weights path and parameters
   std::string weights_path = info[0].As<Napi::String>().Utf8Value();
   LM::Inference::Params params;
    if (info[1].IsObject()) {
        auto param_obj = info[1].As<Napi::Object>();
        if (param_obj.Has("seed")) {
            params.seed = param_obj.Get("seed").As<Napi::Number>().Int32Value();
        }
       // parse other parameters...
    }

   // construct the inference object
   inference_ = std::unique_ptr<LM::Inference>(LM::Inference::construct(weights_path, params));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return InferenceWrapper::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);
