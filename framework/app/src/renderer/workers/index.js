import CalcMarketDataAvgVolumeWorker from 'worker-loader!@/assets/workers/calcMarketDataAvgVolume.js';
import GetLogWorker from 'worker-loader!@/assets/workers/getLog.js';
import DealInstruments from 'worker-loader!@/assets/workers/dealInstruments.js';

export default {
    calcMarketDataAvgVolumeWorker: new CalcMarketDataAvgVolumeWorker(),
    getLogWorker: new GetLogWorker(),
    dealInstruments: new DealInstruments(),
}