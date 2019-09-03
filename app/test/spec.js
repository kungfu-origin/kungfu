const Application = require('spectron').Application
const assert = require('assert')
const electronPath = require('electron') // Require Electron from the binaries included in node_modules.
const path = require('path');
const config = require('./config.json');

process.env.APP_TYPE = 'test'
process.env.KUNGFU_ENGINE_PATH = config.KF_ENGINE;
process.env.ELEC_BASE_DIR = config.KF_HOME;

function delay(interval) {
	return new Promise(resolve => {
		let timer = setTimeout(() => {
			// console.log('finish  ' + interval)
			resolve()
			clearTimeout(timer)
		}, interval || 1000)
	})
}

describe('test add account', function () {
    this.timeout(10000)

    before(function () {
		const t = this;
        t.app = new Application({
			path: electronPath,
			args: [path.join(__dirname, '..')]
        })
		return t.app.start()
    })

    after(function () {
        if (this.app && this.app.isRunning()) {
        	return this.app.stop()
        }
    })

	it('test add account 0', function() {
		const t = this;
		return delay(1000)
		.then(async () => {
			const window0 = t.app.client.windowByIndex(0)
			const window1 = t.app.client.windowByIndex(1)
			const window0Title = await window0.$('#add-account-btn')
			const window1Title = await window1.$('#add-account-btn')
		})
		.then(async () => {
			await t.app.client.$('#add-account-btn').click()
			await delay(500)
			await t.app.client.$('#select-source-dialog .source-xtp .el-radio').click()
			await delay(500)
			await t.app.client.$('#confirm-select-source-btn').click()
			await delay(500)
		})
		.then(async () => {		
			const accounts0 = config.accounts.xtp[0]
			const promises = Object.keys(accounts0).map((key) => {
				const value = accounts0[key]
				return t.app.client.$(`#confirm-set-account-dialog .${key} input`).setValue(value)
			})
			await Promise.all(promises)
			await delay(500)
		})
		.then(async () => {				
			await t.app.client.$('#confirm-set-account-btn').click()
			await delay(500)
			const errors = await t.app.client.$$('#confirm-set-account-dialog .el-form-item__error')
			return assert.equal(errors.length, 0)
		})
		.then(async () => {
			const accounts0 = config.accounts.xtp[0]
			const userId = accounts0.user_id;
			const targetAccount = await t.app.client.$$(`.accounts-table .xtp_${userId}`)
			return assert.equal(targetAccount.length, 1)
		})
	})
})

