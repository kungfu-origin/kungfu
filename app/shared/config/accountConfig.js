// import { blankValidator, tcpUriValidator, o99Validator, intValidator } from '__assets/validator'
import * as VALIDATOR from '__assets/validator'
import { platform } from '__gConfig/platformConfig';
import { getExtensionConfigs } from '__gUtils/busiUtils';

const path = require('path');


export const ifSourceDisable = {
    ctp: platform === 'mac',
    xtp: false
}

const tagColor = {
    'future': 'danger', //red,
    'stock': ''
};

export const getAccountSource = async () => {
    let sources = {}
    try {
        const configs = await getExtensionConfigs();
        configs.forEach(c => {
            if(c.type === 'source') {
                const config = c.config;
                const source = config.name;
                const typeName = config.type;
                const type = tagColor[typeName] || '';
                const itemConfig = config.config;
                itemConfig.forEach(item => {
                    if(item.validator) {
                        item.validator = (item.validator || []).map(validatorName => VALIDATOR[validatorName])
                    }
                })
                sources[source] = {
                    ...config,
                    source,
                    type,
                    typeName,
                    config: itemConfig
                }
            }
        })
    } catch (err) {
        sources = {}
    }
    return sources
}


