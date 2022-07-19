import { FunctionalComponent } from '@vue/runtime-core/dist/runtime-core';
import { AntdIconProps } from '@ant-design/icons-vue/lib/components/AntdIcon';

const basePrefixs: Record<string, PrefixRegisterProps> = {
  default: {
    key: 'default',
    prefix: '',
    prefixType: 'text',
  },
};

export interface PrefixRegisterProps {
  key: string;
  prefix: string | FunctionalComponent<AntdIconProps>;
  prefixType: 'text' | 'icon';
}

export class PrefixRegister {
  prefixs: Record<string, PrefixRegisterProps>;
  callbacks: Array<(PrefixRegisterProps, PrefixRegister) => void>;

  constructor() {
    this.prefixs = { ...basePrefixs };
    this.callbacks = [];
  }

  register(props: PrefixRegisterProps) {
    if (this.prefixs[props.key]) return;

    this.prefixs[props.key] = props;
    this.callbacks.forEach((callback) => {
      callback(this.prefixs[props.key], this);
    });
  }

  onRegister(callback) {
    this.callbacks.push(callback);
  }

  isRegistered(key: string) {
    return !!this.prefixs[key];
  }

  getPrefixDataBykey(key: string) {
    return this.prefixs[key] || this.prefixs['default'];
  }
}

export const usePrefix = () => {
  const builtPrefixMap = (
    prefixRegister: PrefixRegister,
    keyList: string[],
  ) => {
    return keyList.reduce((prefixMap, key) => {
      if (prefixRegister.isRegistered(key)) {
        prefixMap[key] = prefixRegister.getPrefixDataBykey(key);
      }
      return prefixMap;
    }, {} as Record<string, PrefixRegisterProps>);
  };

  return {
    builtPrefixMap,
  };
};
