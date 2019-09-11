import { LOG_DIR } from '__gConfig/pathConfig';
import Dashboard from '@/assets/components/Dashboard';
import { DEFAULT_PADDING, TABLE_BASE_OPTIONS, dealStatus, parseToString, dealLog, getStatus } from '@/assets/scripts/utils';
import { processListObservable } from '@/assets/scripts/actions';
import { dealLogMessage, getLog } from '__gUtils/busiUtils';
import { addFile } from '__gUtils/fileUtils';
import { switchMaster, switchLedger } from '__io/actions/base';
import { switchTd, switchMd } from '__io/actions/account';
import { switchStrategy } from '__io/actions/strategy';
import { startMaster } from '../../../../app/shared/utils/processUtils';

const blessed = require('blessed');
const colors = require('colors');
const path = require('path');
const { Tail } = require('tail');

const WIDTH_LEFT_PANEL = 30;



export class MonitorDashboard extends Dashboard {
    screen: any;

    boards: any;

    globalData: {
        processList: any[]
    }
  
    logWatchers: any[];
    
    constructor(){
        super()
        this.screen.title = 'Processes Dashboard';

        this.boards = {
            processList: null,
            mergedLogs: null,
            message: null,
            boxInfo: null
        }

        this.globalData = {
            processList: [],
        }

        this.logWatchers = [];

        this.init();
    }

    init(){
        const t = this;
        t.initProcessList();
        t.initLog();
        t.initBoxInfo();
        t.initLoader();
        t.screen.render();

        t.bindEvent();
        t.bindData();

        
    }


    initProcessList(){
        this.boards.processList = blessed.list({
            ...TABLE_BASE_OPTIONS,
            label: ' Processes ',
            parent: this.screen,
            padding: DEFAULT_PADDING,
            top: '0',
            left: '0',
            interactive: true,
			mouse: false,			
            width: WIDTH_LEFT_PANEL + '%',
            height: '95%-1',
            style: {
                ...TABLE_BASE_OPTIONS.style,
                selected: {
					bold: true,
					bg: 'blue',
                }
            }
        })
        this.boards.processList.focus()
    }

    initLog(){
        this.boards.mergedLogs = blessed.log({
            ...TABLE_BASE_OPTIONS,
            label: ' Merged Logs ',
            parent: this.screen,
            top: '0',
            left: WIDTH_LEFT_PANEL + '%',
            width: 100 - WIDTH_LEFT_PANEL + '%',
            height: '95%-1',
            padding: DEFAULT_PADDING,
			mouse: true,
            style: {
                ...TABLE_BASE_OPTIONS.style,
                selected: {
					bold: true,
                }
            }
        })
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

    bindData() {
        const t = this;
        processListObservable.subscribe((processList: any) => {
            t.globalData.processList = processList;
            const processListResolve = processList
                .map((proc: ProcessListItem) => parseToString([
                    proc.typeName,
                    proc.processName,
                    proc.statusName
                ], [5, 15, 8]))
            t.boards.processList.setItems(processListResolve);
            t.screen.render();
        })
    }

    bindEvent(){
        const t = this;
        let i: number = 0;
        let boards = ['processList', 'mergedLogs'];
        t.screen.key(['left', 'right'], (ch: string, key: any) => {
            (key.name === 'left') ? i-- : i++;
            if (i === 2) i = 0;
            if (i === -1) i = 2;
            const nameKey: string = boards[i];
            t.boards[nameKey].focus();
        });
    
        t.screen.key(['escape', 'q', 'C-c'], (ch: any, key: string) => {
            t.screen.destroy();
            process.exit(0);
        });	

        t.boards.processList.key(['enter'], () => {
            const selectedIndex: number = t.boards.processList.selected;
            t._switchProcess(selectedIndex)
        })
    }

    _switchProcess(selectedIndex: number){
        const t = this;
        const proc = t.globalData.processList[selectedIndex];
        const status = proc.status === 'online';
        const startOrStop = !!status ? 'Stop' : 'Start';
        switch(proc.type) {
            case 'main':
                if (proc.processId === 'master') {
                    switchMaster(!status)
                    .then(() => {t.boards.message.log(`${startOrStop} Master process sucessfully!`, 2)})
                    .catch((err: Error) => {})
                }
                else if(proc.processId === 'ledger') {
                    switchLedger(!status)
                    .then(() => {t.boards.message.log(`${startOrStop} Ledger process sucessfully!`, 2)})
                    .catch((err: Error) => {})
                } 
                break
            case 'md':
                switchMd(proc, !status)
                .then(() => {t.boards.message.log(`${startOrStop} MD process sucessfully!`, 2)})
                .catch((err: Error) => {})
                break
            case 'td':
                switchTd(proc, !status)
                .then(() => {t.boards.message.log(`${startOrStop} TD process sucessfully!`, 2)})
                .catch((err: Error) => {})
                break;
            case 'strategy':
                switchTd(proc.processId, !status)
                .then(() => {t.boards.message.log(`${startOrStop} Strategy process sucessfully!`, 2)})
                .catch((err: Error) => {})
                break;
        }
    }

 

    // getLogs(){
    //     const t = this;
    //     const processStatus = t.globalData.processStatus;
    //     const aliveProcesses = Object.keys(processStatus || {}).filter(k => processStatus[k] === 'online' && k !== 'master');
    //     const logPromises = aliveProcesses.map(p => {
    //         const logPath = path.join(LOG_DIR, `${p}.log`);
    //         addFile('', logPath, 'file')		
    //         return getLog(logPath)
    //     })

    //     Promise.all(logPromises).then(logList => {
    //         let mergedLogs = [];
    //         logList.map(({list}) => {
    //             mergedLogs = [...mergedLogs, ...list]
    //         })
            
    //         //sort
    //         mergedLogs.sort((a, b) => {
    //             if(a.updateTime > b.updateTime) return 1
    //             else if(a.updateTime < b.updateTime) return -1
    //             else return 0
    //         })
    //         mergedLogs.forEach(l => {
    //             t.mergedLogs.add(dealLog(l))
    //         })
    //     })
    // }



    // _triggerWatchLogFiles(processes){
    //     const t = this;
    //     t.logWatchers.forEach(w => {
    //         w.unwatch()
    //         w = null;
    //     })
    //     t.logWatchers = [];

    //     processes.forEach(p => {
    //         const logPath = path.join(LOG_DIR, `${p}.log`);
	// 	    addFile('', logPath, 'file')		
    //         const watcher = new Tail(logPath);
    //         watcher.watch();
    //         watcher.on('line', line => {
    //             const logData = dealLogMessage(line);
    //             logData.forEach(l => t.mergedLogs.add(dealLog(l)))
    //         })
    //         watcher.on('error', err => {
    //             watcher.unwatch();
    //             watcher = null;
    //         })
    //         t.logWatchers.push(watcher);
    //     })
    // }
}

export default () => {
    const monitorDashboard = new MonitorDashboard()
    // monitorDashboard.render();
    // monitorDashboard.getData();
}

