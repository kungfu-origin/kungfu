// import { blankValidator, tcpUriValidator, o99Validator, intValidator } from '__assets/validator'
import * as VALIDATOR from '__assets/validator'
import { platform } from '__gConfig/platformConfig';
import { getExtensionConfigs } from '__gUtils/busiUtils';
import { sourceTypeConfig } from '__gConfig/tradingConfig';

export const getAccountSource = async (): Promise<Sources> => {
    let sources: Sources = {}
    try {
        const configs = await getExtensionConfigs();
        configs.forEach((c: any): void => {
            if(c.type === 'source') {
                const config: AccountSettingOrigin = c.config;
                const source: string = config.name;
                const typeName: string = config.type;
                const type: string = (sourceTypeConfig[typeName] || {}).color || '';
                const itemConfig: AccountSettingItem[] = config.config;
                itemConfig.forEach((item: AccountSettingItem): void => {
                    if(!item['validator']) return;
                    const validators: any = VALIDATOR;
                    item['validator'] = (item['validator'] || []).map((validatorName: string): any => validators[validatorName])
                })
                const accountSetting: AccountSetting = {
                    ...config,
                    source,
                    type,
                    typeName,
                    key: config.key,
                    config: itemConfig
                }
                sources[source] = accountSetting
            }
        })
    } catch (err) {
        sources = {}
    }
    return sources
}
