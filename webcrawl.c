
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <curl/curl.h>

int main(int argc, char *argv[]) {
    const char *seed_sites[] = {
        "https://paulgraham.com"
    };
    int num_sites = sizeof(seed_sites) / sizeof(seed_sites[0]);
    
    GQueue *queue = g_queue_new();
    
    int depth = atoi(argv[1]);
    
    // Add sites to queue
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
                
                curl_easy_setopt(curl, CURLOPT_URL, site);
                res = curl_easy_perform(curl);
                if (res != CURLE_OK)
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                
                // Add your processing logic here
                
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
