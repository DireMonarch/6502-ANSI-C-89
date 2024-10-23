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


/**
 * This project is based on the ANSI X3.159-1989 standard, also known as ANSI C or C89
 * published at https://nvlpubs.nist.gov/nistpubs/Legacy/FIPS/fipspub160.pdf
 * 
 * Specifics for this module were also developed from https://en.wikipedia.org/wiki/C_preprocessor
 */

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "language.h"
#include "helpers.h"



void replace_digraphs(std::string &in_buffer) {
    for (int i = 1; i < in_buffer.length() - 1; i++) {
        if (in_buffer[i] == '?' && in_buffer[i-1] == '?') {
            switch (in_buffer[i+1])
            {
            case '=':
                in_buffer[i-1] = '#';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '/':
                in_buffer[i-1] = '\\';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '\'':
                in_buffer[i-1] = '^';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '(':
                in_buffer[i-1] = '[';
                in_buffer.erase(i, 2);
                i--;
                break;
            case ')':
                in_buffer[i-1] = ']';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '!':
                in_buffer[i-1] = '|';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '<':
                in_buffer[i-1] = '{';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '>':
                in_buffer[i-1] = '}';
                in_buffer.erase(i, 2);
                i--;
                break;
            case '-':
                in_buffer[i-1] = '~';
                in_buffer.erase(i, 2);
                i--;
                break;                                                                                                                                           
            default:
                break;
            }
        }
    }
}

void splice_lines(std::string &in_buffer) {
    for (int i = 1; i < in_buffer.length(); i++) {
        if (in_buffer[i] == '\n' && in_buffer[i-1] == '\\') {
            in_buffer.erase(i-1, 2);
            i--;
        }
    }
    
    if (in_buffer.back() != '\n') {
        in_buffer.append("\n");
    }
}

std::string tokenize(std::string &in_buffer) {
    std::stringstream out_buffer;

    /* comment status flags */
    bool in_comment_inline = false;
    bool in_comment_block = false;
    uint64_t comment_start;

    /* quote staus flags */
    bool in_quotes_single = false;
    bool in_quotes_double = false;

    /* token status flags */
    bool in_token = false;

    int len;  // Variable used in general for temporary storage of substring length

    /* Parse in_buffer one character at a time */
    for (int i = 0; i < in_buffer.length(); i++) {

        /* Process Quotes */
        if (in_buffer[i] == '"' && !(in_comment_inline || in_comment_block || in_quotes_single)) {
            out_buffer << "\"";
            in_quotes_double = !in_quotes_double;
            in_token = in_quotes_double;
            if (!in_token) out_buffer << " ";
        }
        else if (in_buffer[i] == '\'' && !(in_comment_inline || in_comment_block || in_quotes_double)) {
            in_quotes_single = !in_quotes_single;
            in_token = in_quotes_single;
            if (!in_token) out_buffer << " ";
        }   

        /* Process Comments */
        else if (!(in_quotes_double || in_quotes_single)) {  // skip if we are in a quote block
            if (in_buffer[i] == '/' && i + 1 < in_buffer.length()) { // forward slash is first interesting character
                if (in_buffer[i+1] == '*' && !(in_comment_inline || in_comment_block)) {  
                    /* we found / * sequence and we are not already in a comment
                        nor are we in a quote block */
                    in_comment_block = true;
                    comment_start = i;
                    i++;
                }
                else if (in_buffer[i+1] == '/' && !(in_comment_inline || in_comment_block))
                {
                    /* we found // sequence and we are not already in a comment
                        nor are we in a quote block */
                    in_comment_inline = true;
                    comment_start = i;
                    i++;
                }
            }
            else if (in_comment_block && 
                    in_buffer[i] == '*' && 
                    i + 1 < in_buffer.length() &&
                    in_buffer[i+1] == '/') {
                /* we are in a comment block and found an end comment block combination */
                len = i+2-comment_start;
                in_buffer.erase(comment_start+1, len-1);
                in_buffer[comment_start] = ' ';
                in_comment_block = false;
                i = comment_start+1;
            }
            else if (in_comment_inline && 
                    in_buffer[i] == '\n') {
                /* we are in an inline and found an end-of-line */
                len = i-comment_start;
                in_buffer.erase(comment_start+1, len-1);
                in_buffer[comment_start] = ' ';
                in_comment_inline = false;
                i = comment_start+1;
            }
        }

        // else if ()
    }
    return out_buffer.str();
}



int main(int argc, char* argv[]) {
    char* filename = argv[argc - 1];
    std::cout << "Pre-Processing file " << filename << std::endl;

    std::string buffer = get_file_contents(filename);

    replace_digraphs(buffer);
    splice_lines(buffer);
    tokenize(buffer);

    std::cout << buffer << std::endl;

}
