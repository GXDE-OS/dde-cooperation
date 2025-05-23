// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logging/appenders/file_appender.h"

namespace Logging {

FileAppender::FileAppender(const BaseKit::Path& file, bool truncate, bool auto_flush, bool auto_start)
    : _file(file), _truncate(truncate), _auto_flush(auto_flush)
{
    // Start the file appender
    if (auto_start)
        Start();
}

FileAppender::~FileAppender()
{
    // Stop the file appender
    if (IsStarted())
        Stop();
}

bool FileAppender::Start()
{
    if (IsStarted())
        return false;

    PrepareFile();
    _started = true;
    return true;
}

bool FileAppender::Stop()
{
    if (!IsStarted())
        return false;

    CloseFile();
    _started = false;
    return true;
}

void FileAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.empty())
        return;

    if (PrepareFile())
    {
        // Try to write logging record content into the opened file
        try
        {
            _file.Write(record.raw.data(), record.raw.size() - 1);

            // Perform auto-flush if enabled
            if (_auto_flush)
                _file.Flush();
        }
        catch (const BaseKit::FileSystemException&)
        {
            // Try to close the opened file in case of any IO error
            CloseFile();
        }
    }
}

void FileAppender::Flush()
{
    if (PrepareFile())
    {
        // Try to flush the opened file
        try
        {
            _file.Flush();
        }
        catch (const BaseKit::FileSystemException&)
        {
            // Try to close the opened file in case of any IO error
            CloseFile();
        }
    }
}

bool FileAppender::PrepareFile()
{
    try
    {
        // 1. Check if the file is already opened for writing
        if (_file.IsFileWriteOpened())
            return true;

        // 2. Check retry timestamp if 100ms elapsed after the last attempt
        if ((BaseKit::Timestamp::utc() - _retry).milliseconds() < 100)
            return false;

        // 3. If the file is opened for reading close it
        if (_file.IsFileReadOpened())
            _file.Close();

        // 4. Open the file for writing
        _file.OpenOrCreate(false, true, _truncate);
        _file.Seek(_file.size());

        // 5. Reset the the retry timestamp
        _retry = 0;

        return true;
    }
    catch (const BaseKit::FileSystemException&)
    {
        // In case of any IO error reset the retry timestamp and return false!
        _retry = BaseKit::Timestamp::utc();
        return false;
    }
}

bool FileAppender::CloseFile()
{
    try
    {
        if (_file)
            _file.Close();
        return true;
    }
    catch (const BaseKit::FileSystemException&) { return false; }
}

} // namespace Logging
