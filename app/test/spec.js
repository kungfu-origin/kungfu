const Application = require('spectron').Application
const assert = require('assert');
const path = require('path');
const electronPath = require('electron') // Require Electron from the binaries included in node_modules.


describe('Application launch', function () {
    this.timeout(100000)
    before(function(){
        this.app = new Application({
        // Your electron path can be any binary
        // i.e for OSX an example path could be '/Applications/MyApp.app/Contents/MacOS/MyApp'
        // But for the sake of the example we fetch it from our node_modules.
            // path: '/Applications/Kungfu.Trader.app',
            path: electronPath,
            args: [path.join(__dirname, '..')]
        })
        return this.app.start()
    })

    after(function () {
        if (this.app && this.app.isRunning()) {
            return this.app.stop()
        }
      })

    it('shows an initial window', function (done) {
        const t = this;
        setTimeout(async () => {
            const list0 = await t.app.client.getSelectedText()
            console.log(list0)
            done()
        }, 5000)
    })
})
