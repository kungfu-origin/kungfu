import Table from '../public/Table';
import { calcuHeaderWidth, parseToString } from './utils';


function MdTable(){
    if (!(this instanceof MdTable)) {
        return new MdTable();
    }
    Table.call(this);
    this.headers = ['Source', 'Account', 'Status']
}

MdTable.prototype = new Table();

MdTable.prototype = new Table();

/**
 * @param  {Object} accountData
 * @param  {Object} processStatus
 */
MdTable.prototype.refresh = function(mdData){
    mdData = Object.values(mdData || {})
	const mdListData = mdData.map(m => parseToString([
        m.source_name,
        m.account_id.toAccountId(),
        m.status
    ], calcuHeaderWidth(this.headers)))
	this.table.setItems(mdListData)
}

export default MdTable()

