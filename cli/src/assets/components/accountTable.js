import colors from 'colors';
import Table from './Table';
import { calcuHeaderWidth, parseToString, parseAccountList, dealNum, dealStatus } from '../utils';
import { sourceType } from '@/assets/config/accountConfig';
import { toDecimal } from '@/assets/js/utils';


function AccountTable(){
    if (!(this instanceof AccountTable)) {
        return new AccountTable();
    }
    Table.call(this)
	this.headers = ['Account', 'Source', 'Status', 'Pnl', 'PnlRt','Total', 'Avail'];
	this.columnWidth = [10, 0, 8, 10, 6, 10, 10]
}

AccountTable.prototype = new Table();

AccountTable.prototype.getData = function(globalData={}){
	return this.getDataMethod().then(list => parseAccountList(globalData, list))
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
		const tdProcess = dealStatus(processStatus[`td_${a.account_id}`]) 
		const cash = cashData[accountId] || {};
		const typeName = (sourceType[a.source_name] || {}).typeName
		const total = typeName === 'future' ? cash.margin : cash.market_value
		const accumulatedPnlRatio = toDecimal(cash.accumulated_pnl_ratio) + '';
		return parseToString([
			accountId,
			a.source_name,
			tdProcess,
			dealNum(cash.accumulated_pnl),
			dealNum(accumulatedPnlRatio, true),
			toDecimal(total) + '' || '--',
			toDecimal(cash.avail) + '' || '--',
        ].slice(0, this.headers.length), calcuHeaderWidth(this.headers, this.columnWidth))
    })
	this.table.setItems(accountListData)
}

export default AccountTable()