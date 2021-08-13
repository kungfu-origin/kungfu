{
  "variables": {
    "gyp_dir": "<(module_root_dir)/.gyp",
    "pipenv_inputs": [
      "<(module_root_dir)/Pipfile"
    ],
    "poetry_inputs": [
      "<@(pipenv_inputs)",
      "<(module_root_dir)/pyproject.toml"
    ],
    "conan_inputs": [
      "<@(poetry_inputs)",
      "<(module_root_dir)/conanfile.py"
    ],
    "module_inputs": [
      "<(module_root_dir)/conanfile.py",
      "<!@(node -p \"require('glob').sync('**/CMakeLists.txt', {ignore:'build/**'}).join(' ');\")",
      "<!@(node -p \"require('glob').sync('**/*.*(h|hpp|c|cc|cpp)', {ignore:'build/**'}).join(' ');\")"
    ],
    "wheel_inputs": [
      "<@(poetry_inputs)",
      "<@(module_inputs)",
      "<!@(node -p \"require('glob').sync('python/**/*.*(py|spec)').join(' ');\")"
    ],
    "kfc_inputs": [
      "<@(wheel_inputs)"
    ]
  },
  "targets": [
    {
      "target_name": "pipenv",
      "type": "none",
      "actions": [
        {
          "action_name": "install",
          "inputs": [
            "<@(pipenv_inputs)"
          ],
          "outputs": [
            "<(module_root_dir)/Pipfile.lock"
          ],
          "action": [
            "python",
            "<(gyp_dir)/gyp_action_pipenv.py"
          ]
        }
      ]
    },
    {
      "target_name": "poetry",
      "type": "none",
      "dependencies": [
        "pipenv"
      ],
      "actions": [
        {
          "action_name": "install",
          "inputs": [
            "<@(poetry_inputs)"
          ],
          "outputs": [
            "<(module_root_dir)/poetry.lock"
          ],
          "action": [
            "python",
            "<(gyp_dir)/gyp_action_poetry.py"
          ]
        }
      ]
    },
    {
      "target_name": "conan",
      "type": "none",
      "dependencies": [
        "poetry"
      ],
      "actions": [
        {
          "action_name": "configure",
          "inputs": [
            "<@(conan_inputs)"
          ],
          "outputs": [
            "<(module_root_dir)/build/conan.lock"
          ],
          "action": [
            "python",
            "<(gyp_dir)/gyp_action_yarn.py",
            "configure"
          ]
        }
      ]
    },
    {
      "target_name": "<(module_name)",
      "type": "none",
      "dependencies": [
        "conan"
      ],
      "actions": [
        {
          "action_name": "compile",
          "inputs": [
            "<@(module_inputs)"
          ],
          "outputs": [
            "<(PRODUCT_DIR)/kungfubuildinfo.json"
          ],
          "action": [
            "python",
            "<(gyp_dir)/gyp_action_yarn.py",
            "compile"
          ]
        }
      ]
    },
    {
      "target_name": "wheel",
      "type": "none",
      "dependencies": [
        "poetry",
        "<(module_name)"
      ],
      "actions": [
        {
          "action_name": "wheel",
          "inputs": [
            "<@(wheel_inputs)"
          ],
          "outputs": [
            "<(module_root_dir)/build/python/kungfubuildinfo.json"
          ],
          "action": [
            "python",
            "<(gyp_dir)/gyp_action_yarn.py",
            "wheel"
          ]
        }
      ]
    },
    {
      "target_name": "kfc",
      "type": "none",
      "dependencies": [
        "wheel"
      ],
      "actions": [
        {
          "action_name": "freeze",
          "inputs": [
            "<@(kfc_inputs)"
          ],
          "outputs": [
            "<(module_path)/kungfubuildinfo.json"
          ],
          "action": [
            "python",
            "<(gyp_dir)/gyp_action_yarn.py",
            "freeze"
          ]
        }
      ]
    }
  ]
}
