import Table from '../public/Table';
import { calcuHeaderWidth, parseToString } from './utils';


function AccountTable(){
    if (!(this instanceof AccountTable)) {
        return new AccountTable();
    }
    Table.call(this)
    this.headers = ['Account', 'Source', 'Status', 'Accum',' AcumRt','Total', 'Avail'];
}

AccountTable.prototype = new Table();

AccountTable.prototype.getData = function(globalData={}){
	return this.getDataMethod().then(list => {
		//td
		list.forEach(l => {
			const accountId = l.account_id.toAccountId();
			if(globalData.accountData[accountId]) globalData.accountData[accountId] = {...globalData.accountData[accountId], ...l}
			else globalData.accountData[accountId] = {...l, status: '--', accumulated_pnl : '--', accumulated_pnl_ratio: '--', 'total': '--', 'avail': '--'}
		});

		//md
		list.filter(l => !!l.receive_md).forEach(l => {
			const source = l.source_name;
			if(globalData.mdData[source]) globalData.mdData[source] = {...globalData.mdData[source], ...l}
			else globalData.mdData[source] = {...l, status: '--'}
        })
        return globalData
	})
}
/**
 * @param  {Object} accountData
 * @param  {Object} processStatus
 */
AccountTable.prototype.refresh = function(accountData, processStatus){

	//accont (td + md)
	accountData = Object.values(accountData || {});
	const accountListData = accountData.map(a => {
		let tdProcess = processStatus[`td_${a.account_id}`] || '--'
		// if(tdProcess === 'online') tdProcess = colors.green(tdProcess);
		// else if(tdProcess === 'error') tdProcess = colors.red(tdProcess)
        // else tdProcess = colors.grey(tdProcess)
		return parseToString([
			a.account_id.toAccountId(),
			a.source_name,
			tdProcess,
			a.accumulated_pnl,
			a.accumulated_pnl_ratio,
			a.total, a.avail
        ], calcuHeaderWidth(this.headers))
    })
	this.table.setItems(accountListData)
}

export default AccountTable()