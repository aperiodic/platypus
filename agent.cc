#include <iostream>
#include <string>
#include <curl/curl.h>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "catalog.h"

int debug = 0;

size_t curl_write_fn(char *ptr, size_t size, size_t nmemb, void *data)
{
    std::string *str = reinterpret_cast<std::string *>(data);

    for (int x = 0; x < size * nmemb; ++x) {
        (*str) += ptr[x];
    }

    return size * nmemb;
}

int retrieve_and_apply_catalog(void)
{
    CURL *curl;
    CURLcode res;

    static const char *pCertFile   = "/Users/kylo/devel/test/client/conf/ssl/certs/kylo.local.pem";
    static const char *pCACertFile = "/Users/kylo/devel/test/client/conf/ssl/certs/ca.pem";
    static const char *pKeyName    = "/Users/kylo/devel/test/client/conf/ssl/private_keys/kylo.local.pem";

    curl_global_init(CURL_GLOBAL_DEFAULT);

    while (true) {   // pseudo-while, always break at the bottom
        curl = curl_easy_init();
        if (!curl) {
            fprintf(stderr, "curl_easy_init() failed\n");
            break;
        }

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
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            if (res == CURLE_SSL_CACERT) {
                fprintf(stderr, "If you're on Mavericks, you will need to add the ca cert to the keychain, e.g. with\n" \
                        "\tsecurity add-trusted-cert <ca.pem>\n" \
                        "\nSee http://curl.haxx.se/mail/archive-2013-10/0036.html for some background.");
            }
            break;
        }

        if (debug)
            std::cout << response << std::endl;

        Catalog catalog(response);
        catalog.dump();

        std::cout << "Applying catalog ..." << std::endl;
        catalog.apply();

        /* always cleanup */
        curl_easy_cleanup(curl);

        break;
    }

    curl_global_cleanup();

    return 0;
}

int main(int argc, char **argv)
{
    retrieve_and_apply_catalog();
}
