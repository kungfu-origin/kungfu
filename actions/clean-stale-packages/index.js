const core = require('@actions/core');
const {getPackage} = require("./query.js");
try {
    const inputRepo = core.getInput('repo');
    const inputPackage = core.getInput('package');
    getPackage(argv.token, argv.owner, inputRepo, inputPackage);
    core.setOutput("matrix", outputString);
} catch (error) {
    core.setFailed(error.message);
}