const core = require('@actions/core');
const github = require("@actions/github");
const {cleanStalePackages} = require("./clean.js");

try {
    const context = github.context;
    const token = core.getInput('token');
    const actionPath = core.getInput('action-path');
    cleanStalePackages(context.repo.owner, token, context.repo.repo, actionPath);
} catch (error) {
    core.setFailed(error.message);
}