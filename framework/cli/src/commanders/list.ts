import { getAllKfConfigOriginData } from '@kungfu-trader/kungfu-js-api/actions';
import { getIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import colors from 'colors';
import { parseToString } from '../assets/methods/utils';

export const listKfLocations = async (): Promise<void> => {
  const { md, td, strategy } = await getAllKfConfigOriginData();

  const mdList = md.map((m): string => {
    return parseToString(
      [colors.yellow('Md'), colors.bold(getIdByKfLocation(m)), m.value],
      [10, 15, 'auto'],
      1,
    );
  });

  const tdList = td.map((t): string => {
    return parseToString(
      [colors.cyan('Td'), colors.bold(getIdByKfLocation(t)), t.value],
      [10, 15, 'auto'],
      1,
    );
  });
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
