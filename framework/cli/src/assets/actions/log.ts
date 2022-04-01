import { Tail } from 'tail';
import path from 'path';
import colors from 'colors';
import { buildProcessLogPath } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
  isCriticalLog,
  KfNumList,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { ensureFileSync } from 'fs-extra';

export class Log {
  logTail: Tail | null;
  lines: number;
  logList: KungfuApi.KfNumList<string>;
  EXTRA_LINES: number;

  constructor(lines = 10000) {
    this.EXTRA_LINES = 1000;
    this.logTail = null;
    this.lines = lines;
    this.logList = new KfNumList(lines + this.EXTRA_LINES);
  }

  initProcessId(processId: string) {
    this.logList = new KfNumList(this.lines + this.EXTRA_LINES);
    this.tailLog(processId);
  }

  tailLog(processId: string) {
    this.logTail && this.logTail.unwatch();
    this.logTail = new Tail(this._getLogPath(processId), {
      follow: true,
      nLines: this.lines,
    });

    this.logTail.on('line', (line: string) => {
      this.logList.insert(dealLogMessage(preDealLogMessage(line)));
    });

    this.logTail.on('error', (err: Error) => {
      console.error(err);
    });

    this.logTail.watch();
  }

  getLogs() {
    return this.logList.list;
  }

  _getLogPath(processId: string) {
    const logPath = path.join(buildProcessLogPath(processId));
    ensureFileSync(logPath);
    return logPath;
  }
}

function preDealLogMessage(line: string): string {
  const lineResolved = line.slice(20).split('] ');
  lineResolved.splice(2, 1);
  if (lineResolved.length > 3) {
    lineResolved.splice(2, 1);
  }
  return lineResolved.join('] ');
}

function dealLogMessage(line: string): string {
  if (isCriticalLog(line)) {
    line = colors.red(line);
    return line;
  }

  line = line
    .replace(/warning/g, colors.yellow(' warning'))
    .replace(/ error /g, colors.red(' error'))
    .replace(/ debug /g, colors.blue('debug'))
    .replace(/ trace /g, colors.grey('trace'));

  return line;
}
