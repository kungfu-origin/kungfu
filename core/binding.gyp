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
                    "action": ["python", ".build-tools/node-gyp.py", "pipenv", "install"]
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
                    "inputs": ["<(module_root_dir)/CMakeLists.txt"],
                    "outputs": ["<(module_root_dir)/build/conanbuildinfo.cmake"],
                    "action": ["python", ".build-tools/node-gyp.py", "configure"]
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
                    "variables": {
                        "sources": [
                            "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./cpp').join(' ');\")"
                        ]
                    },
                    "inputs": ["<@(sources)"],
                    "outputs": ["<(PRODUCT_DIR)/kungfubuildinfo.json"],
                    "action": ["python", ".build-tools/node-gyp.py", "compile"]
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
                    "inputs": ["<(PRODUCT_DIR)/kungfubuildinfo.json"],
                    "outputs": ["<(module_path)/kungfubuildinfo.json"],
                    "action": ["python", ".build-tools/node-gyp.py", "freeze"]
                }
            ]
        }
    ]
}
