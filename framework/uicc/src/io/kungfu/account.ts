import { getAllKfConfig, setKfConfig, removeKfConfig } from '@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils';
import { hidePasswordByLogger } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { logger } from '@kungfu-trader/kungfu-uicc/utils/logUtils';

const USER = process.env.RENDERER_TYPE === 'admin' ? "ADMIN" : "";

export const getTdList = (): Promise<Td[]> => {
    return new Promise(resolve => {
        const kfConfigList = getAllKfConfig();
        const tdList: Td[] = Object.values(kfConfigList || {})
            .filter((tdData: any) => {
                return +tdData.category === 1
            })
            .map((tdData: any) => {
                return {
                    account_id: `${tdData.group}_${tdData.name}`,
                    source_name: tdData.group.toString(),
                    config: tdData.value
                }
            })
            .sort((tdData1: any, tdData2: any) => {
                const result = tdData1.source_name.localeCompare(tdData2.source_name);
                return result === 0 ? tdData1.account_id.localeCompare(tdData2.account_id) : result;
            })

        resolve(tdList)
    })
}

export const getMdList = (): Promise<Md[]> => {
    return new Promise(resolve => {
        const kfConfigList = getAllKfConfig();
        const mdList = Object.values(kfConfigList || {})
            .filter((mdData: any) => {
                return +mdData.category === 0
            })
            .map((mdData: any) => {
                return {
                    source_name: mdData.group,
                    config: mdData.value
                }
            })
            .sort((mdData1: any, mdData2: any) => {
                return mdData1.source_name.localeCompare(mdData2.source_name);
            })
            
        resolve(mdList)
    })
}

export const addTd = (accountId: string, config: string) => {
    return new Promise(resolve => {
        const td = setKfConfig(accountId, 'td', config);
        logger.info("Add Trade Account", USER, accountId, hidePasswordByLogger(config));
        resolve(td)
    })
}

export const addMd = (sourceName: string, config: string) => {
    return new Promise(resolve => {
        const md = setKfConfig(sourceName, 'md', config);
        logger.info("Add Market Source", USER, sourceName, hidePasswordByLogger(config));
        resolve(md)
    })
}

export const updateTdConfig = (accountId: string, config: string, type?: string) => {
    return new Promise(resolve => {
        const td = setKfConfig(accountId, 'td', config);
        if (type === 'risk') {
            logger.info("Update Trade Account Risk", USER, accountId, hidePasswordByLogger(config));
        } else {
            logger.info("Update Trade Account", USER, accountId, hidePasswordByLogger(config));
        }
        resolve(td)
    })
}

export const updateMdConfig = (sourceName: string, config: string) => {
    return new Promise(resolve => {
        const md = setKfConfig(sourceName, 'md', config)
        logger.info("Update Market Source", USER, sourceName, hidePasswordByLogger(config));
        resolve(md)
    })
}

export const deleteTd = (accountId: string) => {
    return new Promise(resolve => {
        const td = removeKfConfig(accountId, 'td');
        logger.info("Delete Trade Account", USER, accountId);
        resolve(td)
    })
}

export const deleteMd = (sourceName: string) => {
    return new Promise(resolve => {
        const md = removeKfConfig(sourceName, 'md');
        logger.info("Delete Market Source", USER, sourceName);
        resolve(md)
    })
}

export const getExistedMdSources = () => getMdList()
    .then((mdList: any) => {
        return mdList.map((m: any) => m.source_name)
    })