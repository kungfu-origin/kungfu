import ResolveInstrumentsWorker from 'worker-loader!@/assets/workers/resolveInstruments.js'
import CalcMarketDataAvgVolumeWorker from 'worker-loader!@/assets/workers/calcMarketDataAvgVolume.js'
import GetLogWorker from 'worker-loader!@/assets/workers/getLog.js'

export default {
    insturmnetWorker: new ResolveInstrumentsWorker(),
    calcMarketDataAvgVolumeWorker: new CalcMarketDataAvgVolumeWorker(),
    getLogWorker: new GetLogWorker()
}