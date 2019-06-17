import Table from './Table';
import { calcuHeaderWidth, parseToString, dealStatus } from '../utils';


function MdTable(){
    if (!(this instanceof MdTable)) {
        return new MdTable();
    }
    Table.call(this);
    this.headers = ['Source', 'Account', 'Status']
	this.columnWidth = [0, 10, 8]
}

MdTable.prototype = new Table();

MdTable.prototype = new Table();

/**
 * @param  {Object} accountData
 * @param  {Object} processStatus
 */
MdTable.prototype.refresh = function(mdData, processStatus){
    mdData = Object.values(mdData || {})
	const mdListData = mdData.map(m => parseToString([
        m.source_name,
        m.account_id.toAccountId(),
        dealStatus(processStatus[`md_${m.source_name}`])
    ].slice(0, this.headers.length), calcuHeaderWidth(this.headers, this.columnWidth)))
	this.table.setItems(mdListData)
}

export default MdTable()

