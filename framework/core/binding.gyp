{
    "targets": [
        {
            "target_name": "pipenv",
            "type": "none",
            "actions": [
                {
                    "action_name": "lock",
                    "inputs": [
                        "<(module_root_dir)/Pipfile"
                    ],
                    "outputs": [
                        "<(module_root_dir)/Pipfile.lock"
                    ],
                    "action": ["python", ".build/yarn-run.py", "pipenv", "lock"]
                },
                {
                    "action_name": "install",
                    "inputs": [
                        "<(module_root_dir)/Pipfile.lock"
                    ],
                    "outputs": [
                        "<(module_root_dir)/build/pipenv.target.mk"
                    ],
                    "action": ["python", ".build/yarn-run.py", "pipenv", "install"]
                }
            ]
        },
        {
            "target_name": "poetry",
            "type": "none",
            "dependencies": ["pipenv"],
            "actions": [
                {
                    "action_name": "lock",
                    "inputs": [
                        "<(module_root_dir)/pyproject.toml"
                    ],
                    "outputs": [
                        "<(module_root_dir)/poetry.lock"
                    ],
                    "action": ["python", ".build/yarn-run.py", "poetry", "lock", "-n"]
                },
                {
                    "action_name": "install",
                    "inputs": ["<(module_root_dir)/poetry.lock"],
                    "outputs": [
                        "<(module_root_dir)/build/poetry.target.mk"
                    ],
                    "action": ["python", ".build/yarn-run.py", "poetry", "install", "-n"]
                }
            ]
        },
        {
            "target_name": "conan",
            "type": "none",
            "dependencies": ["poetry"],
            "actions": [
                {
                    "action_name": "configure",
                    "inputs": ["<(module_root_dir)/conanfile.py"],
                    "outputs": ["<(module_root_dir)/build/conanbuildinfo.cmake"],
                    "action": ["python", ".build/yarn-run.py", "configure"]
                }
            ]
        },
        {
            "target_name": "<(module_name)",
            "type": "none",
            "dependencies": ["conan"],
            "actions": [
                {
                    "action_name": "compile",
                    "inputs": [
                        "<!@(node -p \"require('glob').sync('**/CMakeLists.txt', {ignore:'build/**'}).join(' ');\")",
                        "<!@(node -p \"require('glob').sync('**/*.*(h|hpp|c|cc|cpp)', {ignore:'build/**'}).join(' ');\")"
                    ],
                    "outputs": ["<(PRODUCT_DIR)/kungfubuildinfo.json"],
                    "action": ["python", ".build/yarn-run.py", "compile"]
                }
            ]
        },
        {
            "target_name": "kfc",
            "type": "none",
            "dependencies": ["<(module_name)"],
            "actions": [
                {
                    "action_name": "freeze",
                    "inputs": [
                        "<(module_root_dir)/poetry.lock",
                        "<(PRODUCT_DIR)/kungfubuildinfo.json",
                        "<!@(node -p \"require('glob').sync('python').join(' ');\")"
                    ],
                    "outputs": ["<(module_path)/kungfubuildinfo.json"],
                    "action": ["python", ".build/yarn-run.py", "freeze"]
                }
            ]
        }
    ]
}
