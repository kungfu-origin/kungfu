<template>
  <div class="kf-no-data__warp">
    <div class="kf-no-data__content">
      <img svg-inline :src="logo" alt="" />
      <p>{{ txt }}</p>
    </div>
  </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
  props: {
    txt: {
      type: String,
    },
  },

  setup() {
    return {
      logo: require('@renderer/assets/svg/LOGO.svg'),
    };
  },
});
</script>

<style lang="less">
.kf-no-data__warp {
  height: 100%;
  width: 100%;
  .kf-no-data__content {
    img {
      max-height: 20%;
      padding-top: 30px;
      filter: invert(0) sepia(0%) saturate(0%) hue-rotate(0) brightness(30%)
        contrast(129%);
    }
  }
}
</style>
