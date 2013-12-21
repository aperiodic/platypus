#include <iostream>
#include <string>
#include <curl/curl.h>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

size_t curl_write_fn(char *ptr, size_t size, size_t nmemb, void *data)
{
    std::string *str = reinterpret_cast<std::string *>(data);

    for (int x = 0; x < size * nmemb; ++x)
    {
        (*str) += ptr[x];
    }

    return size * nmemb;
}

int main(void)
{
  CURL *curl;
  CURLcode res;

  static const char *pCertFile   = "/Users/kylo/devel/test/client/conf/ssl/certs/kylo.local.pem";
  static const char *pCACertFile = "/Users/kylo/devel/test/client/conf/ssl/certs/ca.pem";
  static const char *pKeyName    = "/Users/kylo/devel/test/client/conf/ssl/private_keys/kylo.local.pem";

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    /* set up callback */
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, "https://localhost:8140/env/catalog/kylo.local");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_fn);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);

    /* set the cert for client authentication */
    curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);

    /* set the private key */
    curl_easy_setopt(curl, CURLOPT_SSLKEY, pKeyName);

    /* set the file with the certs vaildating the server */
    curl_easy_setopt(curl, CURLOPT_CAINFO, pCACertFile);

    /* disconnect if we can't validate server's cert */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    std::cout << response << std::endl;

    // Break ths out into a catalog class
    rapidjson::Document d;
    d.Parse<0>(response.c_str());

    // validate top-level and document_type
    if (!d.IsObject() ||
        !d.HasMember("document_type") ||
        !d["document_type"].IsString() ||
        strncmp(d["document_type"].GetString(), "Catalog", strlen("Catalog")) != 0)
    {
        std::cout << "No document_type?" << std::endl;
    }

    if (!d.HasMember("metadata") ||
        !d["metadata"].IsObject() )
    {
        std::cout << "No metadata?" << std::endl;
    }

    // nested objects need to be retrieved as a json string and parsed
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer( sb );
    d["metadata"].Accept( writer );
    rapidjson::Document metadata;
    metadata.Parse<0>(sb.GetString());

    // validate metadata object
    if (!metadata.HasMember("api_version") ||
        !metadata["api_version"].IsInt() ||
        metadata["api_version"].GetInt() != 1)
    {
        std::cout << "Bad metadata?" << std::endl;
    }

    // validate that there is a data object
    if (!d.HasMember("data"))
    {
        std::cout << "No data?" << std::endl;
    }

    // nested objects need to be retrieved as a json string and parsed
    rapidjson::StringBuffer sb2;
    rapidjson::Writer<rapidjson::StringBuffer> writer2( sb );
    d["data"].Accept( writer2 );
    rapidjson::Document data;
    data.Parse<0>(sb2.GetString());

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}
