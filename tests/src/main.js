
const  {kfDebugger} = require('__gUtils/kungfuDebugger')
const fs = require('fs');
const path = require('path');

console.log(process.env)

function walk(dir, done) {
  var results = [];
  fs.readdir(dir, function(err, list) {
    if (err) return done(err);
    var i = 0;
    (function next() {
      var file = list[i++];
      if (!file) return done(null, results);
      file = path.resolve(dir, file);
      fs.stat(file, function(err, stat) {
        if (stat && stat.isDirectory()) {
          walk(file, function(err, res) {
            results = results.concat(res);
            next();
          });
        } else {
          results.push(file);
          next();
        }
      });
    })();
  });
};

walk('/Users/luqing/Documents/code/kungfu/tests/cases',  function(err, results) {
  if (err) throw err;
  results.map(filePath => {
    console.log(`execute script: ${filePath}`)
    kfDebugger.execLocalFile(filePath)})
});
