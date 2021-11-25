const { getThemeVariables } = require('ant-design-vue/dist/theme');
const path = require("path");

module.exports = {
    css: {
        loaderOptions: {
            less: {  
                lessOptions: {
                    modifyVars: {
                        ...getThemeVariables({
                            dark: true, // Enable dark mode
                        }),
                        'primary-color': '#FAAD14',
                    },    
                    javascriptEnabled: true,
                }
            }
        }
    },

    devServer: {
        port: '9090'
    },

    pages: {
        index: {
            entry: 'src/renderer/index/main.ts',
            template: 'public/index.html',
            filename: 'index.html',
            title: '功夫交易系统'
        }
    },

    configureWebpack: (config) => {

        config.resolve.alias['@renderer'] = path.resolve(__dirname, 'src/renderer');
        config.resolve.extensions = ['.js', '.ts', '.vue', '.json', '.css', '.less', '.node']
        config.target = "electron-renderer"
    }
}