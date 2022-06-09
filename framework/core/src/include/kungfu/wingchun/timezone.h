#ifndef WINGCHUN_TIMEZONE_H
#define WINGCHUN_TIMEZONE_H

#include <chrono>
#include <ctime>

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
} // namespace kungfu::wingchun

#endif // WINGCHUN_TIMEZONE_H
