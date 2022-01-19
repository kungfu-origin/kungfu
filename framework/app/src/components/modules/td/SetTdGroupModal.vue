<script lang="ts" setup>
import { setTdGroup } from '@kungfu-trader/kungfu-js-api/actions';
import { getIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  getInstrumentTypeColor,
  useAllKfConfigData,
  useExtConfigsRelated,
  useModalVisible,
  useTdGroups,
} from '@renderer/assets/methods/uiUtils';
import { AntTreeNodeDropEvent, DataNode } from 'ant-design-vue/lib/tree';
import { computed, ComputedRef, getCurrentInstance, toRaw, toRefs } from 'vue';

const props = withDefaults(
  defineProps<{
    visible: boolean;
  }>(),
  {
    visible: false,
  },
);

defineEmits<{
  (e: 'update:visible', visible: boolean): void;
  (e: 'close'): void;
}>();

const app = getCurrentInstance();
const { modalVisible, closeModal } = useModalVisible(props.visible);
const { extTypeMap } = useExtConfigsRelated();
const tdGroup = useTdGroups();
const { td } = toRefs(useAllKfConfigData());

const tdTreeData: ComputedRef<DataNode[]> = computed(() => {
  const tdGroupResolved: Record<string, DataNode> = {};
  const tdResolved: DataNode[] = [];
  const markedNameToTdGroup: Record<string, KungfuApi.KfExtraLocation> = {};
  [...tdGroup.data, ...td.value].forEach((item) => {
    if ('children' in item) {
      markedNameToTdGroup[item.name] = { ...item };
      tdGroupResolved[item.name] = {
        ...transformKfConfigToDataNode(item),
        children: [],
      };
      return;
    }

    const accountId = `${item.group}_${item.name}`;
    const targetGroupNames = Object.keys(markedNameToTdGroup).filter((name) => {
      const tdGroup = markedNameToTdGroup[name];
      return (tdGroup.children || []).includes(accountId);
    });
    if (targetGroupNames.length) {
      const targetGroupName = targetGroupNames[0];
      tdGroupResolved[targetGroupName].children?.push(
        transformKfConfigToDataNode(item),
      );
      return;
    }

    tdResolved.push(transformKfConfigToDataNode(item));
  });
  return [...Object.values(tdGroupResolved), ...tdResolved];
});

function transformKfConfigToDataNode(
  target: KungfuApi.KfConfig | KungfuApi.KfExtraLocation,
): DataNode {
  return {
    name: target.name,
    title: target.name,
    category: target.category,
    group: target.group,
    key: `${target.category}_${target.group}_${target.name}`,
  };
}

function isGroup(node: DataNode): KungfuApi.KfExtraLocation | null {
  if ('children' in node) {
    const name = node.name;
    const targetGroups = tdGroup.data.filter((item) => {
      return item.name === name;
    });
    if (targetGroups.length) {
      return targetGroups[0];
    } else {
      return null;
    }
  } else {
    return null;
  }
}

function isInGroup(node: DataNode): KungfuApi.KfExtraLocation | null {
  const accountId = getIdByKfLocation(
    node as unknown as KungfuApi.KfExtraLocation,
  );
  const targetGroups = tdGroup.data.filter((item) => {
    return item.children.includes(accountId);
  });

  if (targetGroups.length) {
    return targetGroups[0];
  } else {
    return null;
  }
}

function handleDrop(info: AntTreeNodeDropEvent) {
  const { dragNode, node, dropPosition, dropToGap } = info;

  if (isGroup(dragNode)) {
    return;
  }

  const oldGroup = isInGroup(dragNode);
  const targetAccountId = getIdByKfLocation(
    dragNode as unknown as KungfuApi.KfExtraLocation,
  );

  if (oldGroup) {
    const oldTargetIndex = oldGroup.children.indexOf(targetAccountId);
    oldGroup.children.splice(oldTargetIndex, 1); //remove from old
  }

  if (isGroup(node)) {
    const groupIndex = tdGroup.data.findIndex(
      (group) => node.name === group.name,
    );
    //from group to bottom
    if (!dropToGap || dropPosition !== groupIndex + 1) {
      isGroup(node).children.push(targetAccountId);
    }
  }

  const newGroup = isInGroup(node);
  if (newGroup) {
    newGroup.children.push(targetAccountId);
  }

  setTdGroup(toRaw(tdGroup.data)).then(() => {
    if (app?.proxy) {
      app?.proxy.$useGlobalStore().setTdGroups();
    }
  });
}

getInstrumentTypeColor;
</script>
<template>
  <a-modal
    :width="420"
    class="set-td-group-modal"
    v-model:visible="modalVisible"
    title="账户分组设置"
    :destroyOnClose="true"
    :footer="null"
    @cancel="closeModal"
  >
    <a-tree
      v-if="tdTreeData.length"
      draggable
      :tree-data="tdTreeData"
      :blockNode="true"
      :defaultExpandAll="true"
      @drop="handleDrop"
    >
      <template #title="{ dataRef }">
        <div v-if="dataRef.category === 'td'">
          <a-tag
            class="kf-td-tree-tag"
            size="small"
            :color="getInstrumentTypeColor(extTypeMap[dataRef.group])"
          >
            {{ dataRef.group }}
          </a-tag>
          {{ dataRef.name }}
        </div>
        <div v-else-if="dataRef.category === 'tdGroup'">
          <a-tag size="small" color="#FAAD14">账户组</a-tag>
          {{ dataRef.name }}
        </div>
      </template>
    </a-tree>
  </a-modal>
</template>
<style lang="less">
.set-td-group-modal {
  .kf-td-tree-tag {
    display: inline-block;
  }
}
</style>
