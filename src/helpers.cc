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
#include <sstream>
#include <iostream>

#include "language.h"
#include "helpers.h"

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

bool is_char_a_non_digit(char character) {
    char item[2] = " ";
    item[0] = character;    
    return _is_in(LANGUAGE_NONDIGIT, item);
}

bool is_char_a_digit(char character) {
    char item[2] = " ";
    item[0] = character;    
    return _is_in(LANGUAGE_DIGIT, item);
}

bool is_token_an_operator(const char *token) {
    return _is_in(LANGUAGE_OPERATORS, token);
}

bool is_token_a_punctuator(const char *token) {
    return _is_in(LANGUAGE_PUNCTUATORS, token);
}
bool is_char_whitepsace(char character){
    char item[2] = " ";
    item[0] = character;
    return _is_in(LANGUAGE_WHITESPACE, item);
}

bool _is_hex_digit(char character) {
    char item[2] = " ";
    item[0] = character;
    return _is_in(LANGAUGE_HEX_DIGITS, item);    
}

bool _is_octal_digit(char character) {
    char item[2] = " ";
    item[0] = character;
    return _is_in(LANGAUGE_OCTAL_DIGITS, item);    
}


bool _is_char_in_source_character_set(char character) {
    char item[2] = " ";
    item[0] = character;
    return _is_in(LANGUAGE_SOURCE_CARACTER_SET, item);    
}


int _is_escape_sequence_at(std::string token, int index) {
    if (token.length() <= index+1 ) return 0;
    if (index < 0) return 0;
    if (token[index] != '\\') return 0;
    
    int i;

    switch (token[index+1])
    {
    case '\'':
    case '"':
    case '?':
    case '\\':
    case 'a':
    case 'g':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case 'v':
        return 2;
        break;
    
    case 'x':
        i = 2;
        while(index+i < token.length() && _is_hex_digit(token[index+i])) {
            i++;
        }
        if (i > 2) return i;
        break;

    default:
        i = 1;
        while(index+i < token.length() && _is_octal_digit(token[index+i])) {
            i++;
        }
        if (i > 1) return i;
        break;
    }
    return 0;
}


bool is_valid_identifier(std::string token) {
    if(token.length() < 1) return false;
    if(is_token_a_keyword(token.c_str())) return false;
    if(!is_char_a_non_digit(token[0])) return false;
    for(int i = 1; i < token.length(); i++){
        if(!(is_char_a_non_digit(token[i]) || is_char_a_digit(token[i]))) return false;
    }
    return true;
}

bool is_valid_header_name(std::string token) {
    if (token[0] != '<' ) return false;
    for (int i = 1; i < token.length(); i++) {
        if (token[i] == '>' && i != token.length()-1) return false;
        if (token[i] == '\n') return false;
        if (token[i] == '\\') {
            int consumed = _is_escape_sequence_at(token, i);
            if (consumed > 0) {
                i += consumed;
            }
            else return false;
        }
        else if (!_is_char_in_source_character_set(token[i])) {
            return false;
        }
    }
 
    if (token[token.length()-1] != '>') return false;
    return true;
}


bool is_valid_string_literal(std::string token) {
    if (token[0] != '"' ) return false;
    for (int i = 1; i < token.length(); i++) {
        if (token[i] == '"' && i != token.length()-1) return false;
        if (token[i] == '\n') return false;
        if (token[i] == '\\') {
            int consumed = _is_escape_sequence_at(token, i);
            if (consumed > 0) {
                i += consumed;
            }
            else return false;
        }
        else if (!_is_char_in_source_character_set(token[i])) {
            return false;
        }
    }
 
    if (token[token.length()-1] != '"') return false;
    return true;
}

bool is_valid_character_constant(std::string token) {
    if (token[0] != '\'' ) return false;
    for (int i = 1; i < token.length(); i++) {
        if (token[i] == '\'' && i != token.length()-1) return false;
        if (token[i] == '\n') return false;
        if (token[i] == '\\') {
            int consumed = _is_escape_sequence_at(token, i);
            if (consumed > 0) {
                i += consumed;
            }        
            else return false;
        }
        else if (!_is_char_in_source_character_set(token[i])) return false;
    }
    if (token[token.length()-1] != '\'') return true;
    return true;
}


std::string get_next_token(std::string &in_buffer, int &index) {
    std::stringstream token;
    /* skip whitespace */
    while(index < in_buffer.length() && is_char_whitepsace(in_buffer[index])) index++;
    if(index >= in_buffer.length()) return token.str();
    if(in_buffer[index] == '"' || in_buffer[index] == '\'') { /* string or character literal */
        char terminator = in_buffer[index];

        token << in_buffer[index];

        index++;
        while(index < in_buffer.length() && (in_buffer[index] != terminator || 
                                            (in_buffer[index-1] == '\\' && in_buffer[index-2] != '\\'))) {  
            token << in_buffer[index];
            index++;
        }
        if(in_buffer[index] == terminator) {
            token << in_buffer[index];
            index++;
            return token.str();
        }
        return std::string();
    }

    /* if not a string/character literal */
    while(index < in_buffer.length() && !is_char_whitepsace(in_buffer[index])) {
        token << in_buffer[index];
        index++;
    }
    return token.str();   
}
