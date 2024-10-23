/*
 * Copyright 2024 Jim Haslett
 *
 * This file is part of the 6502 C Compiler implementation.
 *
 * 6502 C Compiler is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * 6502 C Compiler is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * the 6502 C Compiler. If not, see <https:// www.gnu.org/licenses/>.
 */

#include <string>
#include <fstream>

#include "language.h"

std::string get_file_contents(const char *filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

bool _is_in(const char *list, const char *item){
    char search[strlen(item)+2];
    strcpy(search, " ");
    strcat(search, item);
    strcat(search, " ");
    return strstr(list, search) != NULL;    
}

bool is_token_a_keyword(const char *token) {
    return _is_in(LANGUAGE_KEYWORDS, token);
}

bool is_char_a_non_digit(const char *character) {
    return _is_in(LANGUAGE_NONDIGIT, character);
}

bool is_char_a_digit(const char *character) {
    return _is_in(LANGUAGE_DIGIT, character);
}

bool is_token_an_operator(const char *token) {
    return _is_in(LANGUAGE_OPERATORS, token);
}

bool is_token_a_punctuator(const char *token) {
    return _is_in(LANGUAGE_PUNCTUATORS, token);
}