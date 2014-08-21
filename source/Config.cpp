#include "Config.h"
#include <jansson.h>
#include <sstream>

static vector<string> splitName(const string& name)
{
    vector<string> nameParts;

    size_t tokenBegin = 0;

    for(;;)
    {
        auto tokenEnd = name.find('.', tokenBegin);

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
    auto prefPath = SDL_GetPrefPath("boardwalk", "Bael'Zharon's Revenge");
    _path = string(prefPath) + "config.json";
    SDL_free(prefPath);

    auto fp = fopen(_path.c_str(), "r");

    if(fp != nullptr)
    {
        json_error_t err;
        _root = json_loadf(fp, 0, &err);

        fclose(fp);

        if(_root == nullptr)
        {
            stringstream errStream;
            errStream << "Configuration parsing error on line " << err.line << ", column " << err.column << ": " << err.text;
            throw runtime_error(errStream.str());
        }

        if(!json_is_object(_root))
        {
            throw runtime_error("Configuration must be an object");
        }
    }
    else
    {
        _root = json_object();
    }
}

Config::~Config()
{
    json_dump_file(_root, _path.c_str(), JSON_INDENT(2) | JSON_SORT_KEYS);
    json_decref(_root);
}

void Config::setBool(const char* name, bool value)
{
    set(name, json_boolean(value));
}

void Config::setInt(const char* name, int value)
{
    set(name, json_integer(value));
}

void Config::setDouble(const char* name, double value)
{
    set(name, json_real(value));
}

void Config::setString(const char* name, const string& value)
{
    set(name, json_string(value.c_str()));
}

bool Config::getBool(const char* name, bool defaultValue)
{
    auto value = get(name);

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
    auto value = get(name);

    if(json_is_integer(value))
    {
        return (int)json_integer_value(value);
    }
    else
    {
        setInt(name, defaultValue);
        return defaultValue;
    }
}

double Config::getDouble(const char* name, double defaultValue)
{
    auto value = get(name);

    if(json_is_real(value))
    {
        return json_real_value(value);
    }
    else
    {
        setDouble(name, defaultValue);
        return defaultValue;
    }
}

string Config::getString(const char* name, const string& defaultValue)
{
    auto value = get(name);

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
    auto nameParts = splitName(name);

    auto lastPart = nameParts.back();
    nameParts.pop_back();

    auto node = _root;

    for(auto& part : nameParts)
    {
        auto childNode = json_object_get(node, part.c_str());

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
    auto nameParts = splitName(name);

    auto node = _root;

    for(auto& part : nameParts)
    {
        if(!json_is_object(node))
        {
            return nullptr;
        }

        node = json_object_get(node, part.c_str());
    }

    return node;
}
