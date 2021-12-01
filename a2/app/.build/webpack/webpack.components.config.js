'use strict';

process.env.BABEL_ENV = 'components';

const toolkit = require('@kungfu-trader/kungfu-js-api/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');

const { VueLoaderPlugin } = require('vue-loader');
const { getThemeVariables } = require('ant-design-vue/dist/theme');

const rootDir = path.dirname(path.dirname(__dirname));
const { getKungfuBuildInfo, getComponentsConfig, isProduction } = toolkit.utils;
const { pyVersion } = getKungfuBuildInfo();

const webpackConfig = (argv) => {
    return merge(toolkit.webpack.makeConfig(argv), {
        devtool: 'eval',
        externals: ['vue'],
        entry: getComponentsConfig(argv),
        module: {
            rules: [
                {
                    test: /\.css$/,
                    use: ['style-loader', 'postcss-loader', 'css-loader'],
                },
                {
                    test: /\.less$/,
                    use: [
                        'style-loader',
                        'postcss-loader',
                        'css-loader',
                        {
                            loader: 'less-loader',
                            options: {
                                lessOptions: {
                                    modifyVars: {
                                        ...getThemeVariables({
                                            dark: true,
                                        }),
                                        'primary-color': '#FAAD14',
                                    },
                                    javascriptEnabled: true,
                                },
                            },
                        },
                    ],
                },
                {
                    test: /\.vue$/,
                    use: 'vue-loader',
                },
                {
                    test: /\.worker\.js$/,
                    use: {
                        loader: 'worker-loader',
                        options: { inline: true, fallback: false },
                    },
                    exclude: /node_modules/,
                },
                {
                    test: /\.html$/,
                    use: 'html-loader',
                },
            ],
        },
        plugins: [new VueLoaderPlugin()],
        resolve: {
            alias: {
                '@root': rootDir,
                '@renderer': path.resolve(rootDir, 'src', 'renderer'),
                '@components': path.resolve(rootDir, 'src', 'components'),
            },
        },
        output: {
            globalObject: 'this',
            filename: '[name].js',
            libraryTarget: 'commonjs2',
            path: path.join(argv.distDir, argv.distName, 'components'),
        },
        target: 'electron-renderer',
    });
};

const prodConfig = {
    plugins: [
        new webpack.DefinePlugin({
            __VUE_OPTIONS_API__: true,
            __VUE_PROD_DEVTOOLS__: true,
            python_version: `"${pyVersion.toString()}"`,
            'process.env.APP_TYPE': '"components"',
        }),
    ],
};

const devConfig = {
    plugins: [
        new webpack.DefinePlugin({
            __VUE_OPTIONS_API__: true,
            __VUE_PROD_DEVTOOLS__: true,
            __resources: `"${path
                .join(rootDir, 'public')
                .replace(/\\/g, '\\\\')}"`,
            python_version: `"${pyVersion.toString()}"`,
            'process.env.APP_TYPE': '"components"',
        }),
    ],
};

// module.exports = (argv) => {
//     return merge(
//         webpackConfig(argv),
//         isProduction(argv) ? prodConfig : devConfig,
//     );
// };

module.exports = () => {
    const argv = {
        mode: 'development',
        distDir: path.join(__dirname, '../../dist'),
        distName: 'app',
    };
    return merge(
        webpackConfig(argv),
        isProduction(argv) ? prodConfig : devConfig,
    );
};
