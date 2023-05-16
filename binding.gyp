{
  "targets": [
    {
      "target_name": "ts4all",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [
         "./libjustlm/llama.cpp/ggml.c",
        "./libjustlm/llama.cpp/llama.cpp",
        './libjustlm/g4a-common.cpp',
        './libjustlm/gptj/gptj.cpp',
        "index.cpp",
       ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "./libjustlm/include",
        "./libjustlm/llama.cpp",
        "./libjustlm",
      ],
      "conditions": [
        ['OS=="mac"', {
            'defines': [
                'LM_NOEXCEPT',
                'NAPI_DISABLE_CPP_EXCEPTIONS'
            ],
        }],
        ['OS=="win"', {
            'defines': [
                'LM_NOEXCEPT',
                'NAPI_DISABLE_CPP_EXCEPTIONS',
                "__AVX2__"
            ],
            "msvs_settings": {
                "VCCLCompilerTool": { "AdditionalOptions": ["/std:c++20"], },
            },
        }, {

        }]
      ],
    },

  ]
}
