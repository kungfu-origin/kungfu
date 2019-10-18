import Dashboard from '@/assets/components/Dashboard';
import MessageBox from '@/assets/components/MessageBox';
import { DEFAULT_PADDING, TABLE_BASE_OPTIONS, parseToString} from '@/assets/scripts/utils';
import { processListObservable, LogsAndWatcherConcatObservable, switchProcess } from '@/assets/scripts/actions';
import { throttleInsert } from '__gUtils/busiUtils';
const blessed = require('blessed');
const moment = require('moment');

const WIDTH_LEFT_PANEL = 45;


export class MonitorDashboard extends Dashboard {
    screen: any;

    boards: any;

    globalData: {
        processList: any[],
        logList: any[]
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
            logList: []
        }

        this.logWatchers = [];

        this.init();
    }

    init(){
        const t = this;
        t.initProcessList();
        t.initLog();
        t.initBoxInfo();
        t.initMessage();
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

    initLog(){
        this.boards.mergedLogs = blessed.list({
            ...TABLE_BASE_OPTIONS,
            label: ' Merged Logs ',
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

    initMessage(){
        const t = this;
        t.boards.message = MessageBox(t.screen)
    }

    initLoader() {
        const t = this;
        t.boards.loader = blessed.loading({
            parent: t.screen,
            top: '100%-5',
            left: '100%-40',
            height: 5,
            align: 'left',
            valign: 'center',
            width: 40,
            tags: true,
            hidden: true,
            border: 'line'
        });
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
            //log the first time get Log
            if(!t.globalData.processList || !t.globalData.processList.length) t._getLogs(processList)

            //processList
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
            if (i === -1) i = 1;
            const nameKey: string = boards[i];
            t.boards[nameKey].focus();
        });
    
        t.screen.key(['escape', 'q', 'C-c'], (ch: any, key: string) => {
            t.screen.destroy();
            process.exit(0);
        });	

        t.boards.processList.key(['enter'], () => {
            const selectedIndex: number = t.boards.processList.selected;
            switchProcess(t.globalData.processList[selectedIndex], t.boards.message)
        })
    }
 
    _getLogs(processList: ProcessListItem[]){
        const t = this;
        const processIds = processList.map((p: ProcessListItem) => p.processId)
        const throttleInsertLogs = throttleInsert(1600);
        t.boards.loader.load('Loading the logs, please wait...')
        LogsAndWatcherConcatObservable(processIds).subscribe((l: any) => {
            t.boards.loader.stop()

            //obserable
            if(typeof l === 'string') {
                throttleInsertLogs(l).then((logList: string[] | boolean) => {
                    if(!logList) return;
                    t.globalData.logList = t.globalData.logList.concat(logList)
                    const len = t.globalData.logList.length;
                    t.globalData.logList = t.globalData.logList.slice(len < 2000 ? 0 : (len - 2000))
                    t.boards.mergedLogs.setItems(t.globalData.logList)
                    if(!t.boards.mergedLogs.focused) {
                        t.boards.mergedLogs.select(t.globalData.logList.length - 1)
                        t.boards.mergedLogs.setScrollPerc(100)
                    }
                })
                t.screen.render();
                return
            }

            //get 
            t.globalData.logList = l;
            t.boards.mergedLogs.setItems(t.globalData.logList)
            t.screen.render();
        })
    }

}

export default () => {
    new MonitorDashboard()
}
