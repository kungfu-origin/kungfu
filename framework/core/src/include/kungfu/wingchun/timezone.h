// SPDX-License-Identifier: Apache-2.0

#ifndef WINGCHUN_TIMEZONE_H
#define WINGCHUN_TIMEZONE_H

#include <chrono>
#include <ctime>
#include <memory>

//////////////////////////////////////////////////////////////////////////////
#include "common.h"

namespace kungfu::wingchun {
/*****************************************************************************
 *  @Copyright (c) 2022, Marsjliu
 *  @All rights reserved
 *  @date     : 2022/06/06 11:15
 *****************************************************************************/
enum class LocationTimeType : char {
  London,           //
  Beijing,          //
  Singapore,        //
  Tokyo,            //
  AmericaEastern,   //
  AmericaCentral,   //
  AustraliaEastern, //
  Berlin            //
};

/*
 * http://zh.thetimenow.com/time-zones-abbreviations.php
 */
enum class ZoneTimeType : char {
  BST,         //
  BDT,         //
  BeijingTime, //
  JST,         //
  SGT,         //
  EST,         //
  EDT,         //
  CST,         //
  CDT,         //
  AEST,        //
  AEDT,        //
  CET,         //
  CEST         //
};

struct LocalTimeInfo {
  time_t seconds = 0;

  bool has_summer_day = false;
  bool is_summer_day = false;

  ZoneTimeType zone_time_type = ZoneTimeType::BST;
};

namespace TimeUtil {
time_t TimeToSeconds(const std::string &time, bool is_gmt);
time_t TimeToSeconds(int year, int month, int day, int hour, int minute, int second, bool is_gmt);

time_t DateToSeconds(const std::string &time, bool is_gmt);

time_t TranslateLocalTimeToGMTime(time_t local_seconds, LocationTimeType loc_type, LocalTimeInfo *info = nullptr);
std::shared_ptr<LocalTimeInfo> TranslateGMTimeToLocalTime(time_t gmt, LocationTimeType loc_type);
} // namespace TimeUtil

//
// param1: local time
// param2: exchangeid
// params3:  YYYYMMDD
inline std::string translate_GMTime_to_localdate_by_exchange_id(time_t lTime, const std::string &exchangeId,
                                                                const std::string strformat = "%Y%m%d") {
  // ExchangeId LocationTime
  static const std::unordered_map<std::string, LocationTimeType> location_time_exchange_id_map = {
      {EXCHANGE_US, LocationTimeType::AmericaEastern},   //
      {EXCHANGE_HK, LocationTimeType::Beijing},          //
      {EXCHANGE_SSE, LocationTimeType::Beijing},         //
      {EXCHANGE_SZE, LocationTimeType::Beijing},         //
      {EXCHANGE_BSE, LocationTimeType::Beijing},         //
      {EXCHANGE_SHFE, LocationTimeType::Beijing},        //
      {EXCHANGE_HK_FUTURE, LocationTimeType::Beijing},   //
      {EXCHANGE_SGX, LocationTimeType::Beijing},         //
      {EXCHANGE_SGX_FUTURE, LocationTimeType::Beijing},  //
      {EXCHANGE_GLFX, LocationTimeType::Beijing},        //
      {EXCHANGE_IPE, LocationTimeType::London},          //
      {EXCHANGE_CBOT, LocationTimeType::AmericaEastern}, //
      {EXCHANGE_CEC, LocationTimeType::AmericaEastern},  //
      {EXCHANGE_LIFE, LocationTimeType::AmericaEastern}, //
      {EXCHANGE_MTIF, LocationTimeType::AmericaEastern}, //
      {EXCHANGE_NYCE, LocationTimeType::AmericaEastern}, //
      {EXCHANGE_CMX, LocationTimeType::AmericaEastern},  //
      {EXCHANGE_SIME, LocationTimeType::AmericaEastern}, //
      {EXCHANGE_CME, LocationTimeType::AmericaEastern},  //
      {EXCHANGE_IMM, LocationTimeType::AmericaEastern},  //
      {EXCHANGE_WIDX, LocationTimeType::AmericaEastern}, //
      {EXCHANGE_FREX, LocationTimeType::AmericaEastern}, //
      {EXCHANGE_METL, LocationTimeType::AmericaEastern}, //
      {EXCHANGE_IPM, LocationTimeType::London}           //
  };
  LocationTimeType type = LocationTimeType::Beijing;
  auto it = location_time_exchange_id_map.find(exchangeId);

  if (it != location_time_exchange_id_map.end()) {
    type = it->second;
  }

  time_t local_time = TimeUtil::TranslateGMTimeToLocalTime(lTime, type)->seconds;

  char datebuf[256] = {0};
  strftime(datebuf, 256, strformat.c_str(), gmtime(&local_time));
  return datebuf;
}

} // namespace kungfu::wingchun

#endif // WINGCHUN_TIMEZONE_H
