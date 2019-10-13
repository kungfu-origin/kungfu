// @flow

declare class ScreenManager {
  render(content: string, bottomContent: string) : void;
  done() : void;
}

declare class BasePrompt {
  rl: ReadLineInterface;
  screen: ScreenManager;
  answers: {[key: string]: any};
  status: string;
  opt: {
    name: string,
    message: string,
    default: string,
    validate: (path: string) => boolean,
    filter: (path: string) => any,
    when: (answers: {}) => boolean,
    [string]: any
  };

  constructor(
    question: {
      name: string,
      message: string,
      default?: string,
      validate?: (path: string) => boolean,
      filter?: (path: string) => any,
      when?: (answers: {}) => boolean,
      [string]: any
    },
    rl: ReadLineInterface,
    answers: {}
  ): BasePrompt;
}

declare module 'inquirer/lib/utils/screen-manager' {
  declare export default typeof ScreenManager;
}

declare module 'inquirer/lib/prompts/base' {
  declare export default typeof BasePrompt;
}
