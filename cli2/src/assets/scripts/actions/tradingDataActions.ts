import { buildTradingDataPipe } from '__io/kungfu/tradingData';
import { map } from 'rxjs/operators';

export const tradingDataObservale = (type: string, processId: string) => {
    return buildTradingDataPipe(type).pipe(
        map((data: any) => {
            console.log(data)
            return data
        })
    )
}
