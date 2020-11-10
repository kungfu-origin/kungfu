
export const ifMasterLedgerRunning = (state) => {
    const processStatus = state.processStatus || {};
    const masterIsRunning = processStatus.master === 'online';
    const ledgerIsRunning = processStatus.ledger === 'online';

    return masterIsRunning && ledgerIsRunning
}