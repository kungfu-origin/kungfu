const path = require('path');
const ESLintPlugin = require('eslint-webpack-plugin');
const cwdDir = process.cwd();
const { isProduction } = require('./utils');

module.exports = {
    makeConfig: (argv) => {
        const production = isProduction(argv.mode);
        return {
            devtool: 'eval-cheap-source-map',
            externals: ['pm2', 'iconv-lite'],
            mode: production ? 'production' : 'development',
            module: {
                rules: [
                    {
                        test: /\.(t|j)s$/,
                        exclude: /node_modules/,
                        use: [
                            {
                                loader: 'babel-loader',
                            },
                        ],
                    },
                    {
                        test: /\.(t|j)s$/,
                        exclude: /node_modules/,
                        use: [
                            {
                                loader: 'ts-loader',
                                options: {
                                    // 指定特定的ts编译配置，为了区分脚本的ts配置
                                    configFile: path.resolve(
                                        cwdDir,
                                        'tsconfig.json',
                                    ),
                                    // 对应文件添加个.ts或.tsx后缀
                                    appendTsSuffixTo: [/\.vue$/],
                                    // transpileOnly: true // 关闭类型检测，即值进行转译
                                },
                            },
                        ],
                    },
                    {
                        test: /\.node$/,
                        use: 'node-loader',
                    },
                    {
                        test: /\.(m?js|node)$/,
                        parser: { amd: false },
                        use: {
                            loader: '@vercel/webpack-asset-relocator-loader',
                            options: {
                                outputAssetBase: 'native_modules',
                                production: production,
                            },
                        },
                    },
                    {
                        test: /\.(png|jpe?g|gif|svg)(\?.*)?$/,
                        use: {
                            loader: 'url-loader',
                            options: {
                                limit: 10000,
                                name: 'imgs/[name]--[folder].[ext]',
                                esModule: false,
                            },
                        },
                    },
                    {
                        test: /\.(mp4|webm|ogg|mp3|wav|flac|aac)(\?.*)?$/,
                        use: {
                            loader: 'url-loder',
                            options: {
                                limit: 10000,
                                name: 'media/[name]--[folder].[ext]',
                                esModule: false,
                            },
                        },
                    },
                    {
                        test: /\.(woff2?|eot|ttf|otf)(\?.*)?$/,
                        use: {
                            loader: 'file-loader',
                            options: {
                                limit: 10000,
                                name: 'fonts/[name]--[folder].[ext]',
                                esModule: false,
                            },
                        },
                    },
                ],
            },
            node: {
                __dirname: !production,
                __filename: !production,
            },
            output: {
                globalObject: 'this',
                filename: '[name].js',
                libraryTarget: 'commonjs2',
                path: path.join(argv.distDir, argv.distName),
            },
            plugins: [
                new ESLintPlugin({
                    fix: true /* 自动帮助修复 */,
                    extensions: ['js', 'json', 'ts', 'json', 'css', 'less'],
                    exclude: 'node_modules',
                    failOnWarning: production ? false : true,
                }),
            ],
            resolve: {
                alias: {
                    '@kungfu-trader/kungfu-core': path.dirname(
                        path.resolve(
                            require.resolve('@kungfu-trader/kungfu-core'),
                        ),
                    ),
                    '@kungfu-trader/kungfu-js-api': path.join(
                        path.dirname(
                            path.dirname(
                                require.resolve('@kungfu-trader/kungfu-js-api'),
                            ),
                        ),
                        'src',
                    ),
                },

                extensions: [
                    '.js',
                    '.ts',
                    '.d.ts',
                    '.vue',
                    '.json',
                    '.css',
                    '.node',
                ],
            },
        };
    },
};
