#include "Log.h"

#define LOG_CAPACITY 64

static int log_size  = 0;
static int log_next_write_index   = 0;
static int log_level = Log::LOG_LEVEL_DEBUG;

static String logs[LOG_CAPACITY];

static void add(const String& message)
{
    Serial.println(message);

    logs[log_next_write_index] = message;

    ++log_next_write_index;
    log_next_write_index = log_next_write_index % LOG_CAPACITY;

    if (log_size < LOG_CAPACITY) {
        ++log_size;
    }
}

void Log::SetLevel(int level)
{
    log_level = level;
}

int Log::Size()
{
    return log_size;
}

bool Log::GetLog(int index, String& log)
{
    if (index < 0 ||
        log_size >= index) {
        return false;
    }

    if (log_next_write_index == log_size) {
        // log_size  != LOG_CAPACITY
        // log_start == 0
        log = logs[index];
        return true;
    }

    // log_size  == LOG_CAPACITY
    // log_start == log_next_write_index
    int i = (log_next_write_index + index) % LOG_CAPACITY;
    log = logs[i];
    return true;
}

void Log::Fatal(const char *message)
{
    if (log_level > LOG_LEVEL_FATAL) {
        return;
    }
    add(String("[FATAL] ") + message);
}

void Log::Error(const char *message)
{
    if (log_level > LOG_LEVEL_ERROR) {
        return;
    }
    add(String("[ERROR] ") + message);
}

void Log::Warn(const char *message)
{
    if (log_level > LOG_LEVEL_WARN) {
        return;
    }
    add(String("[WARN ] ") + message);
}

void Log::Info(const char *message)
{
    if (log_level > LOG_LEVEL_INFO) {
        return;
    }
    add(String("[INFO ] ") + message);
}

void Log::Debug(const char *message)
{
    if (log_level > LOG_LEVEL_DEBUG) {
        return;
    }
    add(String("[DEBUG] ") + message);
}

void Log::Trace(const char *message)
{
    if (log_level > LOG_LEVEL_TRACE) {
        return;
    }
    add(String("[TRACE] ") + message);
}
