import monitor from '../components/monitor';
// it is so important, because inquirer event will conflict with blessed
process.stdin.removeAllListeners('data');

export const monitPrompt = async (list: boolean) => {
  if (list) return;
  return monitor();
};
