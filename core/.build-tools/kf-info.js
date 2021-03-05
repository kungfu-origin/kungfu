#!/usr/bin/env node

const fs = require("fs");
const path = require("path");

require("yargs/yargs")(process.argv.slice(2))
    .command("dir", "Show kungfu core base directory", (yargs) => {
    }, (argv) => {
        console.log(fs.realpathSync(path.dirname(__dirname)));
    })
    .command("info", "SHow kungfu core build info", (yargs) => {
    }, (argv) => {
        const buildinfo = require(path.join(path.dirname(__dirname), "build", "kfc", "kungfubuildinfo.json"));
        console.log(buildinfo);
    })
    .demandCommand()
    .help()
    .argv;