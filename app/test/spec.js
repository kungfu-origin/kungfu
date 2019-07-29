const Application = require('spectron').Application
const assert = require('assert')
const electronPath = require('electron') // Require Electron from the binaries included in node_modules.
const path = require('path');
const config = require('./config.json');

process.env.APP_TYPE = 'test'
process.env.KUNGFU_ENGINE = config.KF_ENGINE;
process.env.ELEC_BASE_DIR = config.KF_HOME

describe('Application launch', function () {
    this.timeout(100000000)

    before(function () {
		const t = this;
        t.app = new Application({
			// Your electron path can be any binary
			// i.e for OSX an example path could be '/Applications/MyApp.app/Contents/MacOS/MyApp'
			// But for the sake of the example we fetch it from our node_modules.
			path: electronPath,
			// path: '/Applications/Kungfu.Trader.app/Contents/MacOS/Kungfu.Trader'

			// Assuming you have the following directory structure

			//  |__ my project
			//     |__ ...
			//     |__ main.js
			//     |__ package.json
			//     |__ index.html
			//     |__ ...
			//     |__ test
			//        |__ spec.js  <- You are here! ~ Well you should be.

			// The following line tells spectron to look and use the main.js file
			// and the package.json located 1 level above.
			args: [path.join(__dirname, '..')]
        })
		return t.app.start()
		.then(() => {
			return delay(5000)
		})

		function delay(interval) {
			return new Promise(resolve => {
				let timer = setTimeout(() => {
					console.log('finish  ' + interval)
					resolve()
					clearTimeout(timer)
				}, interval || 1000)
			})
		}
    })

    after(function () {
        if (this.app && this.app.isRunning()) {
        	return this.app.stop()
        }
    })

    it('shows an initial window', function () {
        return this.app.client.getWindowCount().then(function (count) {
        	assert.equal(count, 1)
        })
    })

   
})