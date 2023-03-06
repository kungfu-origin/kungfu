<template>
  <a-popover
    v-model:visible="popoverVisible"
    :overlayStyle="{ width: '200px' }"
    trigger="click"
  >
    <template #title>
      <div class="kf-update-controller-title__wrap">
        <span>{{ $t('globalSettingConfig.update') }}</span>
        <CloseOutlined @click="popoverVisible = false" />
      </div>
    </template>
    <template #contents>
      <template v-if="hasNewVersion">
        <template v-if="downloadStarted">
          <a-progress :precent="process" strokeColor="#FAAD14"></a-progress>
          <div v-if="process === 100">
            <span>{{ $t('globalSettingConfig.downloaded') }}</span>
            <a-button
              style="margin-left: 8px"
              type="link"
              @click="handleQuitAndInstall"
            >
              {{ $t('globalSettingConfig.to_install') }}
            </a-button>
          </div>
        </template>
        <template v-else>
          <span>
            {{ $t('globalSettingConfig.new_version') + ': ' + newVersion }}
          </span>
          <a-button type="link" @click="handleToStartDownload">
            {{ $t('globalSettingConfig.start_download') }}
          </a-button>
        </template>
      </template>
      <template v-else>
        <div>
          {{ $t('globalSettingConfig.current_version') + ':' }}
        </div>
        <div>
          {{
            currentVersion +
            ' (' +
            $t('globalSettingConfig.already_latest_version') +
            ')'
          }}
        </div>
      </template>
    </template>
    <div>
      <interaction-outlined />
      <span>{{ $t('globalSettingConfig.update') }}</span>
    </div>
  </a-popover>
</template>

<script lang="ts" setup>
import { InteractionOutlined, CloseOutlined } from '@ant-design/icons-vue';
import { useUpdateVersion } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';

const {
  popoverVisible,
  newVersion,
  currentVersion,
  hasNewVersion,
  downloadStarted,
  process,
  handleToStartDownload,
  handleQuitAndInstall,
} = useUpdateVersion();
</script>

<style lang="less">
.kf-update-controller-title__wrap {
  width: 100%;
  display: flex;
  justify-content: space-between;
  padding: 0 4px;
}
</style>
