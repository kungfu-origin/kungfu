
<script>
import makeOrderUIMixin from '@/components/Base/makeOrder/makeOrderUIMixin.vue';
import makeOrderMixin from '@/components/Base/makeOrder/js/makeOrderMixin';
import makeOrderIPCMixin from '@/components/Base/makeOrder/js/makeOrderIPCMixin';


export default {
    mixins: [ makeOrderMixin, makeOrderIPCMixin, makeOrderUIMixin ]
}

</script>
