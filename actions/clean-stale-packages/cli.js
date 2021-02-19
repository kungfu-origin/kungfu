const argv = require("yargs/yargs")(process.argv.slice(2))
    .option("owner", {description: "owner name", default: "kungfu-trader"})
    .option("token", {description: "github access token"})
    .option("repo", {description: "repository name"})
    .option("action-path", {description: "action path"})
    .demandOption(["token", "repo", "action-path"])
    .help()
    .argv;

const {getPackage} = require("./query.js");

getPackage(argv.owner, argv.token, argv.repo, argv.actionPath);