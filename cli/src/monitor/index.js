import blessed  from 'blessed';
import colors from 'colors';
import path from 'path';
import { Tail } from 'tail';

import { LOG_DIR } from '__gConfig/pathConfig';
import Dashboard from '../public/Dashboard';
import { DEFAULT_PADDING, TABLE_BASE_OPTIONS, parseAccountList, dealStatus, switchMd, switchTd, switchStrategy, switchMaster, parseToString, dealLog } from '../public/utils';
import { getAccountList } from '@/io/account.js';
import { getStrategyList } from '@/io/strategy.js';
import { dealLogMessage, getLog } from '@/assets/js/utils';
import { listProcessStatus } from '__gUtils/processUtils';

const WIDTH_LEFT_PANEL = 30;

class MonitorDashboard extends Dashboard {
    constructor(...args){
        super(args)
        this.screen.title = 'Account Dashboard';
        this.globalData = {
            processes: [],
            accountData: {},
            mdData: {},
            strategyList: [],
            processStatus: {},
            logList: [],
        }

        this.logWatchers = [];
    }

    init(){
        const t = this;
        t.initMasterProcess();
        t.initProcessList();
        t.initLog();
        // t.initMetaData();
        t.initBoxInfo();
        t.initLoader();
        t.screen.render();
        t.bindEvent();
        t.getProcessStatus().then(() => t.getLogs());
    }


    initMasterProcess(){
        this.masterProcess = blessed.list({
            ...TABLE_BASE_OPTIONS,
            label: ' Master Processes ',
            parent: this.screen,
            top: '0',
            left: '0',
            width: WIDTH_LEFT_PANEL + '%',
            height: 5,
            padding: DEFAULT_PADDING,
            items: [" Master  -- "],
            interactive: true,
            style: {
                ...TABLE_BASE_OPTIONS.style,
                selected: {
					bold: true,
					bg: 'blue',
                },
            }
        })

        this.masterProcess.focus();
    }

    initProcessList(){
        this.processList = blessed.list({
            ...TABLE_BASE_OPTIONS,
            label: ' Processes ',
            parent: this.screen,
            padding: DEFAULT_PADDING,
            top: 5,
            left: '0',
            interactive: false,
            width: WIDTH_LEFT_PANEL + '%',
            height: '95%-6',
            style: {
                ...TABLE_BASE_OPTIONS.style,
                selected: {
					bold: true,
					bg: 'blue',
                }
            }
        })
    }

    initLog(){
        this.mergedLogs = blessed.log({
            ...TABLE_BASE_OPTIONS,
            label: ' Merged Logs ',
            parent: this.screen,
            top: '0',
            left: WIDTH_LEFT_PANEL + '%',
            width: 100 - WIDTH_LEFT_PANEL + '%',
            height: '95%-1',
            padding: DEFAULT_PADDING,
            style: {
                ...TABLE_BASE_OPTIONS.style,
                selected: {
					bold: true,
                }
            }
        })
    }

    initBoxInfo() {
        this.boxInfo = blessed.text({
            content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit | enter: process-switch',
            parent: this.screen,		
            left: '0%',
            top: '94%',
            width: '100%',
            height: '6%',
            valign: 'middle',
            tags: true
        });	
    }

    initLoader(){
        const t = this;
        t.message = blessed.message({
            parent: t.screen,
            top: '100%-5',
            left: '100%-30',
            height: 5,
            align: 'left',
            valign: 'center',
            width: 30,
            tags: true,
            hidden: true,
            border: 'line'
        });
    }

    bindEvent(){
        const t = this;
        let i = 0;
        let boards = ['masterProcess', 'processList', 'mergedLogs','metaData'];
        t.screen.key(['left', 'right'], (ch, key) => {
            (key.name === 'left') ? i-- : i++;
            if (i === 4) i = 0;
            if (i === -1) i = 3;
            t[boards[i]].focus();
        });
    
        t.screen.key(['escape', 'q', 'C-c'], (ch, key) => {
            t.screen.destroy();
            process.exit(0);
        });	

        t.masterProcess.on('focus', () => t.masterProcess.interactive = true)
        t.masterProcess.on('blur', () => t.masterProcess.interactive = false)
        t.masterProcess.key(['enter'], (ch, key) => {
            switchMaster(t.globalData.processStatus).then(() => {t.message.log(' operation sucess!', 2)})
        })
        t.processList.on('focus', () => t.processList.interactive = true)
        t.processList.on('blur', () => t.processList.interactive = false)
        t.processList.key(['enter'], () => {
            const selectedIndex = t.processList.selected;
            const currentProcess = t.globalData.processes[selectedIndex];
            switch(currentProcess.type) {
                case 'md':
                    switchMd(currentProcess, t.globalData.processStatus).then(() => {t.message.log(' operation sucess!', 2)})
                    break;
                case 'td':
                    switchTd(currentProcess, t.globalData.processStatus).then(() => {t.message.log(' operation sucess!', 2)})
                    break;
                case 'strategy':
                    switchStrategy(currentProcess, t.globalData.processStatus).then(() => {t.message.log(' operation sucess!', 2)})
                    break;
            }
        })
    }

    refresh(){
        const { processStatus } = this.globalData;
        //master
        this.masterProcess.setItems([parseToString(['Master', dealStatus(processStatus.master)], [23, 8, 'auto'])])
        //processes
        const processes = this._dealProcessList();
        this.processList.setItems(processes);;
        this.screen.render();
    }

    getData(){
        const t = this;
        const getAccountDataPromise = getAccountList().then(accountList => {
            const { mdData, accountData } = parseAccountList(t.globalData, accountList)
            t.globalData = {
                ...t.globalData,
                mdData,
                accountData
            }
        })

        const getStrategyPromise = getStrategyList().then(strategyList => {
            t.globalData.strategyList = strategyList
        })

        var timer = null
        const promiseAll = () => {
            clearTimeout(timer)
            Promise.all([getAccountDataPromise, getStrategyPromise]).finally(() => {
                t.refresh();
                timer = setTimeout(promiseAll, 3000)
            })
        }
        promiseAll();
    }

    getProcessStatus(){
        const t = this;
        //循环获取processStatus
        var listProcessTimer;
        const startGetProcessStatus = () => {
            clearTimeout(listProcessTimer)
            return listProcessStatus()
            .then(res => {
                t._diffOnlineProcess(res, t.globalData.processStatus);
                t.globalData.processStatus = Object.freeze(res);
                t.refresh();
                return res;
            })
            .catch(err => console.error(err))
            .finally(() => listProcessTimer = setTimeout(startGetProcessStatus, 1000))
        }
        return startGetProcessStatus()
    }

    getLogs(){
        const t = this;
        const processStatus = t.globalData.processStatus;
        const aliveProcesses = Object.keys(processStatus || {}).filter(k => processStatus[k] === 'online' && k !== 'master');
        const logPromises = aliveProcesses.map(p => {
            const logPath = path.join(LOG_DIR, `${p}.log`);
            return getLog(logPath)
        })

        Promise.all(logPromises).then(logList => {
            let mergedLogs = [];
            logList.map(({list}) => {
                mergedLogs = [...mergedLogs, ...list]
            })
            
            //sort
            mergedLogs.sort((a, b) => {
                if(a.updateTime > b.updateTime) return 1
                else if(a.updateTime < b.updateTime) return -1
                else return 0
            })
            mergedLogs.forEach(l => {
                t.mergedLogs.add(dealLog(l))
            })
        })
    }

    _dealProcessList(){
        const t = this;
        const { mdData, accountData, processStatus, strategyList } = t.globalData;
        const mdDataValues = Object.values(mdData || {})
        const mdList = mdDataValues.map(m => {
            const mdProcess = `md_${m.source_name}`;
            const type = colors.magenta('Md');
            const status = dealStatus(processStatus[mdProcess]);
            return parseToString([
                type,
                mdProcess,
                status
            ], [5, 15, 8])
        })

        const tdDataValues = Object.values(accountData || {});
        const tdList = tdDataValues.map(a => {
            const tdProcess = `td_${a.account_id}`;
            const type = colors.cyan('Td');
            const status = dealStatus(processStatus[tdProcess]);
            return parseToString([
                type,
                tdProcess,
                status
            ], [5, 15, 8])
        })
        
        const stratList = strategyList.map(s => {
            const stratProcess = s.strategy_id;
            const type = colors.yellow('Strat');
            const status = dealStatus(processStatus[stratProcess]);
            return parseToString([
                type,
                stratProcess,
                status
            ], [5, 15, 8])
        })

        t.globalData.processes = [
            ...mdDataValues.map(m => ({...m, type: "md"})), 
            ...tdDataValues.map(td => ({...td, type: 'td'})), 
            ...strategyList.map(s => ({...s, type: 'strategy'}))
        ];
        return [...mdList, ...tdList, ...stratList]
    }

    _diffOnlineProcess(processStatus, oldProcessStatus){
        const t = this;
        const aliveProcesses = Object.keys(processStatus)
        .filter(p => processStatus[p] === 'online')
        .sort((a, b) => {
           if(a > b) return 1;
           else if(a < b) return -1;
           else return 0
        })

        const oldAliveProcesses = Object.keys(oldProcessStatus)
        .filter(p => oldProcessStatus[p] === 'online')
        .sort((a, b) => {
           if(a > b) return 1;
           else if(a < b) return -1;
           else return 0
        })

        if(aliveProcesses.join('') !== oldAliveProcesses.join('')) t._triggerWatchLogFiles(aliveProcesses)
    }

    _triggerWatchLogFiles(processes){
        const t = this;
        t.logWatchers.forEach(w => {
            w.unwatch()
            w = null;
        })
        t.logWatchers = [];

        processes.forEach(p => {
            const logPath = path.join(LOG_DIR, `${p}.log`);
            const watcher = new Tail(logPath);
            watcher.watch();
            watcher.on('line', line => {
                const logData = dealLogMessage(line);
                logData.forEach(l => t.mergedLogs.add(dealLog(l)))
            })
            watcher.on('error', err => {
                watcher.unwatch();
                watcher = null;
            })
            t.logWatchers.push(watcher);
        })
    }
}

export default () => {
    const monitorDashboard = new MonitorDashboard()
    monitorDashboard.init();
    monitorDashboard.render();
    monitorDashboard.getData();
}

