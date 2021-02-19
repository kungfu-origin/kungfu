const core = require('@actions/core');
const fs = require('fs')
try {
    const inputFilePath = core.getInput('inputFile');
    const inputFile = fs.readFileSync(inputFilePath);
    const inputJson = JSON.parse(inputFile);
    const outputString = `{"include":${JSON.stringify(inputJson)}}`
    core.setOutput("matrix", outputString);
} catch (error) {
    core.setFailed(error.message);
}