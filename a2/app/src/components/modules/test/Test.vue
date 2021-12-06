<script setup lang="ts">
import type { StrategyRow } from '@kungfu-trader/kungfu-js-api/typings/index';

</script>

<template>
    <a-table ref="table" size="small" :pagination="false">
        <template #bodyCell="props">
        {{ (props.record as StrategyRow) }}
        </template>
    </a-table>
</template>
