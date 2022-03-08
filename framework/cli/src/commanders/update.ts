import colors from 'colors';
import inquirer from 'inquirer';
import { getAllKfConfigOriginData } from '@kungfu-trader/kungfu-js-api/actions';
import {
  parseToString,
  getKfCategoryFromString,
} from '../assets/methods/utils';
import { getIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

export const updateMdTdStrategy = async () => {
  const { md, td, strategy } = await getAllKfConfigOriginData();
  const mdTdStrategyList = [
    ...md.map((item) =>
      parseToString(
        [colors.yellow('md'), getIdByKfLocation(item)],
        [8, 'auto'],
        1,
      ),
    ),
    ...td.map((item) =>
      parseToString(
        [colors.blue('td'), getIdByKfLocation(item)],
        [8, 'auto'],
        1,
      ),
    ),
    ...strategy.map((item) =>
      parseToString(
        [colors.cyan('strategy'), getIdByKfLocation(item)],
        [8, 'auto'],
        1,
      ),
    ),
  ];

  const answers: { process: string } = await inquirer.prompt([
    {
      type: 'autocomplete',
      name: 'process',
      message: 'Select targeted md / td / strategy to update    ',
      source: async (answersSoFar: { process: string }, input: string) => {
        input = input || '';
        return mdTdStrategyList.filter((s: string): boolean =>
          s.includes(input),
        );
      },
    },
  ]);

  console.log(answers);

  const processes = answers.process;
  const splits = processes.split(' ');
  const targetType = splits[0].trim();
  const targetId = splits[splits.length - 1].trim();
  const type = getKfCategoryFromString(targetType);

  console.log(type, targetId, '===');
};
