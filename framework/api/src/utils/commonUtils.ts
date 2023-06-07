// 此文件内只放 不依赖外部逻辑 的 纯函数

export const booleanProcessEnv = (
  val: string | boolean | undefined,
): boolean => {
  if (val === undefined) {
    return false;
  }

  if (val === 'null') {
    return false;
  }

  if (val === 'true') {
    return true;
  } else if (val === 'false') {
    return false;
  } else {
    return !!val;
  }
};
