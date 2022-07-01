import colors from 'colors';
import { getPadSpace, isObject } from '../methods/utils';

class JsonTree {
  title: string;
  json: Record<string, Record<string, string | boolean | number>>;
  tipsMap: Record<string, Record<string, string>>;
  aliasKeyMap: Record<string, string>;
  contentList: string[];

  pad: number;
  gap: number;
  keyWidth: number;
  tipWidth: number;
  constructor({
    title = '',
    json = {},
    pad = 1,
    gap = 2,
    keyWidth = 12,
    tipWidth = 50,
    tipsMap = {},
    aliasKeyMap = {},
  }) {
    this.title = title;
    this.json = json;
    this.pad = pad;
    this.gap = gap;
    this.keyWidth = keyWidth;
    this.tipWidth = tipWidth;
    this.tipsMap = tipsMap;
    this.aliasKeyMap = aliasKeyMap;
    this.contentList = [];
  }

  addLines(str: string | string[]) {
    const line = typeof str === 'string' ? [str] : str;
    this.contentList.push(...line);
  }

  goNewLine() {
    this.addLines('');
  }

  dealKey(key: string) {
    if (!key) return key;

    const alias = this.aliasKeyMap[key] ?? key;
    return alias
      .split(' ')
      .map((item) => item[0].toUpperCase() + item.slice(1))
      .join(' ');
  }

  dealValue(value: unknown) {
    if (['boolean', 'number'].indexOf(typeof value) !== -1) return value;
    return value || 'None';
  }

  build() {
    this.goNewLine();
    this.title && this.addLines(`${colors.bold(this.title)}:`);

    this.buildTree(this.json, 0, this.tipsMap);
    this.goNewLine();

    return this.contentList.join('\n');
  }

  getKeysToTipsMap(
    keys: string[],
    tipsMap: Record<string, Record<string, string>>,
  ) {
    return keys.reduce((pre, cur) => {
      return {
        ...pre,
        ...(tipsMap[cur] || {}),
      };
    }, {});
  }

  buildTree(json, level = 0, tipsMap = {}) {
    const keys = Object.keys(json);

    keys.forEach((key) => {
      const value = json[key];
      const hasChildren = isObject(value);
      const currentPad = this.pad * level;
      const currentTipString =
        typeof tipsMap[key] === 'string' ? `${tipsMap[key]}` : '';

      if (hasChildren) {
        const currentKeyString = colors.bold(
          `${this.dealKey(key)}${currentTipString && `(${currentTipString})`}:`,
        );

        this.addLines(currentKeyString);
        this.buildTree(value, 2, tipsMap[key]);
        this.goNewLine();
      } else {
        const currentKeyString = this.parseKeyToString(
          this.dealKey(key),
          currentPad,
          this.keyWidth,
        );

        const parsedTip = this.parseTipToStringArr(
          currentTipString,
          currentPad + this.keyWidth + this.gap,
          this.tipWidth,
        );

        const currentContent = `${currentKeyString}${getPadSpace(this.gap)}${
          parsedTip[0]
        }${getPadSpace(this.gap)}${this.dealValue(value)}`;

        this.addLines(currentContent);

        if (parsedTip.length > 1) {
          this.addLines(parsedTip.slice(1));
        }
      }
    });
  }

  parseKeyToString(str: string, pad: number, targetWidth: number) {
    if (!str) return str;

    const leftPad = getPadSpace(pad);
    const rightPad = getPadSpace(
      targetWidth >= str.length ? targetWidth - str.length : 0,
    );
    return leftPad + str + rightPad;
  }

  parseTipToStringArr(str: string, pad: number, targetWidth: number) {
    if (str.length <= targetWidth)
      return [this.parseKeyToString(str, 0, this.tipWidth)];

    const stringArr: string[] = [str.slice(0, targetWidth)];
    let start = targetWidth;
    while (start < str.length) {
      const curStr = getPadSpace(pad) + str.slice(start, start + targetWidth);
      stringArr.push(curStr);
      start += targetWidth;
    }

    return stringArr;
  }
}

export default JsonTree;
