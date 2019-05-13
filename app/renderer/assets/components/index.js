import TrNoData from './tr/TrNoData';
import TableHeader from './tr/TableHeader';
import TableHeaderItem from './tr/TableHeaderItem';
import TrStatus from './tr/TrStatus';
import TrTabTop from './tr/TrTabTop.vue';
import TrTabTopItem from './tr/TrTabTopItem.vue';
import TrFooterItem from './tr/TrFooterIterm.vue';
import TrMenu from './tr/TrMenu.vue';
import TrMenuItem from './tr/TrMenuItem.vue';
import TrPnl from './tr/TrPnl.vue';
import TrDashboard from './tr/TrDashboard.vue';
import TrDashboardHeaderItem from './tr/TrDashboardHeaderItem.vue';
import TrTable from './tr/TrTable.vue';
import TrSearchInput from './tr/TrSearchInput.vue';

import MainContent from '@/components/Layout/MainContent';
import PopFrame from '@/components/Layout/PopFrame';


const components = [
    TrNoData,
    TableHeader,
    TableHeaderItem,
    MainContent,
    PopFrame,
    TrStatus,
    TrTabTop,
    TrTabTopItem,
    TrFooterItem,
    TrMenu,
    TrMenuItem,
    TrDashboard,
    TrDashboardHeaderItem,
    TrPnl,
    TrTable,
    TrSearchInput
]

export default function (Vue) {
    components.map(component => {
        Vue.component(component.name, component)
    })
}
