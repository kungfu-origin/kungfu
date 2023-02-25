import { app, session } from 'electron';
import debug from 'electron-debug';
import path from 'path';

app.on('ready', async () => {
  try {
    const vueDevtoolDir = path.join(
      globalThis.__publicResources,
      'devtools',
      'vue-devtool',
    );
    await session.defaultSession.loadExtension(vueDevtoolDir, {
      allowFileAccess: true,
    });
  } catch (err) {
    console.error(err);
  }
});

debug();
require('./index');
