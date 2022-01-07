'use strict';

const toolkit = require('@kungfu-trader/kungfu-js-api/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');
const {
    getWebpackExternals,
    getAppDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const appDir = getAppDir();
const { getKungfuBuildInfo, isProduction } = toolkit.utils;
const { gitCommitVersion, pyVersion, buildTimeStamp } = getKungfuBuildInfo();

const webpackConfig = (argv) =>
    merge(toolkit.webpack.makeConfig(argv), {
        externals: getWebpackExternals(),

        entry: {
            main: path.join(appDir, 'src', 'main', 'index.ts'),
        },

        resolve: {
            alias: {
                '@root': appDir,
                '@main': path.resolve(appDir, 'src', 'main'),
            },
        },
        target: 'electron-main',
    });

const prodConfig = {
    plugins: [
        new webpack.DefinePlugin({
            __git_commit_version: `"${gitCommitVersion.toString()}"`,
            __python_version: `"${pyVersion.toString()}"`,
            __build_timestamp: `"${buildTimeStamp.toString()}"`,
            'process.env.APP_TYPE': '"main"',
            __resources: '',
        }),
    ],
};

const devConfig = {
    plugins: [
        new webpack.DefinePlugin({
            __git_commit_version: `"${gitCommitVersion.toString()}"`,
            __python_version: `"${pyVersion.toString()}"`,
            __build_timestamp: `"${buildTimeStamp.toString()}"`,
            'process.env.APP_TYPE': '"main"',
            __resources: `"${path
                .join(appDir, 'public')
                .replace(/\\/g, '\\\\')}"`,
        }),
    ],
};

module.exports = (argv) => {
    return merge(
        webpackConfig(argv),
        isProduction(argv) ? prodConfig : devConfig,
    );
};
