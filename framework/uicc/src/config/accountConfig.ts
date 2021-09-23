import * as VALIDATOR from '@kungfu-trader/kungfu-uicc/assets/validator'
import { getExtensionConfigs, deepClone } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { SourceTypeConfig } from '@kungfu-trader/kungfu-uicc/config/tradingConfig';
import { EXTENSION_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';

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
    let riskSources: Sources = {};

    try {
        const configs = await getExtensionConfigs(EXTENSION_DIR);
        configs.forEach((c: any): void => {
            if(c.type === 'source') {
                const config: AccountSettingOrigin = c.config;
                const source: string = config.name;
                const typeName: string = config.type;
                const type: string = (SourceTypeConfig[typeName] || {}).color || '';
        
                let tdItemConfig: AccountSettingItem[] = deepClone((config.td_config || []).filter((configItem: AccountSettingItem) => !configItem.risk) || []);
                let mdItemConfig: AccountSettingItem[] = deepClone(config.md_config || []);
                let riskItemConfig: AccountSettingItem[] = deepClone((config.td_config || []).filter((configItem: AccountSettingItem) => configItem.risk) || []);
                tdItemConfig = dealValidator(tdItemConfig); 
                mdItemConfig = dealValidator(mdItemConfig);
                riskItemConfig = dealValidator(riskItemConfig);

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
                
                const riskAccountSetting: AccountSetting = {
                    ...config,
                    source,
                    type,
                    typeName,
                    key: config.key,
                    config: riskItemConfig
                }

                if (tdItemConfig.length) tdSources[source] = tdAccountSetting;
                //sim md_config 为 []
                if (mdItemConfig.length || config.name === 'sim') mdSources[source] = mdAccoutSetting;
                if (riskItemConfig.length) riskSources[source] = riskAccountSetting;
            }
        })
    } catch (err) {
        tdSources = {}
        mdSources = {}
        riskSources = {}
    }
    return {
        td: tdSources,
        md: mdSources,
        risk: riskSources
    }
}
