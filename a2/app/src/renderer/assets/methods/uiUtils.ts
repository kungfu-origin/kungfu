import fs from 'fs-extra';
import { Component, SetupContext, Ref, ref, watch } from 'vue';
import path from 'path';
import { APP_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { buildObjectFromArray } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { StateStatusConfig } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';

export interface KfUIComponent {
    name: string;
    component: Component;
}

// this utils file is only for ui components
export const getUIComponents = (): {
    [prop: string]: KfUIComponent[keyof KfUIComponent] | null | undefined;
    [prop: number]: KfUIComponent[keyof KfUIComponent] | null | undefined;
} => {
    const componentsDir = path.join(APP_DIR, 'components');
    const files = fs.readdirSync(componentsDir);
    const jsFiles = files.filter((file) => file.includes('.js'));
    const existedNames: string[] = [];
    const uicList = jsFiles
        .map((file: string): KfUIComponent | null => {
            const fullpath = path.join(componentsDir, file);
            const uic = global.require(fullpath).default as Component;
            const { name } = uic;
            if (!name) {
                console.error('no name property in components' + fullpath);
                return null;
            }

            if (existedNames.includes(name)) {
                console.error(`component name ${name} is existed, ${fullpath}`);
            }
            return {
                name,
                component: uic,
            };
        })
        .filter((item: KfUIComponent | null) => !!item);

    return buildObjectFromArray<KfUIComponent | null>(
        uicList,
        'name',
        'component',
    );
};

export const getStateStatusInfoByStatusType = (
    statusType: string | number,
): string => {
    // return StateStatusConfig[statusType];
    StateStatusConfig;
    statusType;
    return '';
};

export const modalVisibleComposition = (
    props: { visible: boolean },
    context: SetupContext,
): { modalVisible: Ref<boolean>; closeModal: () => void } => {
    const modalVisible = ref<boolean>(props.visible);
    watch(
        () => props.visible,
        (visible) => {
            modalVisible.value = visible;
        },
    );
    const closeModal = () => {
        context.emit('update:visible', false);
    };

    return {
        modalVisible,
        closeModal,
    };
};
