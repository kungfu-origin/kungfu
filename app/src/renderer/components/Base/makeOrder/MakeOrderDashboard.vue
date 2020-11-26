

<script>

import makeOrderUIMixin from '@/components/Base/makeOrder/makeOrderUIMixin.vue';
import makeOrderMixin from '@/components/Base/makeOrder/js/makeOrderMixin';
import makeOrderCoreMixin from '@/components/Base/makeOrder/js/makeOrderCoreMixin';


export default {
    name: 'MakeOrderDashboard',
    mixins: [ makeOrderMixin, makeOrderCoreMixin, makeOrderUIMixin ],

}
</script>