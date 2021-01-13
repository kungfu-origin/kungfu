{
  "targets": [
    {
      "target_name": "<(module_name)",
      "rules": [
        {
          "rule_name": "cmake",
          "extension": "CMakeLists.txt",
          "action": ["yarn", "configure"]
        },
        {
          "rule_name": "header",
          "extension": "h",
          "action": ["yarn", "compile"]
        },
        {
          "rule_name": "cpp",
          "extension": "cpp",
          "action": ["yarn", "compile"]
        }
      ],
      "actions": [
        {
          "action_name": "configure",
          "inputs": ["CMakeLists.txt"],
          "outputs": ["build/CMakeCache.txt"],
          "action": ["yarn", "configure"]
        },
        {
          "action_name": "compile",
          "inputs": ["build/Makefile"],
          "outputs": ["<(PRODUCT_DIR)"],
          "action": ["yarn", "compile"]
        },
        {
          "action_name": "freeze",
          "inputs": ["<(PRODUCT_DIR)/*"],
          "outputs": ["<(module_path)"],
          "action": ["yarn", "freeze"]
        }
      ]
    }
  ]
}