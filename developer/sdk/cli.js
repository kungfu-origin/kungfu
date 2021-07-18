#!/usr/bin/env node

const fse = require("fs-extra");
const path = require("path");
const os = require("os");
const treeify = require('treeify');
const sdk = require("./index.js");

const defaultLibSiteURL = "http://ex.libkungfu.cc";

require("yargs/yargs")(process.argv.slice(2))
    .command("list", "List libs", (yargs) => {
        yargs.option("url", {
            type: "string",
            default: defaultLibSiteURL,
            describe: "lib site URL"
        }).option("match-name", {
            type: "string",
            default: ".*",
            describe: "re expression to filter lib name"
        }).option("match-version", {
            type: "string",
            default: ".*",
            describe: "re expression to filter lib version"
        }).option("list-version", {
            type: "boolean",
            default: true,
            describe: "show lib versions"
        }).option("list-platform", {
            type: "boolean",
            default: false,
            describe: "show platform implementaions"
        });
    }, async (argv) => {
        const libs = await sdk.list(argv.url, argv.matchName, argv.matchVersion, argv.listVersion, argv.listPlatform);
        console.log(treeify.asTree(libs, true));
    })
    .command("install", "Download and install lib", (yargs) => {
        yargs.option("url", {
            type: "string",
            default: defaultLibSiteURL,
            describe: "lib site URL"
        }).option("lib-name", {
            type: "string",
            describe: "lib name"
        }).option("lib-version", {
            type: "string",
            describe: "lib version"
        }).option("arch", {
            type: "string",
            describe: "lib arch",
            default: os.arch(),
            choices: ["x86", "x64"]
        }).option("os", {
            type: "string",
            describe: "os platform",
            choices: ["macos", "linux", "windows"],
            default: detectPlatform()
        });
    }, (argv) => {
        if (argv.libName) {
            sdk.install(argv.url, argv.libName, argv.libVersion, argv.os, argv.arch);
        } else {
            sdk.installBatch(argv.url, argv.os, argv.arch);
        }
    })
    .command("configure", "Generate configurations", (yargs) => {
    }, (argv) => {
        sdk.configure();
    })
    .command("build", "Build via cmake-js", (yargs) => {
    }, (argv) => {
        sdk.build();
    })
    .command("rebuild", "Clean and build", (yargs) => {
    }, (argv) => {
        sdk.clean();
        sdk.configure();
        sdk.build();
    })
    .command("dist", "Make distribution", (yargs) => {
    }, (argv) => {
        sdk.dist();
    })
    .command("clean", "Clean build related files", (yargs) => {
        yargs.option("keep-libs", {
            type: "boolean",
            default: true,
            describe: "Keep downloaded lib files"
        });
    }, (argv) => {
        sdk.clean(argv.keepLibs);
    })
    .demandCommand()
    .help()
    .argv;