#ifndef SRC_HELPERS_H_
#define SRC_HELPERS_H_

#include <string>

std::string get_file_contents(const char *filename);

bool is_token_a_keyword(const char *token);

#endif  // SRC_HELPERS_H_
