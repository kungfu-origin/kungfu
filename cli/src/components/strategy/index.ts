import posTable from '@/assets/components/PosTable';
import orderTable from '@/assets/components/OrderTable';
import tradeTable from '@/assets/components/TradeTable';
import Dashboard from '@/assets/components/Dashboard';

import { parseToString, TABLE_BASE_OPTIONS, DEFAULT_PADDING, dealNum } from '@/assets/scripts/utils';
import { tradingDataObservale, processListObservable, switchProcess } from '@/assets/scripts/actions';

import { logger } from '__gUtils/logUtils';

const blessed = require('blessed');

// 定义全局变量
const WIDTH_LEFT_PANEL = 60;

class StrategyDashboard extends Dashboard {
	strategyId: string
	screen: any;
	globalData: {
		processList: ProcessListItem[];
	};
	boards: any;

	constructor(strategyId: string){
		super()
		this.screen.title = 'Strategy Dashboard';
		this.strategyId = strategyId;
		this.globalData = {
			processList: []
		};

		this.boards = {};
		this.init()
	}

	init(){
		const t = this;
		t.initStrategyProcessTable();
		t.initAssetsTable();
		t.initPosTable();
		t.initOrderList();
		t.initTradeList();
		t.initBoxInfo();
		t.initLoader();
		t.screen.render();
		t.bindEvent();
		t.bindData();
	}
	
	initStrategyProcessTable(){
		const t = this;
		this.boards.strategyTable = blessed.list({
            ...TABLE_BASE_OPTIONS,
            label: ' Strategy Trading Engines ',
            parent: this.screen,
            padding: DEFAULT_PADDING,
            top: '0',
            left: '0',
            interactive: true,
			mouse: false,			
            width: 45,
            height: '40%',
            style: {
                ...TABLE_BASE_OPTIONS.style,
                selected: {
					bold: true,
					bg: 'blue',
                }
            }
        })
        this.boards.strategyTable.focus()
	}

	initAssetsTable(){
		const t = this;
		this.boards.assetTable = blessed.list({
            ...TABLE_BASE_OPTIONS,
            label: ' Assets ',
            parent: this.screen,
            padding: DEFAULT_PADDING,
            top: '0',
            left: 45,
            interactive: false,
			mouse: false,			
            width: 100 - WIDTH_LEFT_PANEL + 5 + '%-45',
            height: '40%',
            style: {
                ...TABLE_BASE_OPTIONS.style,
                selected: {
					bold: true,
					bg: 'blue',
                }
            }
        })
	}
	
	initPosTable(){
		const t = this;
		t.boards.posTable = posTable()
		t.boards.posTable.build({
			label: ' Positions ',
			parent: t.screen,
			top: '0',
			left: 100 - WIDTH_LEFT_PANEL + 5 + '%',
			width: WIDTH_LEFT_PANEL - 5 + '%',
			height: '40%',
		})
	}
	
	initOrderList(){
		const t = this;
		t.boards.orderTable = orderTable('account')
		t.boards.orderTable.build({
			label: ' Orders Records (3days)',
			parent: t.screen,
			top: '40%',
			width: WIDTH_LEFT_PANEL - 5 + '%',
			height: '55%'
		});
	}
	
	initTradeList(){
		const t = this;
		t.boards.tradeTable = tradeTable('account')
		t.boards.tradeTable.build({
			label: ' Trades Records (3days)',
			parent: t.screen,
			top: '40%',
			left: WIDTH_LEFT_PANEL - 5 + '%',
			width: 100 - WIDTH_LEFT_PANEL + 5 + '%',
			height: '55%'
		});
	}
	
	initBoxInfo() {
		const t = this;
		t.boards.boxInfo = blessed.text({
			content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit | Enter: process-switch',
			parent: t.screen,		
			left: '0%',
			top: '95%',
			width: '100%',
			height: '6%',
			valign: 'middle',
			tags: true
		});	
	}

	initLoader(){
        const t = this;
        t.boards.message = blessed.message({
            parent: t.screen,
            top: '100%-5',
            left: '100%-40',
            height: 5,
            align: 'left',
            valign: 'center',
            width: 40,
            tags: true,
            hidden: true,
            border: 'line',
            style: {
                bg: 'blue'
            }
        });
    }
	
	bindEvent() {
		const t = this;
		let i = 0;
		let boards = ['strategyTable', 'assetTable', 'posTable', 'orderTable', 'tradeTable'];
		t.screen.key(['left', 'right'], (ch: any, key: any) => {
			(key.name === 'left') ? i-- : i++;
			if (i === 5) i = 0;
			if (i === -1) i = 4;
			t.boards[boards[i]].focus();
		});
	
		t.screen.key(['escape', 'q', 'C-c'], function(ch: any, key: any) {
			t.screen.destroy();
			process.exit(0);
		});	

		t.boards.strategyTable.key(['enter'], () => {
			const selectedIndex: number = t.boards.strategyTable.selected;
            switchProcess(t.globalData.processList[selectedIndex], t.boards.message)
        });
	}


	bindData() {
		const t = this;

		tradingDataObservale('strategy', t.strategyId).subscribe((tradingData: any) => {
			const type = tradingData[0];
			const data = tradingData[1];
			switch (type) {
				case 'order':
					t.boards.orderTable.setItems(data);
					break;
				case 'trade':
					t.boards.tradeTable.setItems(data);
					break;
				case 'pos':
					t.boards.posTable.setItems(data);
					break;
				case 'asset':
					const assetData = Object.entries(data)
						.map((kvPair: any[]) => {
							let key = kvPair[0]
							let val = kvPair[1]
							let theFirst = key[0]
							key = theFirst.toUpperCase() + key.slice(1)
							if(['UnRealizedPnl', 'RealizedPnl'].indexOf(key) !== -1) {
								val = dealNum(+val)
							}
							return parseToString(
								[key, val],
								[16, 16]
							)
						})
					t.boards.assetTable.setItems(assetData)
					break
			}
			t.screen.render()
		})

		processListObservable().subscribe((processList: any) => {
			processList = processList
				.filter((proc: ProcessListItem) => {
					if(proc.type === 'strategy') return true
					else if(proc.type === 'main') return true
					else return false
			})
			t.globalData.processList = processList;
			const processListResolve = processList
                .map((proc: ProcessListItem) => parseToString([
                    proc.typeName,
                    proc.processName,
                    proc.statusName
                ], [5, 15, 8]))
            t.boards.strategyTable.setItems(processListResolve);
            t.screen.render();
        })
	}
}
	


export default (strategyId: string) => new StrategyDashboard(strategyId)




