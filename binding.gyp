{
  "targets": [
    {
      "target_name": "ts4all",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [
        "index.cpp",
             ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "./libjustlm/include",
        "./libjustlm/llama.cpp",
        "./libjustlm"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}
