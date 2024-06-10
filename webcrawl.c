
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <curl/curl.h>
#include <regex.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize +  1);
  if (ptr == NULL){
    printf("Insufficient memory");
    return 0;
  }
}

int main(int argc, char *argv[]) {
    const char *seed_sites[] = {
        "https://paulgraham.com"
    };
    
    // Website queue
    GQueue *queue = g_queue_new();

    // Databse
    GHashTable *text_data = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_list_free_full);
    
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
                if (res != CURLE_OK){
                  fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                } else {
                process_html(chunk.memory, queue, text_data);
                }
                free(chunk.memory);
                g_free(site);
                
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
