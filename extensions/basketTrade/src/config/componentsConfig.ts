import BasketSetting from '../components/modules/BasketSetting/Index.vue';
import BasketDetail from '../components/modules/BasketDetail/Index.vue';
import BasketOrder from '../components/modules/BasketOrder/Index.vue';
import Order from '../components/modules/Order/Index.vue';
import Trade from '../components/modules/Trade/Index.vue';

export const componentsMap = {
  ['篮子设置']: BasketSetting,
  ['篮子详情']: BasketDetail,
  ['篮子委托记录']: BasketOrder,
  ['委托记录']: Order,
  ['成交记录']: Trade,
};
