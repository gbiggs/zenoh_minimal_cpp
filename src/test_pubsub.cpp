#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <mutex>
#include <unordered_map>
#include <string>
#include <chrono>

extern "C" {
  #include "zenoh/zenoh-ffi.h"
}

std::mutex sub_callback_mutex;
std::unordered_map<std::string, size_t> topic_map;

void sub_callback(const zn_sample * sample) {
    std::lock_guard<std::mutex> guard(sub_callback_mutex);

    printf("\n>> Received on: %.*s, %d\n( ",
           sample->key.len,
           sample->key.val,
           sample->value.len);

    for (int i = 0 ; i < sample->value.len; i++) {
      printf("%02x ",*(sample->value.val + i));
    }

    printf(")\n");
}

int main(int argc, char** argv) {
    char *key_expr = "/test";

    ZNSession *s = zn_open(PEER, 0, 0);

    if (s == 0) {
        printf("Error creating session!\n");
        exit(-1);
    }

    sleep(1);

    if (argc > 1)
    {
      for (int i = 1; i < argc; i++) {
        key_expr = argv[i];
        topic_map[std::string(key_expr)] = zn_declare_resource(s, key_expr);
        zn_declare_subscriber(s, key_expr, zn_subinfo_default(), sub_callback);

        printf("Subscription expression to %s (%ld)\n", key_expr, topic_map[std::string(key_expr)]);
      }
    } else {
      topic_map[std::string(key_expr)] = zn_declare_resource(s, key_expr);
      zn_declare_subscriber(s, key_expr, zn_subinfo_default(), sub_callback);

      printf("Subscription expression to %s (%ld)\n", key_expr, topic_map[std::string(key_expr)]);
    }

    for (int i = 0; i < 3; i++)
    {
      for (std::pair<std::string, size_t> element : topic_map) {
        std::string msg = "hello world from " + element.first + " " + std::to_string(i);

        zn_write_wrid(s,
                      element.second,
                      msg.c_str(),
                      strlen(msg.c_str()));
        printf("PUBLISHED TO %s (%ld): %s\n", element.first.c_str(), element.second, msg.c_str());
      }
      sleep(3);
    }

    // char ch;
    // read(0, &ch, 1);
}
