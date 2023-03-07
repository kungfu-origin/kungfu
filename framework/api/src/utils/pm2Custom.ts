import pm2 from 'pm2';
import os from 'os';

const pm2Custom = new pm2['custom']({ pm2_home: os.homedir() });

export default pm2Custom;
