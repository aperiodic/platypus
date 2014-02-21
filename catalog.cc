#include <iostream>
#include <string>
#include <curl/curl.h>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "catalog.h"
#include "util.h"

Catalog::Catalog(std::string json)
{
    mJson = json;
    from_json();
}

bool Catalog::validate_document_type(rapidjson::Document& d)
{
    // validate top-level and document_type
    if (!d.IsObject() ||
            !d.HasMember("document_type") || !d["document_type"].IsString() ||
            strncmp(d["document_type"].GetString(), "Catalog", strlen("Catalog")) != 0)
    {
        std::cout << "No document_type?" << std::endl;
        return false;
    }
    return true;
}

bool Catalog::validate_metadata(rapidjson::Document& d)
{
    if (!d.HasMember("metadata") || !d["metadata"].IsObject() )
    {
        std::cout << "No metadata?" << std::endl;
        return false;
    }

    rapidjson::Document metadata = get_subobject(d, "metadata");
    if (!metadata.HasMember("api_version") || !metadata["api_version"].IsInt() ||
            metadata["api_version"].GetInt() != 1)
    {
        std::cout << "Bad metadata?" << std::endl;
        return false;
    }

    return true;
}

bool Catalog::parse_data(rapidjson::Document &d)
{
    // validate that there is a data object
    if (!d.HasMember("data") || !d["data"].IsObject())
    {
        std::cout << "No data?" << std::endl;
        return false;
    }

    rapidjson::Document data = get_subobject(d, "data");

    if (!data.HasMember("tags")        || !data["tags"].IsArray()         ||
        !data.HasMember("name")        || !data["name"].IsString()        ||
        !data.HasMember("version")     || !data["version"].IsInt()        ||
        !data.HasMember("environment") || !data["environment"].IsString() ||
        !data.HasMember("resources")   || !data["resources"].IsArray()    ||
        !data.HasMember("edges")       || !data["edges"].IsArray()        ||
        !data.HasMember("classes")     || !data["classes"].IsArray())
    {
        std::cout << "Bad format for data?" << std::endl;
        return false;
    }

    mVersion     = data["version"].GetInt();
    mName        = data["name"].GetString();
    mEnvironment = data["environment"].GetString();

    const rapidjson::Value &tags = data["tags"];
    for (rapidjson::SizeType i = 0; i < tags.Size(); i++)
    {
        mTags.push_back(tags[i].GetString());
    }

    const rapidjson::Value &resources = data["resources"];
    for (rapidjson::SizeType i = 0; i < resources.Size(); i++)
    {
        const rapidjson::Value &r = resources[i];
        if (!r.IsObject())
            std::cout << "Resource not an object?" << std::endl;

        rapidjson::Document rsrc = get_subobject(r);
        Resource *resource = new Resource(rsrc);
        mResources[resource->getIdentifier()] = resource;

        // at this point, treat everything as a top-level resource
        // the list will be pruned as we process edges below and discover
        // which resources are contained by other resources
        mTopLevelResources.push_back(resource->getIdentifier());
    }

    const rapidjson::Value &edges = data["edges"];
    for (rapidjson::SizeType i = 0; i < edges.Size(); i++)
    {
        const rapidjson::Value &e = edges[i];
        if (!e.IsObject())
            std::cout << "Edge not an object?" << std::endl;

        rapidjson::Document edge = get_subobject(e);
        if (!edge.HasMember("source") || !edge["source"].IsString() ||
            !edge.HasMember("target") || !edge["target"].IsString())
        {
            std::cout << "Bad edge object?" << std::endl;
            continue;
        }

        Resource *source = mResources[edge["source"].GetString()];
        if (!source) continue;
        Resource *target = mResources[edge["target"].GetString()];
        if (!target) continue;

        source->addContainedResource(target);
        mTopLevelResources.remove(target->getIdentifier());
    }

    return true;
}

void Catalog::from_json()
{
    mValid = true;

    rapidjson::Document d;
    d.Parse<0>(mJson.c_str());

    mValid &= validate_document_type(d);
    mValid &= validate_metadata(d);
    mValid &= parse_data(d);
}

void Catalog::apply()
{
    // apply resources
    std::map<std::string, Resource*>::iterator iter;
    for (iter = mResources.begin(); iter != mResources.end(); ++iter)
    {
        iter->second->apply();
    }
}

void Catalog::dump()
{
    std::cout << "name:\t\t"        << mName << std::endl;
    std::cout << "environment:\t"   << mEnvironment << std::endl;
    std::cout << "version:\t"       << mVersion << std::endl;
    std::cout << "tags:\t\t"        << join (mTags, std::string(",")) << std::endl;
    std::cout << "top resources:\t" << join (mTopLevelResources, std::string(",")) << std::endl;
    std::cout << std::endl;

    // dump resources
    std::map<std::string, Resource*>::iterator iter;
    for (iter = mResources.begin(); iter != mResources.end(); ++iter)
    {
        iter->second->dump();
    }
}
