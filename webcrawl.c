#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/uri.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static int process_html(char* memory, GQueue *queue, const char *base_url) {
  
    
    htmlDocPtr doc = htmlReadMemory(memory, strlen(memory), NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == NULL) {
        fprintf(stderr, "HTML PARSING FAILED\n");
        return -1;
    }

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        fprintf(stderr, "FAILED TO CREATE XPATH CONTENT\n");
        xmlFreeDoc(doc);
        return -1;
    }

    //find all <a> tags
    xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar *)"//a/@href", context);
    if (result == NULL) {
        fprintf(stderr, "CANNOT EVAULUATE XPATH EXPR\n");
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return -1;
    }

    xmlNodeSetPtr nodes = result->nodesetval;

    //add href values to queue
    for (int i = 0; i < nodes->nodeNr; i++) {
        xmlChar *href = xmlNodeListGetString(doc, nodes->nodeTab[i]->xmlChildrenNode, 1);
        if (href != NULL) {
            char *link = (char *)href;

            //make absolute url (some links are relative)
            xmlChar *absolute_url = xmlBuildURI((xmlChar *)link, (xmlChar *)base_url);
            if (absolute_url != NULL) {
                if (g_str_has_prefix((char *)absolute_url, "http://") || g_str_has_prefix((char *)absolute_url, "https://")) {
                    g_queue_push_tail(queue, g_strdup((char *)absolute_url));
                }
                xmlFree(absolute_url);
            } else {
                printf("UNRESOLVED: %s\n", link);
            }
            xmlFree(href);
        }
    }

    //free space
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    return 0;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        // Out of memory!
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int main(int argc, char *argv[]) {
    const char *seed_sites[] = {
        "https://paulgraham.com"
    };

    // Website queue
    GQueue *queue = g_queue_new();

    // Argument for how many times to go to the next link
    int depth = atoi(argv[1]);

    // Add sites to queue
    int num_sites = sizeof(seed_sites) / sizeof(seed_sites[0]);
    for (int i = 0; i < num_sites; i++) {
        g_queue_push_tail(queue, g_strdup(seed_sites[i]));
    }

    // Start curl
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {  // Check if Curl handle is initialized successfully
        while (depth > 0) {
            if (!g_queue_is_empty(queue)) {
                char *site = (char *)g_queue_pop_head(queue);
                printf("Fetching site: %s\n", site);

                struct MemoryStruct chunk;
                chunk.memory = malloc(1);
                chunk.size = 0;

                curl_easy_setopt(curl, CURLOPT_URL, site);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                } else {
                    process_html(chunk.memory, queue, site);  // Pass the current site as the base URL
                }
                free(chunk.memory);
                g_free(site);
            } else {
                break;
            }

            depth--;
        }

        curl_easy_cleanup(curl);  // Clean up Curl handle
    } else {
        fprintf(stderr, "Failed to initialize CURL\n");
    }

    g_queue_free(queue);  // Free the queue

    return 0;
}

