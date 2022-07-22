import { FunctionalComponent } from '@vue/runtime-core/dist/runtime-core';
import { AntdIconProps } from '@ant-design/icons-vue/lib/components/AntdIcon';

export interface PrefixProps {
  key: string;
  prefix: string | FunctionalComponent<AntdIconProps>;
  prefixType: 'text' | 'icon';
}

const basePrefixs: Record<string, PrefixProps> = {
  default: {
    key: 'default',
    prefix: '',
    prefixType: 'text',
  },
};

export class PrefixHooks {
  hooks: Record<string, PrefixProps>;

  constructor() {
    this.hooks = new Proxy(basePrefixs, {
      get(target: Record<string, PrefixProps>, prop: string) {
        const locationPairs = prop.split('_');
        if (locationPairs.length != 3) {
          console.warn(`Invalid hook key: ${prop}`);
          return Reflect.get(target, 'default');
        }

        const [category, group, name] = prop.split('_');
        if (target[`${category}_${group}_${name}`]) {
          return target[`${category}_${group}_${name}`];
        } else if (target[`${category}_*_${name}`]) {
          return target[`${category}_*_${name}`];
        } else if (target[`${category}_${group}_*`]) {
          return target[`${category}_${group}_*`];
        } else if (target[`${category}_*_*`]) {
          return target[`${category}_*_*`];
        }

        return Reflect.get(target, 'default');
      },
      set(
        target: Record<string, PrefixProps>,
        prop: string,
        value: PrefixProps,
      ) {
        if (Reflect.has(target, prop)) {
          return true;
        }

        Reflect.set(target, prop, value);
        return true;
      },
    });
  }

  register(kfLocation: KungfuApi.DerivedKfLocation, props: PrefixProps) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    Reflect.set(this.hooks, key, props);
  }

  isRegistered(key: string) {
    return Reflect.has(this.hooks, key);
  }

  trigger(kfLocation: KungfuApi.DerivedKfLocation) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    return this.hooks[key];
  }
}
