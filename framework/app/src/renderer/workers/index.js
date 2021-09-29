import CalcMarketDataAvgVolumeWorker from 'worker-loader!__renderer/assets/workers/calcMarketDataAvgVolume.js';
import GetLogWorker from 'worker-loader!__renderer/assets/workers/getLog.js';
import DealInstruments from 'worker-loader!__renderer/assets/workers/dealInstruments.js';

export default {
    calcMarketDataAvgVolumeWorker: new CalcMarketDataAvgVolumeWorker(),
    getLogWorker: new GetLogWorker(),
    dealInstruments: new DealInstruments(),
}