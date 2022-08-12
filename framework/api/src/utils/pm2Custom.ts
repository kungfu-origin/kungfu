import pm2 from 'pm2';
import os from 'os';

const pm2Custom = new pm2['custom']();

if (process.platform === 'win32') {
  const USERNAME = os.userInfo().username;
  pm2Custom._conf.DAEMON_RPC_PORT = `\\\\.\\pipe\\${USERNAME}\\rpc.sock`;
  pm2Custom._conf.DAEMON_PUB_PORT = `\\\\.\\pipe\\${USERNAME}\\pub.sock`;
  pm2Custom._conf.INTERACTOR_RPC_PORT = `\\\\.\\pipe\\${USERNAME}\\interactor.sock`;
}

export default pm2Custom;
