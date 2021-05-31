import { resolveInstruments } from '__gUtils/busiUtils';

self.addEventListener('message', e => {
    const { instruments } = e.data || {};
    const instrumentsResolved = resolveInstruments(instruments);
    self.postMessage({
        updateTime: new Date().getTime(),
        instruments: instrumentsResolved
    })
})