#ifndef SRC_HELPERS_H_
#define SRC_HELPERS_H_

#include <string>

std::string get_file_contents(const char *filename);

bool is_token_a_keyword(const char *token);
bool is_char_a_non_digit(char character);
bool is_char_a_digit(char character);
bool is_token_an_operator(const char *token);
bool is_token_a_punctuator(const char *token);
bool is_char_whitepsace(char character);

bool is_valid_header_name(std::string token);
bool is_valid_string_literal(std::string token);
bool is_valid_character_constant(std::string token);
std::string get_next_token(std::string &in_buffer, int &index);


#endif  // SRC_HELPERS_H_
