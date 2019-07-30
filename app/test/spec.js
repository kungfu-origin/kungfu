const Application = require('spectron').Application
const assert = require('assert')
const electronPath = require('electron') // Require Electron from the binaries included in node_modules.
const path = require('path');
const config = require('./config.json');

process.env.APP_TYPE = 'test'
process.env.KUNGFU_ENGINE = config.KF_ENGINE;
process.env.ELEC_BASE_DIR = config.KF_HOME;

function delay(interval) {
	return new Promise(resolve => {
		let timer = setTimeout(() => {
			console.log('finish  ' + interval)
			resolve()
			clearTimeout(timer)
		}, interval || 1000)
	})
}

describe('Application launch', function () {
    this.timeout(100000000)

    before(function () {
		const t = this;
        t.app = new Application({
			path: electronPath,
			args: [path.join(__dirname, '..')]
        })
		return t.app.start()
    })

    // after(function () {
    //     if (this.app && this.app.isRunning()) {
    //     	return this.app.stop()
    //     }
    // })

	it('test add account of xtp', function() {
		const t = this;
		return delay(500)
		.then(async () => {
			const window0 = t.app.client.windowByIndex(0)
			const window1 = t.app.client.windowByIndex(1)
			const window0Title = await window0.$('#add-account-btn')
			const window1Title = await window1.$('#add-account-btn')

		})
		.then(async () => {
			await delay(500)
			await t.app.client.$('#add-account-btn').click()
			await delay(500)
			await t.app.client.$('#select-source-dialog .source-xtp .el-radio').click()
			await delay(500)
			await t.app.client.$('.confirm-select-source-btn').click()
			await delay(500)
		})
	})

   
})