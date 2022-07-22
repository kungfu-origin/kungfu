import { Pm2StartOptions } from '../typings/global';

export type ResolveStartOptionsHook = (
  kfLocation: KungfuApi.DerivedKfLocation,
  options: Pm2StartOptions,
) => Promise<Pm2StartOptions>;

export class ResolveStartOptionsHooks {
  hooks: Record<string, ResolveStartOptionsHook>;
  constructor() {
    this.hooks = new Proxy(
      {},
      {
        get(target: Record<string, ResolveStartOptionsHook>, prop: string) {
          const locationPairs = prop.split('_');
          if (locationPairs.length != 3) {
            console.warn(`Invalid hook key: ${prop}`);
            return [];
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

          return (
            kfLocation: KungfuApi.DerivedKfLocation,
            options: Pm2StartOptions,
          ) => Promise.resolve(options);
        },

        set(
          target: Record<string, ResolveStartOptionsHook>,
          prop: string,
          value: ResolveStartOptionsHook,
        ) {
          if (Reflect.has(target, prop)) {
            console.warn(`ResolveStartOption hook ${prop} already exists`);
            return true;
          }

          console.log(`ResolveStartOption hook ${prop} register success`);
          Reflect.set(target, prop, value);
          return true;
        },
      },
    );
  }

  register(
    kfLocation: KungfuApi.DerivedKfLocation,
    method: ResolveStartOptionsHook,
  ) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    Reflect.set(this.hooks, key, method);
  }

  trigger(kfLocation: KungfuApi.DerivedKfLocation, options: Pm2StartOptions) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    return this.hooks[key](kfLocation, options);
  }
}
