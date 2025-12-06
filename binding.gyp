{
  "targets": [
    {
      "target_name": "keys_generator",
      "sources": [
        "src/napi_wrapper.cpp",
        "src/platform_utils.cpp",
        "src/keyring.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "src/",
        "<(node_root_dir)/include/node"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
      "conditions": [
        [
          "OS=='win'",
          {
            "defines": ["WINDOWS_PLATFORM"],
            "libraries": [
              "-ladvapi32",
              "-lcrypt32"
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "ExceptionHandling": 1
              }
            }
          }
        ],
        [
          "OS=='linux'",
          {
            "defines": ["LINUX_PLATFORM"],
            "cflags": [
              "<!@(pkg-config --cflags libsecret-1 || echo '')"
            ],
            "libraries": [
              "<!@(pkg-config --libs libsecret-1 || echo '')"
            ],
            "conditions": [
              [
                "<!(pkg-config --exists libsecret-1 >/dev/null 2>&1 && echo 1 || echo 0)==1",
                {
                  "defines": ["HAVE_LIBSECRET"]
                }
              ]
            ]
          }
        ],
        [
          "OS=='mac'",
          {
            "defines": ["MACOS_PLATFORM"],
            "libraries": [
              "-framework Security",
              "-framework CoreFoundation"
            ],
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "CLANG_CXX_LIBRARY": "libc++",
              "MACOSX_DEPLOYMENT_TARGET": "10.15"
            }
          }
        ]
      ]
    }
  ]
}
