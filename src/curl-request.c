/*
 * request.c
 *
 *  Created on: May 6, 2018
 *      Author: max
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <curl/curl.h>

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}






static
void dump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size)
{
  size_t i;
  size_t c;
  unsigned int width=0x10;

  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
          text, (long)size, (long)size);

  for(i=0; i<size; i+= width) {
    fprintf(stream, "%4.4lx: ", (long)i);

    /* show hex to the left */
    for(c = 0; c < width; c++) {
      if(i+c < size)
        fprintf(stream, "%02x ", ptr[i+c]);
      else
        fputs("   ", stream);
    }

    /* show data on the right */
    for(c = 0; (c < width) && (i+c < size); c++) {
      char x = (ptr[i+c] >= 0x20 && ptr[i+c] < 0x80) ? ptr[i+c] : '.';
      fputc(x, stream);
    }

    fputc('\n', stream); /* newline */
  }
}

static
int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
  const char *text;
  (void)handle; /* prevent compiler warning */
  (void)userp;

  switch (type) {
  case CURLINFO_TEXT:
    fprintf(stderr, "== Info: %s", data);
  default: /* in case a new one is introduced to shock us */
    return 0;

  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }

  dump(text, stderr, (unsigned char *)data, size);
  return 0;
}







int request(uint8_t* ip, int16_t port, uint8_t* cmd, uint8_t* resp)
{
  CURL *curl;
  CURLcode res;
  struct curl_slist *list = NULL;
  //char command[] = "{\"command\":\"getBalances\",\"addresses\":[\"ZGSGUWTSNQVM9BYEPJQGOAMHQSGAVBXSRHM9IBL9PRCUTEZDTKPZE9BSJTCOAOVFF99GGWCYODPCHZ9FZ\"],\"threshold\":100}\r\n";
  char buffer[255] = {0};

  curl = curl_easy_init();
  if(curl) {
    struct string s;
    init_string(&s);

    sprintf(buffer, "%s:%d",ip,port);
    curl_easy_setopt(curl, CURLOPT_URL, buffer);
    //append header
    list = curl_slist_append(list, "Content-Type: application/json");
    list = curl_slist_append(list, "X-IOTA-API-Version: 1");

    //curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cmd);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    res = curl_easy_perform(curl);

    memcpy(resp, s.ptr,s.len);

    free(s.ptr);

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}
