#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "rapidjson/document.h"

class Resource
{
public:
    Resource(rapidjson::Document &json);
    ~Resource() {};

    void dump();
    std::string getIdentifier() { return mIdentifier; }
    void addContainedResource(Resource *contained);
    std::vector<std::string> getContainedResourceNames();

private:

    std::string                        mType;
    std::string                        mTitle;
    std::string                        mIdentifier; // mType[mTitle]
    int                                mLine;
    std::string                        mFile;
    bool                               mExported;
    std::vector<std::string>           mTags;
    std::map<std::string, std::string> mParameters;
        // TODO: non-string parameters

    std::vector<Resource *>            mContainedResources;

    void dumpParameters();
    bool parse_resource(rapidjson::Document &resource);
};
