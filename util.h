#ifndef __UTIL_H__
#define __UTIL_H__

// This is really a rapidjson helper, extract for reuse
static rapidjson::Document get_subobject(rapidjson::Document& base, const char *subobject_name)
{
    // nested objects need to be retrieved as a json string and parsed
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer( sb );
    base[subobject_name].Accept( writer );
    rapidjson::Document subobject;
    subobject.Parse<0>(sb.GetString());
    return subobject;
}

static rapidjson::Document get_subobject(const rapidjson::Value& obj)
{
    // nested objects need to be retrieved as a json string and parsed
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer( sb );
    obj.Accept( writer );
    rapidjson::Document subobject;
    subobject.Parse<0>(sb.GetString());
    return subobject;
}

// some utility join functions for use in debug output
// inspired by: http://stackoverflow.com/questions/1430757/c-array-to-string
template <class Iter, class String>
String join(const Iter &begin, const Iter &end, const String &sep)
{
    String result;
    for (Iter it = begin; it != end; ++it) {
        if (!result.empty())
            result.append(sep);
        result.append(*it);
    }
    return result;
}

template <class Container, class String>
String join(const Container& container, const String &sep)
{
    return join(container.begin(), container.end(), sep);
}

#endif // __UTIL_H__
