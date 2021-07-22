{
    "targets": [
        {
            "target_name": "pipenv",
            "type": "none",
            "actions": [
                {
                    "action_name": "install",
                    "inputs": ["<(module_root_dir)/Pipfile"],
                    "outputs": ["<(module_root_dir)/Pipfile.lock"],
                    "action": ["python", ".build/yarn-run.py", "pipenv", "--bare", "install"]
                }
            ]
        },
        {
            "target_name": "poetry",
            "type": "none",
            "dependencies": ["pipenv"],
            "actions": [
                {
                    "action_name": "install",
                    "inputs": [
                        "<(module_root_dir)/Pipfile",
                        "<(module_root_dir)/pyproject.toml"
                    ],
                    "outputs": ["<(module_root_dir)/poetry.lock"],
                    "action": ["python", ".build/yarn-run.py", "poetry", "install", "-n", "-q"]
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
                    "inputs": [
                        "<(module_root_dir)/Pipfile",
                        "<(module_root_dir)/pyproject.toml",
                        "<(module_root_dir)/conanfile.py"
                    ],
                    "outputs": ["<(module_root_dir)/build/conan.lock"],
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
                        "<(module_root_dir)/conanfile.py",
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
            "dependencies": ["poetry", "<(module_name)"],
            "actions": [
                {
                    "action_name": "bundle",
                    "inputs": [
                        "<(module_root_dir)/Pipfile",
                        "<(module_root_dir)/pyproject.toml",
                        "<(module_root_dir)/conanfile.py",
                        "<!@(node -p \"require('glob').sync('**/CMakeLists.txt', {ignore:'build/**'}).join(' ');\")",
                        "<!@(node -p \"require('glob').sync('**/*.*(h|hpp|c|cc|cpp)', {ignore:'build/**'}).join(' ');\")",
                        "<!@(node -p \"require('glob').sync('python/**/*.*(py|spec)').join(' ');\")"
                    ],
                    "outputs": ["<(module_path)/kungfubuildinfo.json"],
                    "action": ["python", ".build/yarn-run.py", "bundle"]
                }
            ]
        }
    ]
}
