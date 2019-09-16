import Table from '@/assets/components/Table';
import { calcuHeaderWidth, parseToString, parseAccountList, dealNum, dealStatus } from '@/assets/utils';
import { sourceType } from '__gConfig/accountConfig';
import { toDecimal } from '__gUtils/busiUtils';


function AccountTable(){
    if (!(this instanceof AccountTable)) {
        return new AccountTable();
    }
    Table.call(this)
	this.headers = ['Account', 'Source', 'Status', 'RealPnl', 'UnrealPnl','Total', 'Avail'];
	this.columnWidth = [10, 0, 8, 10, 10, 10, 10]
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
		return parseToString([
			accountId,
			a.source_name,
			tdProcess,
			dealNum(cash.realized_pnl),
			dealNum(cash.unrealized_pnl),
			toDecimal(total) + '' || '--',
			toDecimal(cash.avail) + '' || '--',
        ].slice(0, this.headers.length), calcuHeaderWidth(this.headers, this.columnWidth))
    })
	this.table.setItems(accountListData)
}

export default AccountTable()