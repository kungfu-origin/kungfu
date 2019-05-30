import Table from '../public/Table';
import { calcuHeaderWidth, parseToString } from './utils';
import { sourceType } from '@/assets/config/accountConfig';
import { toDecimal } from '@/assets/js/utils';


function AccountTable(){
    if (!(this instanceof AccountTable)) {
        return new AccountTable();
    }
    Table.call(this)
	this.headers = ['Account', 'Source', 'Status', 'Pnl', 'PnlRt','Total', 'Avail'];
	this.columnWidth = [10, 0, 0, 8, 6, 8, 8]
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
AccountTable.prototype.refresh = function(accountData, processStatus, cashData){

	//accont (td + md)
	accountData = Object.values(accountData || {});
	const accountListData = accountData.map(a => {
		const accountId = a.account_id.toAccountId();
		const tdProcess = processStatus[`td_${a.account_id}`] || '--';
		const cash = cashData[accountId] || {};
		const typeName = (sourceType[a.source_name] || {}).typeName
		const total = typeName === 'future' ? cash.margin : cash.market_value
		const accumulatedPnlRatio = toDecimal(cash.accumulated_pnl_ratio) + ''
		return parseToString([
			accountId,
			a.source_name,
			tdProcess,
			toDecimal(cash.accumulated_pnl) + '' || '--',
			accumulatedPnlRatio ? accumulatedPnlRatio + '%' : '--',
			toDecimal(total) + '' || '--',
			toDecimal(cash.avail) + '' || '--',
        ], calcuHeaderWidth(this.headers, this.columnWidth))
    })
	this.table.setItems(accountListData)
}

export default AccountTable()