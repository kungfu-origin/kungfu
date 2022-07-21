import { Proc } from 'pm2';

export type StartProcessHook = (
  kfLocation: KungfuApi.KfLocationResolved,
) => Promise<Proc | void>;

export class StartProcessHooks {
  hooks: Record<string, StartProcessHook>;
  constructor() {
    this.hooks = new Proxy(
      {},
      {
        get(target: Record<string, StartProcessHook>, prop: string) {
          const locationPairs = prop.split('_');
          if (locationPairs.length != 3) {
            console.warn(`Invalid hook key: ${prop}`);
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

          return []-;
        },
      },
    );
  }

  register(
    kfLocation: KungfuApi.KfLocationResolved,
    method: PreStartProcessMethod,
  ) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    if (!this.hooks[key]) {
      this.hooks[key] = [];
    }

    this.hooks[key].push(method);
  }

  run(kfLocation: KungfuApi.KfLocationResolved) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    return Promise.all(this.hooks[key].map((method) => method(kfLocation)));
  }
}
