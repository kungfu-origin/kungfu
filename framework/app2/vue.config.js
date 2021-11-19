const { getThemeVariables } = require('ant-design-vue/dist/theme');
const path = require("path");

module.exports = {
    css: {
        modules: false,
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

    configureWebpack: (config) => {
        config.resolve.alias['@'] = path.resolve(__dirname, 'src');
        config.resolve.extensions = ['.js', '.ts', '.vue', '.json', '.css', '.less', '.node']

    }
}