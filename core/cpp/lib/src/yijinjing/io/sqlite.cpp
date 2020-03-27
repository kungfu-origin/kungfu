//
// Created by Keren Dong on 2020/2/25.
//

#include <kungfu/common.h>
#include <kungfu/yijinjing/common.h>
#include <sqlite3.h>

namespace kungfu::yijinjing {
void handle_sql_error(int rc, const std::string &error_tip) {
  if (SQLITE_OK != rc) {
    SPDLOG_ERROR("sqlite3 rc {}", rc);
    throw yijinjing_error(error_tip);
  }
}

int sql_callback(void *NotUsed, int argc, char **argv, char **azColName) {
  int i;
  for (i = 0; i < argc; i++) {
    SPDLOG_INFO("[sqlite3] callback: {} = {}", azColName[i], argv[i] ? argv[i] : "nullptr");
  }
  return 0;
}

void exec_sql(sqlite3 *db, char **db_error_msg, const std::string &sql, const std::string &error_tip) {
  int rc = sqlite3_exec(db, sql.c_str(), sql_callback, nullptr, db_error_msg);
  handle_sql_error(rc, error_tip);
}

void sqlite3_log(void *callback, int result_code, const char *msg) {
  SPDLOG_WARN("[sqlite3] [{}] {}", result_code, msg);
}

int wal_hook(void *NotUsed, sqlite3 *db, const char *name, int page_nb) {
  SPDLOG_DEBUG("wal committed to {} with page number {}", name, page_nb);
  return 0;
}

int xConflict(void *pCtx, int eConflict, sqlite3_changeset_iter *pIter) { return 0; }

struct sqlite_initilize {
  sqlite_initilize() {
    int rc = sqlite3_config(SQLITE_CONFIG_LOG, sqlite3_log, nullptr);
    handle_sql_error(rc, "failed to config sqlite3 log");
    rc = sqlite3_config(SQLITE_CONFIG_MMAP_SIZE, 1048577);
    handle_sql_error(rc, "failed to config sqlite3");
    sqlite3_initialize();
  }
};

struct sqlite_shutdown {
  sqlite_shutdown() {
    sqlite3_shutdown();
    SPDLOG_DEBUG("sqlite shutdown");
  }
};

void ensure_sqlite_initilize() { static sqlite_initilize instance{}; }

void ensure_sqlite_shutdown() { static sqlite_shutdown instance{}; }
} // namespace kungfu::yijinjing
