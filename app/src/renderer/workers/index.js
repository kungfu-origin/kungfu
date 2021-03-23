import ResolveInstrumentsWorker from 'worker-loader!@/assets/workers/resolveInstruments.js'
import CalcMarketDataAvgVolumeWorker from 'worker-loader!@/assets/workers/calcMarketDataAvgVolume.js'

export default {
    insturmnetWorker: new ResolveInstrumentsWorker(),
    calcMarketDataAvgVolumeWorker: new CalcMarketDataAvgVolumeWorker()
}