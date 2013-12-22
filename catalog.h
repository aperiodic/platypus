#include <stdint.h>

#include <list>
#include <map>
#include <string>
#include <vector>

#include "rapidjson/document.h"

#include "resource.h"

class Catalog
{
public:
    Catalog(std::string json);
    ~Catalog() {};

    void dump();

private:

    bool mValid;
    std::string mJson;

    int mVersion;
    std::string mEnvironment;
    std::string mName;
    std::vector<std::string>         mTags;
    std::map<std::string, Resource*> mResources;
    std::list<std::string>           mTopLevelResources; // i.e. not contained by another

    void from_json();
    bool validate_document_type(rapidjson::Document &d);
    bool validate_metadata(rapidjson::Document &d);
    bool parse_data(rapidjson::Document &d);
};
