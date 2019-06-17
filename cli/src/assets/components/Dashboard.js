import blessed  from 'blessed';
import { DEFAULT_PADDING, TABLE_BASE_OPTIONS } from '../utils';

class Dashboard {
    constructor(){
        this.screen = blessed.screen({
            smartCSR: true,
            fullUnicode: true,
            autoPadding: true,
        });

        this.screen.title = 'Account Dashboard';
    }

    init(){
        console.error('init must be cover')
    }

    refresh(){
        console.error('refresh must be cover')
    }

    bindEvent(){
        console.error('bindEvent must be cover')
    }
    
    render(){
        const t = this;
        t.screen.render();
        // async refresh of the ui
        setInterval(function () {
            t.screen.render();
        }, 300);
    }

    initMessage(){
        const t = this;
        t.message = blessed.message({
            parent: t.screen,
            top: '100%-5',
            left: '100%-30',
            height: 5,
            padding: DEFAULT_PADDING,
            align: 'left',
            valign: 'center',
            width: 30,
            tags: true,
            hidden: true,
            border: 'line'
        });
    }

    
    
}

export default Dashboard