import strategy from './strategy/index'
import account from './account/index'
import monitor from './monitor/index'
import { initDB } from '../../app/src/main/base';

initDB()

strategy()
export default {
    strategy,
    account,
    monitor
}