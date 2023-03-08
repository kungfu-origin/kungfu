import pm2 from 'pm2';
import os from 'os';
import path from 'path';

process.env.OVER_HOME = path.resolve(path.join(os.homedir(), '.pm2'));
if (process.env.NODE_ENV === 'development') {
  process.env.PM2_DEBUG = true;
}
const pm2Custom = new pm2['custom']();

export default pm2Custom;
