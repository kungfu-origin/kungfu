import { Subject } from 'rxjs';
import {
    startGetKungfuWatcherStep,
    startUpdateKungfuWatcherQuotes,
} from './watcher';
export const tradingDataSubject = new Subject<KungfuApi.Watcher>();

startGetKungfuWatcherStep(2000, (watcher) => {
    tradingDataSubject.next(watcher);
});

startUpdateKungfuWatcherQuotes(3000);
