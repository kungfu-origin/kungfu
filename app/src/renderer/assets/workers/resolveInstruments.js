
import { resolveInstruments } from '__gUtils/busiUtils';

self.addEventListener('message', (e) => {
    const instruments = e.data || [];
    const instrumentsResolved = resolveInstruments(instruments)
    postMessage(instrumentsResolved)
})