interface PromptAnswer {
  source: string;
}

type PromptInputType =
  | 'input'
  | 'number'
  | 'confirm'
  | 'list'
  | 'rawlist'
  | 'expand'
  | 'checkbox'
  | 'password'
  | 'editor'
  | 'path';

interface PromptQuestion {
  type: PromptInputType;
  name: string;
  default?: KungfuApi.KfConfigValue;
  choices: (string | number)[];
  message: string;
  validate: (value: KungfuApi.KfConfigValue) => Promise<Error | boolean>;
}
