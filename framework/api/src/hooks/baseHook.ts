export class ResetOptionHook<Method, ValueType> {
  hookName: string;
  hooks: Record<string, Method>;
  constructor(name: string) {
    this.hookName = name;
    this.hooks = new Proxy(
      {},
      {
        get(target: Record<string, Method>, prop: string) {
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

          return (kfLocation: KungfuApi.DerivedKfLocation, value: ValueType) =>
            Promise.resolve(value);
        },

        set(target: Record<string, Method>, prop: string, value: Method) {
          if (Reflect.has(target, prop)) {
            console.warn(`${name} hook ${prop} already exists`);
            return true;
          }

          console.log(`${name} hook ${prop} register success`);
          Reflect.set(target, prop, value);
          return true;
        },
      },
    );
  }

  register(kfLocation: KungfuApi.DerivedKfLocation, method: Method) {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    Reflect.set(this.hooks, key, method);
  }

  trigger(
    kfLocation: KungfuApi.DerivedKfLocation,
    options: ValueType,
  ): Promise<ValueType> {
    const { category, group, name } = kfLocation;
    const key = `${category}_${group}_${name}`;
    return Reflect.get(this.hooks, key)(kfLocation, options);
  }
}
