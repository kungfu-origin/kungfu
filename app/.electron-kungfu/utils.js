const fs = require("fs");
const path = require('path');
const getCommitVersion = () => {
  var gitCommitVersion = 'latest'
  try {
    var gitHEAD = fs.readFileSync(path.join(__dirname, '..', '..', '.git', 'HEAD'), 'utf-8').trim() // ref: refs/heads/develop
    if((gitHEAD || '').split(': ').length <= 1) {
      gitCommitVersion = gitHEAD
    } else {
      var ref = gitHEAD.split(': ')[1] // refs/heads/develop
      var develop = gitHEAD.split('/')[2] // 环境：develop
      var gitVersion = fs.readFileSync(path.join(__dirname, '..', '..', '.git', ref), 'utf-8').trim() // git版本号，例如：6ceb0ab5059d01fd444cf4e78467cc2dd1184a66
      gitCommitVersion = develop + ': ' + gitVersion
    }
  } catch (err) {
    console.error(err)
  }
  return gitCommitVersion.toString();
}

exports.getCommitVersion = getCommitVersion;