import { getCurrentInstance } from 'vue';

export interface CategoryRegisterProps {
    name: string;
    commonData: KungfuApi.KfTradeValueCommonData;

    order?: {
        getter: (
            orders: KungfuApi.DataTable<KungfuApi.Order>,
            kfLocation: KungfuApi.KfExtraLocation,
        ) => KungfuApi.Order[];
    };

    trade?: {
        getter: (
            trades: KungfuApi.DataTable<KungfuApi.Trade>,
            kfLocation: KungfuApi.KfExtraLocation,
        ) => KungfuApi.Trade[];
    };
    position?: {
        getter: (
            positions: KungfuApi.DataTable<KungfuApi.Position>,
            kfLocation: KungfuApi.KfExtraLocation,
        ) => KungfuApi.Position[];
    };
}

export class GlobalCategoryRegister {
    globalRegisterdCategories: Record<string, CategoryRegisterProps>;

    constructor() {
        this.globalRegisterdCategories = {};
    }

    register(props: CategoryRegisterProps): void {
        if (this.globalRegisterdCategories[props.name]) {
            console.warn(`${props.name} has been registed`);
        }

        this.globalRegisterdCategories[props.name] = props;
    }

    getExtraCategory(): Record<string, KungfuApi.KfTradeValueCommonData> {
        return Object.keys(this.globalRegisterdCategories).reduce(
            (extraCategoryData, key) => {
                extraCategoryData[key] =
                    this.globalRegisterdCategories[key].commonData;
                return extraCategoryData;
            },
            {} as Record<string, KungfuApi.KfTradeValueCommonData>,
        );
    }
}

export const useExtraCategory = (): {
    getExtraCategoryData: (
        source:
            | KungfuApi.DataTable<KungfuApi.Order>
            | KungfuApi.DataTable<KungfuApi.Trade>
            | KungfuApi.DataTable<KungfuApi.Position>,
        kfLocation: KungfuApi.KfExtraLocation,
        type: 'order' | 'trade' | 'position',
    ) => TradingDataItem[];
} => {
    const app = getCurrentInstance();

    const getExtraCategoryData = (
        source:
            | KungfuApi.DataTable<KungfuApi.Order>
            | KungfuApi.DataTable<KungfuApi.Trade>
            | KungfuApi.DataTable<KungfuApi.Position>,
        kfLocation: KungfuApi.KfExtraLocation,
        type: 'order' | 'trade' | 'position',
    ): TradingDataItem[] => {
        if (app?.proxy) {
            const targetCategories =
                app?.proxy?.$globalCategoryRegister?.globalRegisterdCategories[
                    kfLocation.category
                ] || ({} as CategoryRegisterProps);

            const defaultFunc = () => [] as TradingDataItem[];
            const defaulFuncData = {
                getter: defaultFunc,
            };
            const dealDataFuncData = targetCategories[type] || defaulFuncData;
            const dealDataFunc = dealDataFuncData.getter;

            return dealDataFunc(
                source as KungfuApi.DataTable<KungfuApi.Order> &
                    KungfuApi.DataTable<KungfuApi.Trade> &
                    KungfuApi.DataTable<KungfuApi.Position>,
                kfLocation,
            );
        }

        return [];
    };

    return {
        getExtraCategoryData,
    };
};
