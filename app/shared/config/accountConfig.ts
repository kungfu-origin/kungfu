import * as VALIDATOR from '__assets/validator'
import { getExtensionConfigs, deepClone } from '__gUtils/busiUtils';
import { sourceTypeConfig } from '__gConfig/tradingConfig';

const dealValidator = (accountConfigItems: AccountSettingItem[]) => {
    return accountConfigItems.map((item: AccountSettingItem): AccountSettingItem => {
        if(!item['validator']) return item;
        const validators: any = VALIDATOR;
        item['validator'] = (item['validator'] || []).map((validatorName: string): any => validators[validatorName])
        return item
    })    
}


export const getAccountSource = async (): Promise<StringToSource> => {
    let tdSources: Sources = {};
    let mdSources: Sources = {};
    console.log(111)
    try {
        console.log(222)
        const configs = await getExtensionConfigs();
        console.log(configs, 333, '---')
        configs.forEach((c: any): void => {
            if(c.type === 'source') {
                const config: AccountSettingOrigin = c.config;
                const source: string = config.name;
                const typeName: string = config.type;
                const type: string = (sourceTypeConfig[typeName] || {}).color || '';
        
                let tdItemConfig: AccountSettingItem[] = deepClone(config.td_config || []); //不可以为 []
                let mdItemConfig: AccountSettingItem[] = deepClone(config.md_config || []); //可以为 []
                tdItemConfig = dealValidator(tdItemConfig);
                mdItemConfig = dealValidator(mdItemConfig);

                const tdAccountSetting: AccountSetting = {
                    ...config,
                    source,
                    type,
                    typeName,
                    key: config.key,
                    config: tdItemConfig
                }

                const mdAccoutSetting: AccountSetting = {
                    ...config,
                    source,
                    type,
                    typeName,
                    key: config.key,
                    config: mdItemConfig
                }


                if(config.td_config !== undefined) tdSources[source] = tdAccountSetting;
                if(config.md_config !== undefined) mdSources[source] = mdAccoutSetting;
            }
        })
    } catch (err) {
        console.log(err)
        tdSources = {}
        mdSources = {}
    }
    return {
        td: tdSources,
        md: mdSources
    }
}
