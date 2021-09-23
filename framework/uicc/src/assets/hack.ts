const fs = require('fs');
const path = require('path');
const Common = require('pm2/lib/Common.js');
const util = require('util');
const debug = require('debug')('pm2:client');

export function hackLaunchDaemon (opts: any, cb: Function) {
    if (typeof(opts) == 'function') {
        cb = opts;
        opts = {
            interactor : false
        };
    }
    
    // @ts-ignore
    var that = this
    var node_args = [];
    var out, err;
    

    out = fs.openSync(that.conf.PM2_LOG_FILE_PATH, 'a'),
    err = fs.openSync(that.conf.PM2_LOG_FILE_PATH, 'a');
    
    // @ts-ignore
    if (this.conf.LOW_MEMORY_ENVIRONMENT) {
        var os = require('os');
        node_args.push('--gc-global'); // Does full GC (smaller memory footprint)
        node_args.push('--max-old-space-size=' + Math.floor(os.totalmem() / 1024 / 1024));
    }
    
    /**
     * Add node [arguments] depending on PM2_NODE_OPTIONS env variable
     */
    if (process.env.PM2_NODE_OPTIONS)
        node_args = node_args.concat(process.env.PM2_NODE_OPTIONS.split(' '));
    
    if (!process.env.PM2_DISCRETE_MODE)
    // @ts-ignore
        Common.printOut(that.conf.PREFIX_MSG + 'Spawning PM2 daemon with pm2_home=' + this.pm2_home);
    
    // a webpack issue, use __non_webpack_require__ repl
    // @ts-ignore
    const ClientJS = path.resolve(__non_webpack_require__.resolve('pm2/lib/Daemon.js'))
    
    var child = require('child_process').fork(ClientJS, node_args, {
        detached   : true,
        cwd        : that.conf.cwd || process.cwd(),
        env        : util._extend({
          'SILENT'      : that.conf.DEBUG ? !that.conf.DEBUG : true,
          'PM2_HOME'   : that.pm2_home
        }, process.env),
        stdio      : ['ipc', out, err]
    })

    function onError(e: Error) {
        console.error(e.message || e);
        return cb ? cb(e.message || e) : false;
    }
    
    child.once('error', onError);
    
    child.unref();
    
    child.once('message', function(msg: any) {
        debug('PM2 daemon launched with return message: ', msg);
        child.removeListener('error', onError);
        child.disconnect();
    
        if (opts && opts.interactor == false)
        return cb(null, child);
    
        if (process.env.PM2_NO_INTERACTION == 'true')
        return cb(null, child);
    });
}
