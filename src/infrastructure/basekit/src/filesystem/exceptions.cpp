// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesystem/exceptions.h"

namespace BaseKit {

std::string FileSystemException::string() const
{
    if (_cache.empty())
    {
        std::stringstream stream;
        stream << "File system exception: " << _message << std::endl;
        if (!_path.empty())
            stream << "File system path: " << _path << std::endl;
        if (!_src.empty())
            stream << "File system source path: " << _src << std::endl;
        if (!_dst.empty())
            stream << "File system destination path: " << _dst << std::endl;
        stream << "System error: " << _system_error << std::endl;
        stream << "System message: " << _system_message << std::endl;
        std::string location = _location.string();
        if (!location.empty())
            stream << "Source location: " << location << std::endl;
        _cache = stream.str();
    }
    return _cache;
}

} // namespace BaseKit
