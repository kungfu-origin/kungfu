<script lang="ts" setup>
import { ClusterOutlined } from '@ant-design/icons-vue';
import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings';
import KfProcessStatus from '@renderer/components/public/KfProcessStatus.vue';
import { ref } from 'vue';
import {
    KfCategory,
    SystemProcessName,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    getExtConfigsRelated,
    getInstrumentTypeColor,
    getKfLocationData,
    getProcessStatusDetailData,
    handleSwitchProcessStatus,
} from '@renderer/assets/methods/uiUtils';
import { FileTextOutlined } from '@ant-design/icons-vue';
import {
    getIfProcessOnline,
    getProcessIdByKfLocation,
    getPropertyFromProcessStatusDetailDataByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

const processControllerBoardVisible = ref<boolean>(false);
const categoryList: KfCategoryTypes[] = ['system', 'td', 'md', 'strategy'];
const allKfLocations = getKfLocationData();
const { processStatusData, processStatusDetailData } =
    getProcessStatusDetailData();
const { extTypeMap } = getExtConfigsRelated();

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
                <template v-if="allKfLocations[category].length">
                    <div class="kf-location-list">
                        <div
                            class="kf-location-item"
                            v-for="location in allKfLocations[category]"
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
                                        (SystemProcessName[location.name] || {})
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
                                            getInstrumentTypeColor(
                                                extTypeMap[location.group],
                                            )
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
                                    <KfProcessStatus
                                        :status-name="
                                            getPropertyFromProcessStatusDetailDataByKfLocation(
                                                processStatusDetailData.value,
                                                location,
                                            ).status
                                        "
                                    ></KfProcessStatus>
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
                                    @click="(checked: boolean) => handleSwitchProcessStatus(checked, location)"
                                ></a-switch>
                            </div>
                            <div class="cpu">
                                CPU:
                                {{
                                    getPropertyFromProcessStatusDetailDataByKfLocation(
                                        processStatusDetailData.value,
                                        location,
                                    ).cpu + '%'
                                }}
                            </div>
                            <div class="memory">
                                MEM:
                                {{
                                    getPropertyFromProcessStatusDetailDataByKfLocation(
                                        processStatusDetailData.value,
                                        location,
                                    ).memory + 'M'
                                }}
                            </div>
                            <div class="actions kf-actions__warp">
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

                    // &.process-id {
                    // width: 100px;
                    // }
                }
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
