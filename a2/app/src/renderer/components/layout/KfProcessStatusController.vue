<script lang="ts" setup>
import { ClusterOutlined } from '@ant-design/icons-vue';
import {
    KfCategoryTypes,
    KfConfig,
} from '@kungfu-trader/kungfu-js-api/typings';
import KfProcessStatus from '@renderer/components/public/KfProcessStatus.vue';
import { ref, toRefs, watch } from 'vue';
import {
    KfCategory,
    SystemProcessName,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    getExtConfigsRelated,
    getInstrumentTypeColor,
    getAllKfConfigData,
    getProcessStatusDetailData,
    handleSwitchProcessStatus,
    openLogView,
} from '@renderer/assets/methods/kfUiUtils';
import { FileTextOutlined } from '@ant-design/icons-vue';
import {
    getIfProcessOnline,
    getProcessIdByKfLocation,
    getPropertyFromProcessStatusDetailDataByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { message, notification } from 'ant-design-vue';

const processControllerBoardVisible = ref<boolean>(false);
const categoryList: KfCategoryTypes[] = ['system', 'td', 'md', 'strategy'];
const allKfConfigData = getAllKfConfigData();
const { processStatusData, processStatusDetailData } = toRefs(
    getProcessStatusDetailData(),
);
const { extTypeMap } = getExtConfigsRelated();

let hasAlertMasterStop = false;
let hasAlertLedgerStop = false;

watch(processStatusData, (newPSD, oldPSD) => {
    if (newPSD.master !== 'online' && oldPSD.master === 'online') {
        if (!hasAlertMasterStop) {
            hasAlertMasterStop = true;
            notification.error({
                message: '主控进程 master 中断',
                description:
                    '主控进程负责策略进程间通信与资源配置, 请重启功夫交易系统',
                duration: 10,
            });
        }
    }

    if (newPSD.ledger !== 'online' && oldPSD.ledger === 'online') {
        if (!hasAlertLedgerStop) {
            hasAlertLedgerStop = true;
            notification.error({
                message: '计算服务 ledger 中断',
                description: '计算服务负责持仓跟资金计算, 请重启功夫交易系统',
                duration: 10,
            });
        }
    }
});

function handleOpenProcessControllerBoard(): void {
    processControllerBoardVisible.value = true;
}

function handleOpenLogview(config: KfConfig): Promise<Electron.BrowserWindow> {
    const hideloading = message.loading('正在打开窗口');
    return openLogView(getProcessIdByKfLocation(config)).finally(() => {
        hideloading();
    });
}
</script>

<template>
    <div
        class="kf-process-status-controller__warp"
        @click="handleOpenProcessControllerBoard"
    >
        <ClusterOutlined style="font-size: 14px; padding-right: 4px" />
        <span class="title">控制中心</span>

        <a-drawer
            v-model:visible="processControllerBoardVisible"
            :width="650"
            class="kf-process-status-controller-board__warp"
            title="控制中心"
            placement="right"
        >
            <div
                class="process-controller-item"
                v-for="category in categoryList"
                :key="category"
            >
                <template v-if="allKfConfigData[category].length">
                    <div class="kf-config-list">
                        <div
                            class="kf-config-item"
                            v-for="config in allKfConfigData[category]"
                        >
                            <div class="process-info">
                                <div class="category info-item">
                                    <a-tag :color="KfCategory[category].color">
                                        {{ KfCategory[category].name }}
                                    </a-tag>
                                </div>
                                <div
                                    class="process-id info-item"
                                    v-if="config.category === 'system'"
                                >
                                    {{
                                        (SystemProcessName[config.name] || {})
                                            .name || ''
                                    }}
                                </div>
                                <div
                                    class="process-id info-item"
                                    v-else-if="config.category !== 'strategy'"
                                >
                                    <a-tag
                                        v-if="
                                            config.category === 'td' ||
                                            config.category === 'md'
                                        "
                                        :color="
                                            getInstrumentTypeColor(
                                                extTypeMap[config.group],
                                            )
                                        "
                                    >
                                        {{ config.group }}
                                    </a-tag>
                                    {{ config.name }}
                                </div>
                                <div class="process-id info-item" v-else>
                                    {{ config.name }}
                                </div>
                            </div>
                            <div class="state-status">
                                <KfProcessStatus
                                    :status-name="
                                        getPropertyFromProcessStatusDetailDataByKfLocation(
                                            processStatusDetailData,
                                            config,
                                        ).status
                                    "
                                ></KfProcessStatus>
                            </div>
                            <div class="switch">
                                <a-switch
                                    size="small"
                                    :checked="
                                        getIfProcessOnline(
                                            processStatusData,
                                            getProcessIdByKfLocation(config),
                                        )
                                    "
                                    @click="(checked: boolean) => handleSwitchProcessStatus(checked, config)"
                                ></a-switch>
                            </div>
                            <div class="cpu">
                                CPU:
                                {{
                                    getPropertyFromProcessStatusDetailDataByKfLocation(
                                        processStatusDetailData,
                                        config,
                                    ).cpu + '%'
                                }}
                            </div>
                            <div class="memory">
                                MEM:
                                {{
                                    getPropertyFromProcessStatusDetailDataByKfLocation(
                                        processStatusDetailData,
                                        config,
                                    ).memory + 'M'
                                }}
                            </div>
                            <div class="actions kf-actions__warp">
                                <FileTextOutlined
                                    @click="handleOpenLogview(config)"
                                    style="font-size: 14px"
                                ></FileTextOutlined>
                            </div>
                        </div>
                    </div>
                </template>
            </div>
        </a-drawer>
    </div>
</template>

<style lang="less">
@import '@renderer/assets/less/variables.less';
.kf-process-status-controller__warp {
    float: right;
    height: 100%;
    padding: 0 8px;
    display: flex;
    align-items: center;
    cursor: pointer;

    &:hover {
        background: @item-active-bg;
        color: @primary-color;
    }

    .title {
        font-size: 12px;
    }
}

.kf-process-status-controller-board__warp {
    .process-controller-item {
        margin-bottom: 24px;

        .category-title {
            font-size: 18px;
            font-weight: bold;
            padding-bottom: 8px;
        }

        .kf-config-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 8px;

            .process-info {
                flex: 1;
                display: flex;
                justify-content: flex-start;
                align-items: center;
                margin-right: 8px;

                .info-item {
                    margin-right: 8px;

                    &.category {
                        width: 70px;
                    }
                }
            }

            .state-status {
                width: 80px;
            }

            .switch {
                width: 40px;
            }

            .cpu {
                width: 80px;
            }

            .memory {
                width: 120px;
            }

            .actions {
                width: 40px;
            }
        }
    }
}
</style>
