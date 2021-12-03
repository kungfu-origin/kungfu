import { app, session } from 'electron';
import debug from 'electron-debug';
import path from 'path';

app.on('ready', async () => {
    try {
        console.log(
            'vue-devtool-path',
            path.resolve('./.devtools/vue-devtool'),
        );
        await session.defaultSession.loadExtension(
            path.resolve('./.devtools/vue-devtool'),
            { allowFileAccess: true },
        );
    } catch (err) {
        console.error(err);
    }
});

debug();
require('./index');
