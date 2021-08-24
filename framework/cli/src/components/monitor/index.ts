import Dashboard from '@kungfu-trader/kungfu-cli/assets/components/Dashboard';
import MessageBox from '@kungfu-trader/kungfu-cli/assets/components/MessageBox';
import Loading from '@kungfu-trader/kungfu-cli/assets/components/Loading';
import { DEFAULT_PADDING, TABLE_BASE_OPTIONS, parseToString } from '@kungfu-trader/kungfu-cli/assets/scripts/utils';
import { switchProcess, processListObservable } from '@kungfu-trader/kungfu-cli/assets/scripts/actions/processActions';
import { getLogSubject, getLogInterval, splitStrByLength } from '@kungfu-trader/kungfu-cli/assets/scripts/actions/logActions';
import { debounce } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { logger } from '@kungfu-trader/kungfu-uicc/utils/logUtils';


const blessed = require('blessed');
const moment = require('moment');
const colors = require('colors');

const WIDTH_LEFT_PANEL = 70;

export class MonitorDashboard extends Dashboard {
    screen: any;

    boards: any;

    globalData: {
        processList: any[],
        logList: any[]
    }

    oldGetLogIntervalTimer: any;

    constructor() {
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
            logList: []
        }

        this.oldGetLogIntervalTimer = null;

        this.init();
    }

    init() {
        const t = this;
        t.initProcessList();
        t.initLog();
        t.initBoxInfo();
        t.initMessage();
        t.initLoader();
        t.screen.render();

        t.bindEvent();
        t.bindData();
        t._bindLogMessage();
    }


    initProcessList() {
        this.boards.processList = blessed.list({
            ...TABLE_BASE_OPTIONS,
            label: ' Processes ',
            parent: this.screen,
            padding: DEFAULT_PADDING,
            top: '0',
            left: '0',
            interactive: true,
            mouse: false,
            width: WIDTH_LEFT_PANEL,
            height: '95%',
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

    initLog() {
        this.boards.mergedLogs = blessed.list({
            ...TABLE_BASE_OPTIONS,
            label: ' Logs ',
            parent: this.screen,
            top: '0',
            left: WIDTH_LEFT_PANEL + 1,
            width: `100%-${WIDTH_LEFT_PANEL + 1}`,
            height: '95%',
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

    initMessage() {
        const t = this;
        t.boards.message = MessageBox(t.screen)
    }

    initLoader() {
        const t = this;
        t.boards.loader = Loading(t.screen)
    }

    initBoxInfo() {
        const t = this;
        t.boards.boxInfo = blessed.text({
            content: ' left/right: switch boards | up/down/mouse: scroll | Ctrl/Cmd-C: exit | Enter: start/stop process',
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
        processListObservable().subscribe((processList: any) => {


            //processList
            t.globalData.processList = processList;
            const processListResolved = processList
                .map((proc: ProcessListItem) => {
                    return parseToString([
                        proc.typeName,
                        proc.processName,
                        proc.statusName,
                        "Cpu " + proc.monit.cpu,
                        "MEM " + proc.monit.memory
                    ], [5, 15, 10, 10, 10])
            })
            t.boards.processList.setItems(processListResolved);
            t.screen.render();

            if (t.oldGetLogIntervalTimer === null) {
                const curProcessItem = processList[0]
                t._getLogs(curProcessItem)
            }
        })
    }

    bindEvent() {
        const t = this;
        let i: number = 0;
        let boards = ['processList', 'mergedLogs'];
        t.screen.key(['left', 'right'], (ch: string, key: any) => {
            (key.name === 'left') ? i-- : i++;
            if (i === 2) i = 0;
            if (i === -1) i = 1;
            const nameKey: string = boards[i];
            t.boards[nameKey].focus();
        });

        t.screen.key(['C-c'], (ch: any, key: any) => {
            logger.info('qqq', ch, key)

            const keyName = key.full;
            if (!keyName) return;            
            if ((keyName === 'escape') || (keyName === 'C-c') || (keyName === 'q')) {
                t.screen.destroy();
                process.exit(0);
            }
        });

        t.boards.processList.key(['enter'], () => {
            const selectedIndex: number = t.boards.processList.selected;
            const curProcessItem = t.globalData.processList[selectedIndex];
            switchProcess(curProcessItem, t.boards.message, t.boards.loader)
        })

        t.boards.processList.key(['up', 'down'], debounce((ch: string, key: any) => {

            logger.info('up / down', ch, key)

            const selectedIndex: number = t.boards.processList.selected;
            const curProcessItem = t.globalData.processList[selectedIndex];
            t._getLogs(curProcessItem)
        }, 1000))
    }

    _getLogs(processItem: ProcessListItem) {
        if (!processItem) return;
        const t = this;
        const processId = processItem.processId;

        if (t.oldGetLogIntervalTimer) t.oldGetLogIntervalTimer.clearLoop(); //unsubscribe the old
        t.boards.mergedLogs.setItems([]) //clear
        t.boards.mergedLogs.setLabel(` Logs (${processId}) `)
        t.globalData.logList = [];
        t.boards.mergedLogs.setItems(t.globalData.logList)
        t.oldGetLogIntervalTimer = getLogInterval(processId);        
    }

    _bindLogMessage () {
        const t = this;
        const boardWidth = this.boards.mergedLogs.width;
        getLogSubject().subscribe((logList: any) => {
            let list = logList.list || [];
            let listResolved: any = []

            list
                .sort((a: any, b: any) => moment(a.updateTime).valueOf() - moment(b.updateTime).valueOf())
                .forEach((l: any) => {
                    const message = l.message;
                    const isCritical = l.isCritical;

                    if (message.length < boardWidth) {
                        listResolved.push(message)
                    } else {
                        splitStrByLength(message, boardWidth).forEach((splitLine: string) => {
                            if (isCritical) {
                                listResolved.push(colors.red(splitLine))
                            } else {
                                listResolved.push(splitLine)
                            }
                        })
                    }
                })

            //get   
            t.globalData.logList = listResolved;
    
            if (!t.boards.mergedLogs.focused) {
                t.boards.mergedLogs.setItems(t.globalData.logList)
                t.boards.mergedLogs.select(t.globalData.logList.length - 1)
                t.boards.mergedLogs.setScrollPerc(100)
            }

            t.screen.render();
        })   
    }

}

export default () => {
    new MonitorDashboard()
}
