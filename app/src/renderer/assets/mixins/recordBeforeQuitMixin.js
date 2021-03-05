
import fse from 'fs-extra';
import { KF_DATASET_MARKETDATA_DIR } from '__gConfig/pathConfig';
import { watcher } from '__io/kungfu/watcher';
import { logger } from '__gUtils/logUtils';

export default {

    data () {
        
        return {
            recordBeforeQuitLoading: false
        }
    },

    methods: {
        recordBeforeQuit () {
            this.recordBeforeQuitLoading = true;
            return Promise.all([
                this.recordQuote(),
                this.timer()
            ]).finally(() => {
                this.recordBeforeQuitLoading = false;
            })
        },

        recordQuote () {
            watcher.step()
            logger.info(watcher.ledger.Quote)
            const Quote = watcher.ledger.Quote;
            
            return Promise.resolve(false)
        },

        
    }
}



