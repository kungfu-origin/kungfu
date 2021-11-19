<template>
    <div class="app__warp">
        <router-view />
    </div>
</template>

<style lang="less">
#app {
    width: 100%;
    height: 100%;
    font-family: Avenir, Helvetica, Arial, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;
    color: #2c3e50;

    .app__warp {
        height: 100%;
        width: 100%;
    }
}
</style>
