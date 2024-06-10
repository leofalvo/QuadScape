#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <curl/curl.h>

int main(int argc, char *argv[]){
  const char *seed_sites[] = {
    "https://paulgraham.com"
  };
  
  int depth = atoi(argv[1]);
  
  //Start curl
  CURL *curl;
  CURLcode res;
  curl = curl_easy_init();

  while (depth > 0){

  }

  return 0;
}
