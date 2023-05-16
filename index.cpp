#include <napi.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "g4a-common.hpp"
#include "justlm.hpp"
#include "justlm_pool.hpp"
#include "justlm_llama.hpp"
#include "justlm_gptj.hpp"
#include "gptj/gptj.hpp"

class InferenceWrapper : public Napi::ObjectWrap<InferenceWrapper> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);

        static 
        InferenceWrapper *construct(const std::string& weights_path, const LM::Inference::Params& p);

        InferenceWrapper(const Napi::CallbackInfo& info);
        ~InferenceWrapper(){};
        void append(const Napi::CallbackInfo& info)
        {
            std::string prompt;
            if(info[0].IsString()) 
            {
                prompt = info[0].As<Napi::String>().Utf8Value();
            }
            inference_->append(prompt);
        }
        Napi::Value run(const Napi::CallbackInfo& info)
        {
            auto env = info.Env();
            std::string_view eend;
            if(info[0].IsString()) {
                eend = std::string_view(info[0].As<Napi::String>().Utf8Value());
            } else {
                Napi::Error::New(env, "invalid string argument").ThrowAsJavaScriptException();
                return Napi::String::New(env, "Error");
            }
            Napi::Value callback = Napi::Value();
            std::function<bool(const char*)> mycb = nullptr;
            if(info[1].IsFunction()) {
                callback = info[1].As<Napi::Function>();
                mycb = [=](const char* generated) {
                    Napi::HandleScope scope(env);
                    Napi::Boolean result = Napi::Boolean::New(env, callback.As<Napi::Function>().Call({ Napi::String::New(env, generated) }).As<Napi::Boolean>());
                    return result.Value();
                };
            }
            return Napi::String::New(env, inference_->run(eend, mycb));
        }
        bool create_savestate(const Napi::CallbackInfo& info)
        {
            auto env = info.Env();
            
            if(info[0].IsObject())
            {
                auto obj = info[0].As<Napi::Object>();

                LM::Inference::Savestate& save_state;
                auto js_buf = obj.Get("buf").As<Napi::Buffer<uint8_t>>();
                uint8_t* inputData = js_buf.Data();
                size_t inputLength = js_buf.Length();

                std::vector<uint8_t> cpp_buf(inputData, inputData + inputLength);
                save_state.buf = cpp_buf;
                
                auto js_tokens = obj.Get("tokens").As<Napi::Array>();
                uint32_t length = js_tokens.Length();
                std::vector<int> outputVector;
                outputVector.reserve(length);

                for (uint32_t i = 0; i < length; i++) 
                {
                  Napi::Value element = inputArray.Get(i);
                  if (!element.IsNumber()) 
                  {
                    Napi::TypeError::New(env, "Array elements must be numbers").ThrowAsJavaScriptException();
                    return;
                  }
                  int convertedValue = element.As<Napi::Number>().Int32Value();
                  outputVector.push_back(convertedValue);
                }
                save_state.tokens = outputVector;
                save_state.prompt = obj.Get("prompt").As<Napi::String>().Utf8Value();
                
                return inference_->create_savestate(save_state);
            } 
            else {
                Napi::TypeError::New(env, "Recieved a non Savestate for create_savestate").ThrowAsJavaScriptException();
                return false;
            }
           
        }
        void restore_savestate(const Napi::CallbackInfo& info)
        {
            
        }
        void serialize(const Napi::CallbackInfo& info)
        {
            if(info[0].IsString()) {
                std::stringstream   s(info[0].As<Napi::String>().Utf8Value()); 
                inference_->serialize(s);
            } 
        }
        void deserialize(const Napi::CallbackInfo& info)
        {
          //todo 
        } 
        Napi::Value get_prompt(const Napi::CallbackInfo& info)
        {
            auto prompt = inference_->get_prompt();
            return Napi::String::New(info.Env(), prompt);
        }
        Napi::Value get_context_size(const Napi::CallbackInfo& info)
        {
            return Napi::Number::New(info.Env(), inference_->get_context_size());
        }


    private:
        std::unique_ptr<LM::Inference> inference_;
};

class LMPoolWrapper : public Napi::ObjectWrap<LMPoolWrapper> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);

        LMPoolWrapper(const Napi::CallbackInfo& info);
        ~LMPoolWrapper(){};
    private:
        std::unique_ptr<LM::InferencePool> pool_;

};

//copied from justlm.cpp because i  dont know how to use that construct in my bidnings
LM::Inference *LM::Inference::construct(const std::string &weights_path, const Params &p) {
    // Read magic
    std::ifstream f(weights_path, std::ios::binary);
    uint32_t magic;
    f.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    // Create model
    if (magic == 0x67676d6c) {
        f.seekg(0);
        return new GPTJInference(weights_path, f, p);
    } else {
        f.close();
        return new LLaMaInference(weights_path, p);
    }
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
          //i think it implicility casts uint_32 -> signed?
            //https://stackoverflow.com/questions/46228676/how-to-correctly-cast-uint32-t-to-unsigned

        if(param_obj.Has("n_threads")) {
            params.n_threads = param_obj.Get("n_threads").As<Napi::Number>().Uint32Value();
        }
        if(param_obj.Has("n_ctx")) {
            params.n_ctx = param_obj.Get("n_ctx").As<Napi::Number>().Uint32Value();
        }
        if(param_obj.Has("n_ctx_window_top_bar")) {
            params.n_ctx_window_top_bar = param_obj.Get("n_ctx_window_top_bar").As<Napi::Number>().Uint32Value();
        }
        if(param_obj.Has("n_batch")) {
            params.n_batch = param_obj.Get("n_batch").As<Napi::Number>().Uint32Value();
        }
        if(param_obj.Has("n_repeat_last")) {
            params.n_repeat_last = param_obj.Get("n_repeat_last").As<Napi::Number>().Uint32Value();
        }
        if(param_obj.Has("scroll_keep")) {
            params.scroll_keep = param_obj.Get("scroll_keep").As<Napi::Number>().FloatValue();
        }
        if(param_obj.Has("top_k")) {
            
            params.top_k = param_obj.Get("top_k").As<Napi::Number>().Uint32Value();
        }
        if(param_obj.Has("top_p")) {

            params.top_p = param_obj.Get("top_p").As<Napi::Number>().FloatValue();
        }
        if(param_obj.Has("temp")) {
            params.temp = param_obj.Get("temp").As<Napi::Number>().FloatValue();
        }
        if(param_obj.Has("repeat_penalty")) {
            params.repeat_penalty = param_obj.Get("repeat_penalty").As<Napi::Number>().FloatValue();
        }
        if(param_obj.Has("eos_ignores")) {
            params.eos_ignores = param_obj.Get("eos_ignores").As<Napi::Number>().Uint32Value();
        }
        if(param_obj.Has("use_mlock")) {
            params.use_mlock = param_obj.Get("use_mlock").As<Napi::Boolean>().Value();
        }

    }

   // construct the inference object
   inference_ = std::unique_ptr<LM::Inference>(LM::Inference::construct(weights_path, params));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return InferenceWrapper::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);
