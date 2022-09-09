import { getAllKfConfigOriginData } from '@kungfu-trader/kungfu-js-api/actions';
import {
  getIdByKfLocation,
  getKfExtensionConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import colors from 'colors';
import { dealKfConfigValue, parseToString } from '../assets/methods/utils';

export const listKfLocations = async (): Promise<void> => {
  const { md, td, strategy } = await getAllKfConfigOriginData();
  const extConfigs = await getKfExtensionConfig();

  const mdList = await Promise.all(
    md.map(async (m): Promise<string> => {
      return parseToString(
        [
          colors.yellow('Md'),
          colors.bold(getIdByKfLocation(m)),
          await dealKfConfigValue(m, extConfigs),
        ],
        [10, 15, 'auto'],
        1,
      );
    }),
  );

  const tdList = await Promise.all(
    td.map(async (t): Promise<string> => {
      return parseToString(
        [
          colors.cyan('Td'),
          colors.bold(getIdByKfLocation(t)),
          await dealKfConfigValue(t, extConfigs),
        ],
        [10, 15, 'auto'],
        1,
      );
    }),
  );
  const strategyList = strategy.map((s): string => {
    return parseToString(
      [
        colors.blue('strategy'),
        colors.bold(getIdByKfLocation(s)),
        JSON.parse(s.value).strategy_path,
      ],
      [10, 15, 'auto'],
      1,
    );
  });

  if ([...mdList, ...tdList, ...strategyList].length === 0) {
    console.log('No mds, tds and strategies');
  } else {
    console.log(
      [
        `\n${colors.bold.underline('Md:')} \n`,
        ...mdList,
        `\n${colors.bold.underline('Td:')} \n`,
        ...tdList,
        `\n${colors.bold.underline('Strategy:')} \n`,
        ...strategyList,
      ].join('\n'),
    );
  }
};
