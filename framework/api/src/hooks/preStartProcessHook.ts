import { Proc } from 'pm2';
import { kfLogger } from '../utils/busiUtils';

export type PreStartProcessMethod = (
  kfLocation: KungfuApi.DerivedKfLocation,
) => Promise<Proc | void>;

export class PreStartProcessHooks {
  hooks: Record<string, PreStartProcessMethod[]>;
  constructor() {
    this.hooks = new Proxy(
      {},
      {
        get(target: Record<string, PreStartProcessMethod[]>, prop: string) {
          const locationPairs = prop.split('_');
          if (locationPairs.length != 3) {
            kfLogger.warn(`Invalid hook key: ${prop}`);
            return [];
          }

          const [category, group, name] = prop.split('_');
          if (
            target[`${category}_${group}_${name}`] &&
            target[`${category}_${group}_${name}`].length
          ) {
            return target[`${category}_${group}_${name}`];
          } else if (
            target[`${category}_*_${name}`] &&
            target[`${category}_*_${name}`].length
          ) {
            return target[`${category}_*_${name}`];
          } else if (
            target[`${category}_${group}_*`] &&
            target[`${category}_${group}_*`].length
          ) {
            return target[`${category}_${group}_*`];
          } else if (
            target[`${category}_*_*`] &&
            target[`${category}_*_*`].length
          ) {
            return target[`${category}_*_*`];
          }

          return [];
        },

        set(
          target: Record<string, PreStartProcessMethod[]>,
          prop: string,
          value: PreStartProcessMethod,
        ) {
          if (!Reflect.has(target, prop)) {
            Reflect.set(target, prop, []);
          }

          const methods = Reflect.get(target, prop);
          methods.push(value);
          Reflect.set(target, prop, methods);
          kfLogger.info(`PreStartProcess hook ${prop} register success`);
          return true;
        },
      },
    );
  }

  register(
    kfLocation: KungfuApi.DerivedKfLocation,
    method: PreStartProcessMethod,
  ) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    Reflect.set(this.hooks, key, method);
  }

  trigger(kfLocation: KungfuApi.DerivedKfLocation) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    return Promise.all(
      Reflect.get(this.hooks, key).map((method) => method(kfLocation)),
    ).catch((err) => {
      console.warn(<Error>err);
    });
  }
}
