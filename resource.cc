#include <iostream>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "resource.h"
#include "util.h"

Resource::Resource(rapidjson::Document &json)
{
    parse_resource(json);
}

bool Resource::parse_resource(rapidjson::Document &resource)
{
    if (!resource.HasMember("tags")        || !resource["tags"].IsArray()         ||
        !resource.HasMember("type")        || !resource["type"].IsString()        ||
        !resource.HasMember("title")       || !resource["title"].IsString()       ||
        !resource.HasMember("exported")    || !resource["exported"].IsBool()      ||
        (resource.HasMember("line")        && !resource["line"].IsInt())          ||
        (resource.HasMember("file")        && !resource["file"].IsString())       ||
        (resource.HasMember("parameters")  && !resource["parameters"].IsObject()))
    {
        std::cout << "Bad format for resource?" << std::endl;
        return false;
    }

    mType        = resource["type"].GetString();
    mTitle       = resource["title"].GetString();
    mIdentifier  = mType + "[" + mTitle + "]";

    const rapidjson::Value &tags = resource["tags"];
    for (rapidjson::SizeType i = 0; i < tags.Size(); i++)
    {
        mTags.push_back(tags[i].GetString());
    }

    mExported = resource["exported"].GetBool();

    if (resource.HasMember("parameters"))
    {
        const rapidjson::Value &p = resource["parameters"];
        if (!p.IsObject())
            std::cout << "Parameters not an object?" << std::endl;

        rapidjson::Document parameters = get_subobject(p);

        // enumerate members, check that they're all strings
        rapidjson::Value::ConstMemberIterator iter;
        for (iter = parameters.MemberBegin(); iter != parameters.MemberEnd(); ++iter)
        {
            if (iter->value.GetType() != rapidjson::kStringType)
            {
                static const char* kTypeNames[] = { "Null", "False", "True", "Object",
                                                    "Array", "String", "Number" };
                std::cout << "Dropping parameter " << iter->name.GetString() <<
                    " with type of " << kTypeNames[iter->value.GetType()] << std::endl;
                continue;
            }

            mParameters[iter->name.GetString()] = iter->value.GetString();
        }
    }

    return true;
}

void Resource::addContainedResource(Resource *contained)
{
    mContainedResources.push_back(contained);
}

static std::string getResourceName(Resource *resource)
{
    return resource->getIdentifier();
}

std::vector<std::string> Resource::getContainedResourceNames()
{
    std::vector<std::string> result;
    std::vector<Resource *>::iterator iter;
    for (iter = mContainedResources.begin(); iter != mContainedResources.end(); ++iter)
    {
        result.push_back((*iter)->getIdentifier());
    }

    return result;
}

void Resource::apply()
{
    // spike on file for a few params
    // actual solution will look nothing like this

    if (mType != "File")
    {
        if (mType != "Stage" && mType != "Class")
        {
            // Stage and Class aren't apply-able resources
            std::cout << __func__ << " not supporting resources of type " << mType << std::endl;
        }
        return;
    }


}

void Resource::dumpParameters()
{
    std::cout << "\tparameters:" << std::endl;
    std::map<std::string, std::string>::iterator iter;
    for (iter = mParameters.begin(); iter != mParameters.end(); ++iter)
    {
        std::cout << "\t\t" << iter->first << ":\t" << iter->second << std::endl;
    }
}

void Resource::dump()
{
    std::cout << mIdentifier << std::endl;
    std::cout << "\ttype:\t\t"      << mType << std::endl;
    std::cout << "\ttitle:\t\t"     << mTitle << std::endl;
    std::cout << "\ttags:\t\t"      << join (mTags, std::string(",")) << std::endl;
    std::cout << "\tcontains:\t"    << join (getContainedResourceNames(), std::string(",")) << std::endl;

    dumpParameters();
}
