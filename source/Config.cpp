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
#include "Config.h"
#include <jansson.h>
#include <sstream>

static vector<string> splitName(const string& name)
{
    vector<string> nameParts;

    size_t tokenBegin = 0;

    for(;;)
    {
        size_t tokenEnd = name.find('.', tokenBegin);

        if(tokenEnd == string::npos)
        {
            break;
        }

        nameParts.push_back(name.substr(tokenBegin, tokenEnd - tokenBegin));

        tokenBegin = tokenEnd + 1;
    }

    nameParts.push_back(name.substr(tokenBegin));

    return nameParts;
}

Config::Config()
{
    char* prefPath = SDL_GetPrefPath("boardwalk", "Bael'Zharon's Revenge");
    path_ = string(prefPath) + "config.json";
    SDL_free(prefPath);

    FILE* fp = fopen(path_.c_str(), "r");

    if(fp != nullptr)
    {
        json_error_t err;
        root_ = json_loadf(fp, 0, &err);

        fclose(fp);

        if(root_ == nullptr)
        {
            stringstream errStream;
            errStream << "Configuration parsing error on line " << err.line << ", column " << err.column << ": " << err.text;
            throw runtime_error(errStream.str());
        }

        if(!json_is_object(root_))
        {
            throw runtime_error("Configuration must be an object");
        }
    }
    else
    {
        root_ = json_object();
    }
}

Config::~Config()
{
    json_dump_file(root_, path_.c_str(), JSON_INDENT(2) | JSON_SORT_KEYS);
    json_decref(root_);
}

void Config::setBool(const char* name, bool value)
{
    set(name, json_boolean(value));
}

void Config::setInt(const char* name, int value)
{
    set(name, json_integer(value));
}

void Config::setFloat(const char* name, fp_t value)
{
    set(name, json_real(value));
}

void Config::setString(const char* name, const string& value)
{
    set(name, json_string(value.c_str()));
}

bool Config::getBool(const char* name, bool defaultValue)
{
    json_t* value = get(name);

    if(json_is_boolean(value))
    {
        return json_is_true(value);
    }
    else
    {
        setBool(name, defaultValue);
        return defaultValue;
    }
}

int Config::getInt(const char* name, int defaultValue)
{
    json_t* value = get(name);

    if(json_is_integer(value))
    {
        return static_cast<int>(json_integer_value(value));
    }
    else
    {
        setInt(name, defaultValue);
        return defaultValue;
    }
}

fp_t Config::getFloat(const char* name, fp_t defaultValue)
{
    json_t* value = get(name);

    if(json_is_real(value))
    {
        return static_cast<fp_t>(json_real_value(value));
    }
    else
    {
        setFloat(name, defaultValue);
        return defaultValue;
    }
}

string Config::getString(const char* name, const string& defaultValue)
{
    json_t* value = get(name);

    if(json_is_string(value))
    {
        return json_string_value(value);
    }
    else
    {
        setString(name, defaultValue);
        return defaultValue;
    }
}

void Config::set(const char* name, json_t* value)
{
    vector<string> nameParts = splitName(name);

    string lastPart = nameParts.back();
    nameParts.pop_back();

    json_t* node = root_;

    for(string& part : nameParts)
    {
        json_t* childNode = json_object_get(node, part.c_str());

        if(!json_is_object(childNode))
        {
            childNode = json_object();
            json_object_set_new(node, part.c_str(), childNode);
        }

        node = childNode;
    }

    json_object_set_new(node, lastPart.c_str(), value);
}

json_t* Config::get(const char* name) const
{
    vector<string> nameParts = splitName(name);

    json_t* node = root_;

    for(string& part : nameParts)
    {
        if(!json_is_object(node))
        {
            return nullptr;
        }

        node = json_object_get(node, part.c_str());
    }

    return node;
}
