#ifndef SRC_LANGUAGE_H_
#define SRC_LANGUAGE_H_

#define LANGUAGE_KEYWORDS   " auto double int struct " \
                            "break else long switch " \
                            "case enum register typedef " \
                            "char extern return union " \
                            "const float short unsigned " \
                            "continue for signed void " \
                            "default goto sizeof volatile " \
                            "do if static while "

#define LANGUAGE_NONDIGIT   " a b c d e f g h i j k l m "\
                            "n o p q r s t u v w x y z " \
                            "A B C D E F G H I J K L M " \
                            "N O P Q R S T U V W X Y Z " \
                            "_ "

#define LANGUAGE_DIGIT      " 0 1 2 3 4 5 6 7 8 9 "

#define LANGUAGE_OPERATORS  " [ ] ( ) . -> " \
                            "++ — & * + ~ ! sizeof " \
                            "/ % << >> < > <= >= == != | && || " \
                            "? ; " \
                            "= *= /= %= += -= <<= >>= &= ^= |= " \
                            ", # ## " 

#define LANGUAGE_PUNCTUATORS    " [ ] ( ) { } * , : = ; ... # "

#endif  // SRC_LANGUAGE_H_