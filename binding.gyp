{
  "targets": [
    {
      "target_name": "fib",
      "msvs_settings": {
        "VCCLCompilerTool": {
          "TreatWChar_tAsBuiltInType": "false",
        },
      },
      "sources": [ "fib.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}