// import { getUIComponents } from '@renderer/assets/methods/uiUtils';
import { App, defineAsyncComponent } from 'vue';

export const useComponenets = (app: App<Element>): void => {
    app.component(
        '持仓',
        defineAsyncComponent(
            () => import('@root/src/components/modules/pos/Pos.vue'),
        ),
    );

    app.component(
        '持仓汇总',
        defineAsyncComponent(
            () =>
                import('@root/src/components/modules/posGlobal/PosGlobal.vue'),
        ),
    );

    app.component(
        '委托记录',
        defineAsyncComponent(
            () => import('@root/src/components/modules/order/Order.vue'),
        ),
    );

    app.component(
        '成交记录',
        defineAsyncComponent(
            () => import('@root/src/components/modules/trade/Trade.vue'),
        ),
    );

    app.component(
        '交易账户',
        defineAsyncComponent(
            () => import('@root/src/components/modules/td/Td.vue'),
        ),
    );

    app.component(
        '行情源',
        defineAsyncComponent(
            () => import('@root/src/components/modules/md/Md.vue'),
        ),
    );

    app.component(
        '策略进程',
        defineAsyncComponent(
            () => import('@root/src/components/modules/strategy/Strategy.vue'),
        ),
    );

    app.component(
        '行情订阅',
        defineAsyncComponent(
            () =>
                import(
                    '@root/src/components/modules/marketdata/MarketData.vue'
                ),
        ),
    );

    app.component(
        '深度行情',
        defineAsyncComponent(
            () =>
                import('@root/src/components/modules/orderBook/OrderBook.vue'),
        ),
    );

    app.component(
        '下单面板',
        defineAsyncComponent(
            () =>
                import(
                    '@root/src/components/modules/makeOrder/MakeOrderDashboard.vue'
                ),
        ),
    );

    app.component(
        '套利指令',
        defineAsyncComponent(
            () =>
                import(
                    '@root/src/components/modules/futureArbitrage/FutureArbitrage.vue'
                ),
        ),
    );

    app.config.globalProperties.$registedKfUIComponents = [
        '持仓',
        '持仓汇总',
        '委托记录',
        '成交记录',
        '交易账户',
        '行情源',
        '策略进程',
        '行情订阅',
        '深度行情',
        '下单面板',
        '套利指令',
    ];

    // if (process.env.NODE_ENV === 'production') {
    //     const uics = getUIComponents();
    //     Object.keys(uics).forEach((key) => {
    //         app.component(key, uics[key] as Component);
    //     });
    //     app.config.globalProperties.$registedKfUIComponents = Object.keys(uics);
    // }
};
