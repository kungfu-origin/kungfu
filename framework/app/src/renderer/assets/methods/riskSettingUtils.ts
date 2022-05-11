import { toRaw } from 'vue';
import { RemoveRiskSetting } from '@kungfu-trader/kungfu-js-api/kungfu/riskSetting';

export function removeEmptyRiskSetting(
  riskSetting: KungfuApi.RiskSetting[],
): KungfuApi.RiskSetting[] {
  return toRaw(
    riskSetting
      .filter((item) => item.account_id !== '')
      .map((risk) => toRaw(risk)),
  );
}

export function dealRiskSettingStoreData(riskData: KungfuApi.RiskSetting[]) {
  const riskDataResolved = riskData.map((item) => {
    return {
      ...item,
      name: item.account_id || '',
      category: item.category || 'td',
      mode: 'live',
      group: item.account_id ? item.account_id.split('_')[0] : '',
      source_id: item.account_id ? item.account_id.split('_')[0] : '',
    };
  });

  return riskDataResolved;
}

export function delateRiskFromStates(
  riskControl: KungfuApi.RiskSetting[],
  accountList: string[],
) {
  if (accountList.length) {
    riskControl.forEach((item) => {
      if (item.account_id && accountList.includes(item.account_id)) {
        const KfLocation: KungfuApi.KfLocation = {
          category: item.category,
          group: item.group,
          name: item.name,
          mode: item.mode,
        };

        RemoveRiskSetting(KfLocation);
      }
    });
  }
}
