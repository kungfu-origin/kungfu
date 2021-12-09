<script lang="ts" setup>
import { ClusterOutlined } from '@ant-design/icons-vue';
import {
    InstrumentTypeEnum,
    KfCategoryTypes,
    KfConfig,
    KfExtConfigs,
    KfLocation,
    KfTradeValueCommonData,
} from '@kungfu-trader/kungfu-js-api/typings';
import KfStateStatus from '@renderer/components/public/KfStateStatus.vue';
import { storeToRefs } from 'pinia';
import { ref, reactive, computed, getCurrentInstance, onMounted } from 'vue';
import {
    InstrumentType,
    KfCategory,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    buildExtTypeMap,
    getIfProcessOnline,
} from '@renderer/assets/methods/uiUtils';
import { FileTextOutlined } from '@ant-design/icons-vue';
import { Pm2ProcessStatusData } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { getProcessIdByKfLocation } from '@kungfu-trader/kungfu-js-api/kungfu/utils';

const app = getCurrentInstance();

const processControllerBoardVisible = ref<boolean>(false);
const categoryList: KfCategoryTypes[] = ['system', 'td', 'md', 'strategy'];

const systemList: KfLocation[] = [
    ...(process.env.NODE_ENV === 'development'
        ? ([
              {
                  category: 'system',
                  group: 'service',
                  name: 'archive',
                  mode: 'LIVE',
              },
          ] as KfLocation[])
        : []),
    {
        category: 'system',
        group: 'master',
        name: 'master',
        mode: 'LIVE',
    },
    {
        category: 'system',
        group: 'service',
        name: 'ledger',
        mode: 'LIVE',
    },
];

const processData = ref<Record<KfCategoryTypes, KfLocation[]>>({
    system: systemList,

    md: [],
    td: [],
    strategy: [],
});

const extConfigs = reactive<{ value: KfExtConfigs }>({
    value: {},
});
const extTypeMap = computed(() => buildExtTypeMap(extConfigs.value, 'td'));

const systemProcessName: Record<string, KfTradeValueCommonData> = {
    master: { name: '主控进程' },
    ledger: { name: '计算服务' },
    archive: { name: '归档服务' },
};

const processStatusData = reactive<{ value: Pm2ProcessStatusData }>({
    value: {},
});

onMounted(() => {
    if (app?.proxy) {
        const store = storeToRefs(app?.proxy.$useGlobalStore());
        processData.value.md = store.mdList as KfConfig[];
        processData.value.td = store.tdList as KfConfig[];
        processData.value.strategy = store.strategyList as KfConfig[];
        extConfigs.value = store.extConfigs as KfExtConfigs;
        processStatusData.value = store.processStatus as Pm2ProcessStatusData;
    }
});

function handleOpenProcessControllerBoard() {
    processControllerBoardVisible.value = true;
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
            class="kf-process-status-controller-board__warp"
            title="控制中心"
            placement="right"
        >
            <div
                class="process-controller-item"
                v-for="category in categoryList"
                :key="category"
            >
                <template v-if="processData[category].length">
                    <div class="kf-location-list">
                        <div
                            class="kf-location-item"
                            v-for="location in processData[category]"
                        >
                            <div class="process-info">
                                <div class="category info-item">
                                    <a-tag :color="KfCategory[category].color">
                                        {{ KfCategory[category].name }}
                                    </a-tag>
                                </div>
                                <div
                                    class="process-id info-item"
                                    v-if="location.category === 'system'"
                                >
                                    {{
                                        (systemProcessName[location.name] || {})
                                            .name || ''
                                    }}
                                </div>
                                <div
                                    class="process-id info-item"
                                    v-else-if="location.category !== 'strategy'"
                                >
                                    <a-tag
                                        v-if="
                                            location.category === 'td' ||
                                            location.category === 'md'
                                        "
                                        :color="
                                            InstrumentType[
                                                InstrumentTypeEnum[
                                                    extTypeMap[location.group]
                                                ]
                                            ].color
                                        "
                                    >
                                        {{ location.group }}
                                    </a-tag>
                                    {{ location.name }}
                                </div>
                                <div class="process-id info-item" v-else>
                                    {{ location.name }}
                                </div>
                                <div class="state-status info-item">
                                    <KfStateStatus
                                        :status-name="
                                            processStatusData.value[
                                                getProcessIdByKfLocation(
                                                    location,
                                                )
                                            ] || 'Unknown'
                                        "
                                    ></KfStateStatus>
                                </div>
                            </div>
                            <div class="switch">
                                <a-switch
                                    size="small"
                                    :checked="
                                        getIfProcessOnline(
                                            processStatusData.value,
                                            getProcessIdByKfLocation(location),
                                        )
                                    "
                                ></a-switch>
                            </div>
                            <div class="actions">
                                <FileTextOutlined
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

        .kf-location-item {
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

                    &.process-id {
                        width: 100px;
                    }
                }
            }

            .switch {
                width: 40px;
            }

            .actions {
                width: 20px;
            }
        }
    }
}
</style>
