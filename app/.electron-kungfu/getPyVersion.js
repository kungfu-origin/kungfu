const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');

childProcess.exec(`cd ${path.join(__dirname, "../../core")} && pipenv run python --version`, (err, stdout) => {
    fs.writeFileSync(path.join(__dirname, '../.pythonVersion'), stdout)
})