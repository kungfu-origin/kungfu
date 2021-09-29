<template>
    <div class="kf-admin-manager__warp">
        <div class="kf-admin-item">
            <TdAccount title="账户设置"></TdAccount>
        </div>
        <div class="kf-admin-item">
            <Strategy title="策略设置"></Strategy>
        </div>
    </div>
</template>

<script>

import TdAccount from '__renderer/components/Account/components/TdAccount';
import Strategy from '__renderer/components/Strategy/Strategy';

export default {

    components: {
        TdAccount,
        Strategy
    }
}

</script>

<style lang="scss">
@import "__renderer/assets/scss/skin.scss";

.kf-admin-manager__warp {
    padding: 8px;
    box-sizing: border-box;
    
    .kf-admin-item {
        height: 300px;
        
        .title {
            font-size: 24px;
            font-weight: 600;
            color: $icon;
        }
    }
    
}

</style>