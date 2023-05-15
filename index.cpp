#define LM_NOEXCEPT

#include <napi.h>
#include <iostream>
#include <sstream>
#include<string>
using namespace std;
 
int countWords(string str)
{
    // Breaking input into word
    // using string stream
   
    // Used for breaking words
    stringstream s(str);
   
    // To store individual words
    string word;
 
    int count = 0;
    while (s >> word)
        count++;
    return count;
}
 
// Driver code
int main()
{
    string s = "geeks for geeks geeks "
               "contribution placements";
    cout << " Number of words are: " << countWords(s);
    return 0;
}
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
            std::string prompt;
            if(info[0].IsString()) 
            {
                prompt = info[0].As<Napi::String>().Utf8Value();
            }
            inference_->append(prompt);
        }
//std::string_view end, const std::function<bool (const char *)> &on_tick = nullptr
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
        void create_savestate(const Napi::CallbackInfo& info)
        {
            
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

LM::Inference *LM::Inference::construct(const std::string &weights_path, const Params &p) {
    // Create model
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
