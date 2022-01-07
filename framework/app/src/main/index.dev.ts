import { app, session } from 'electron';
import debug from 'electron-debug';
import path from 'path';

app.on('ready', async () => {
    try {
        const vueDevtoolDir = path.join(__resources, 'devtools', 'vue-devtool');
        console.log(vueDevtoolDir, '+++');
        await session.defaultSession.loadExtension(vueDevtoolDir, {
            allowFileAccess: true,
        });
    } catch (err) {
        console.error(err, '----');
    }
});

debug();
require('./index');
