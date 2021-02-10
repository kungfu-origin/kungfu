const argv = require("yargs/yargs")(process.argv.slice(2))
    .option("token", {description: "github access token"})
    .option("repo", {description: "repository name"})
    .option("package", {description: "package name"})
    .option("owner", {description: "owner name", default: "kungfu-trader"})
    .demandOption(["token", "repo", "package"])
    .help()
    .argv;

const {getPackage} = require("./query.js");

getPackage(argv.token, argv.owner, argv.repo, argv.package);