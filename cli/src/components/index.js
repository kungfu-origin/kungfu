import strategy from './strategy/index'
import account from './account/index'
import monitor from './monitor/index'

export const monitPrompt =  (monit) => {  
    if(monit === 'strategy') return strategy();
    if(monit === 'account') return account();
    if(monit === 'all') return monitor();
}