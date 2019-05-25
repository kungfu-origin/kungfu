
#include "../instrument/instrument.h"
#include <kungfu/wingchun/constant.h>
#include <cassert>
#include <fmt/format.h>

int main()
{
    std::string db_file = fmt::format(FUTURE_INSTRUMENT_DB_FILE_FORMAT, "/shared/kungfu");
    kungfu::FutureInstrumentStorage future_instrument_storage(db_file);
    kungfu::FutureInstrument input = {};
    strcpy(input.instrument_id, "a");
    strcpy(input.exchange_id, "b");
    input.instrument_type = '1';
    strcpy(input.product_id, "");
    input.contract_multiplier = 10;
    input.price_tick = 1;
    strcpy(input.open_date, "c");
    strcpy(input.create_date, "d");
    strcpy(input.expire_date, "e");
    input.delivery_year = 3;
    input.delivery_month = 2;
    input.is_trading = true;
    input.long_margin_ratio = 0.2;
    input.short_margin_ratio = 0.3;
    future_instrument_storage.add_future_instrument(input);

    std::shared_ptr<kungfu::InstrumentManager> instrument_manager =
        kungfu::InstrumentManager::get_instrument_manager();
    std::vector<const kungfu::FutureInstrument*> v = instrument_manager->get_future_instruments();
    assert(v.size() == 1);
    const kungfu::FutureInstrument* result = instrument_manager->get_future_instrument("a", "b");
    assert(strcmp(result->instrument_id, "a") == 0);
    assert(strcmp(result->exchange_id, "b") == 0);
    assert(result->instrument_type == '1');
    assert(strcmp(result->product_id, "") == 0);
    assert(result->contract_multiplier == 10);
    assert(result->price_tick == 1);
    assert(strcmp(result->open_date, "c") == 0);
    assert(strcmp(result->create_date, "d") == 0);
    assert(strcmp(result->expire_date, "e") == 0);
    assert(result->delivery_year == 3);
    assert(result->delivery_month == 2);
    assert(result->is_trading == true);
    assert(result->long_margin_ratio == 0.2);
    assert(result->short_margin_ratio == 0.3);
/*
    kungfu::CommissionStorage commission_storage(fmt::format(COMMISSION_DB_FILE_FORMAT, "/shared/kungfu", "acc_1"));
    kungfu::InstrumentCommissionRate input1 = {
        "instrument_1", "exchange_1", '2', "acc_1", "broker_1", '0', 1, 2, 3, 4};
    kungfu::InstrumentCommissionRate input2 = {
        "instrument_2", "exchange_2", '2', "acc_1", "broker_2", '1', 10, 20, 30, 40};
    commission_storage.add_commission(input1);
    commission_storage.add_commission(input2);
    kungfu::CommissionManager commission_manager("xtp_acc1");
    const kungfu::InstrumentCommissionRate* result1 =
        commission_manager.get_commission_rate('2', "instrument_1", "exchange_1");
    assert(strcmp(result1->instrument_id, "instrument_1") == 0);
    assert(strcmp(result1->exchange_id, "exchange_1") == 0);
    assert(result1->instrument_type == '2');
    assert(strcmp(result1->account_id, "acc_1") == 0);
    assert(strcmp(result1->broker_id, "broker_1") == 0);
    assert(result1->mode == '0');
    assert(result1->open_ratio == 1);
    assert(result1->close_ratio == 2);
    assert(result1->close_today_ratio == 3);
    assert(result1->min_commission == 4);

    kungfu::CommissionStorage commission_storage2(fmt::format(COMMISSION_DB_FILE_FORMAT, "/shared/kungfu", "acc_2"));
    kungfu::CommissionManager commission_manager2("xtp_acc_2");
    kungfu::InstrumentCommissionRate input3 = {
        "instrument_2", "exchange_2", '1', "acc_2", "broker_2", '1', 10, 20, 30, 40};
    commission_storage2.add_commission(input3);
    const kungfu::InstrumentCommissionRate* result2 =
        commission_manager2.get_commission_rate('1', "instrument_2", "exchange_2");
    assert(strcmp(result2->instrument_id, "instrument_2") == 0);
    assert(strcmp(result2->exchange_id, "exchange_2") == 0);
    assert(result2->instrument_type == '1');
    assert(strcmp(result2->account_id, "acc_2") == 0);
    assert(strcmp(result2->broker_id, "broker_2") == 0);
    assert(result2->mode == '1');
    assert(result2->open_ratio == 10);
    assert(result2->close_ratio == 20);
    assert(result2->close_today_ratio == 30);
    assert(result2->min_commission == 40);
*/
    return 0;
}
