{
  'targets': [
    {
      'target_name': '<(module_name)',
      'actions': [
        {
          'action_name': 'pipenv',
          'inputs': ['Pipfile'],
          'outputs': ['Pipfile.lock'],
          'action': ['python', 'python/gyp-yarn.py', 'pipenv-install']
        },
        {
          'action_name': 'configure',
          'inputs': ['CMakeLists.txt'],
          'outputs': ['build/conanbuildinfo.cmake'],
          'action': ['python', 'python/gyp-yarn.py', 'configure']
        },
        {
          'action_name': 'compile',
          'variables': {
            'sources': ["<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./cpp').join(' ');\")"]
          },
          'inputs': ['<@(sources)'],
          'outputs': ['<(PRODUCT_DIR)/kungfubuildinfo.json'],
          'action': ['python', 'python/gyp-yarn.py', 'compile']
        },
        {
          'action_name': 'freeze',
          'inputs': ['<(PRODUCT_DIR)/kungfubuildinfo.json'],
          'outputs': ['<(module_path)/kungfubuildinfo.json'],
          'action': ['python', 'python/gyp-yarn.py', 'freeze']
        }
      ]
    }
  ]
}