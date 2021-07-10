{
    "targets": [
        {
            "target_name": "pipenv",
            "type": "none",
            "actions": [
                {
                    "action_name": "pipenv",
                    "inputs": ["<(module_root_dir)/Pipfile"],
                    "outputs": ["<(module_root_dir)/Pipfile.lock"],
                    "action": ["python", ".build/yarn-run.py", "pipenv-js", "install"]
                }
            ]
        },
        {
            "target_name": "conan",
            "type": "none",
            "dependencies": ["pipenv"],
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
