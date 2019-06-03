import blessed  from 'blessed';
import { TABLE_BASE_OPTIONS } from '../public/utils';

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
    
    render = function(){
        const t = this;
        t.screen.render();
        // async refresh of the ui
        setInterval(function () {
            t.screen.render();
        }, 300);
    }
    
    
}

export default Dashboard