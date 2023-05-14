{
  "targets": [
    {
      "target_name": "ts4all",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [
         "./libjustlm/llama.cpp/ggml.c",
        "./libjustlm/llama.cpp/llama.cpp",
        "./libjustlm/gptj/gptj.cpp",
        "index.cpp",
       ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "./libjustlm/include",
        "./libjustlm/gptj",
        "./libjustlm/llama.cpp",
        "./libjustlm",
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}
