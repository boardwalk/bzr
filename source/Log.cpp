/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "Log.h"
#include "Config.h"
#include "Core.h"
#include <iostream>

namespace {

// left unopened intentionally
ofstream g_nullStream;

ostream& operator<<(ostream& os, LogSeverity sev)
{
    switch(sev)
    {
        case LogSeverity::kDebug:
            os << "DEBUG ";
            break;
        case LogSeverity::kInfo:
            os << "INFO  ";
            break;
        case LogSeverity::kNotice:
            os << "NOTICE";
            break;
        case LogSeverity::kWarn:
            os << "WARN  ";
            break;
        case LogSeverity::kError:
            os << "ERROR ";
            break;
        case LogSeverity::kCrit:
            os << "CRIT  ";
            break;
        case LogSeverity::kAlert:
            os << "ALERT ";
            break;
        case LogSeverity::kEmerg:
            os << "EMERG ";
            break;
        default:
            throw out_of_range("Bad LogSeverity");
    }

    return os;
}

} // unnamed namespace

Log::Log()
{
    string output = Core::get().config().getString("Log.output", "stderr");

    if(output == "stdout")
    {
        os_ = &cout;
    }
    if(output == "stderr")
    {
        os_ = &cerr;
    }
    else
    {
        // TODO put in proper folder?
        fs_.open(output);
        os_ = &fs_;
    }

    if(!os_->good())
    {
        throw runtime_error("failed to open log file");
    }

    verbosity_ = Core::get().config().getInt("Log.verbosity", static_cast<int>(LogSeverity::kWarn));
}

ostream& Log::write(LogSeverity sev)
{
    if(static_cast<int>(sev) < verbosity_)
    {
        return g_nullStream;
    }

    return *os_ << sev << " ";
}
