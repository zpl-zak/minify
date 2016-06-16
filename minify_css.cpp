// (c) 2016 ZaKlaus; All Rights Reserved

/* TODO(zaklaus):
Minifier still has some flaws and leaves a few whitespaces in the code.
It also does not support comments, but leaves them in the code aswell.
Code requires refactoring.
*/

#include "minify_api.h"

enum STATES
{
    WRITE,
    IGNORE,
    NUM_OF_STATES
};

size_t
strlen(char* s)
{
    char * str = s;
    size_t Result = 0;
    while(*(str++) != 0)
    {
        Result ++;
    }
    return Result;
}

char
is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

char
lookup_char(char * ptr, char c)
{
    char * p = ptr;
    while(*(p) != 0)
    {
        if(!is_whitespace(*p) && *p != c)
        {
            return 1;
        }
        if(*p == c)
        {
            return 0;
        }
        p++;
    }
    return 1;
}

extern "C" MINIFY_FILE(MinifyFile)
{
    // NOTE(zaklaus): Handle minification of file
    
    char * output = (char *) api->AllocMem(sizeof(char) * (strlen(api->FileContent) + 1));
    
    int state = WRITE;
    char * b = output;
    char * ptr = api->FileContent;
    
    while(*ptr != 0)
    {
        char c = *(ptr++);
        
        switch(c)
        {   
            case '{':
            {
                // NOTE(zaklaus): ???
                
                state = IGNORE;
                *(output++) = '{';
            }break;
            
            case '}':
            {
                state = WRITE;
                *(output++) = '}';
            }break;
            
            case '\n': case '\r':
            {
                continue;
            }break;
            
            case '.': case '#':
            {
                state = WRITE;
                *(output++) = c;
            }break;
            
            case ':':
            {
                state = WRITE;
                *(output++) = c;
                
                if (is_whitespace(*ptr))
                {
                    ptr++;
                }
            }break;
            
            case ';':
            {
                state = IGNORE;
                *(output++) = c;
            }break;
            
            default:
            {
                // TODO(zaklaus): Replace with IsCharWhitespace
                if(state == WRITE && (c == ' ' || c == '\t' || c == '\n'))
                {
                    if(lookup_char(ptr-1, '{'))
                    {
                        *(output++) = c;
                    }
                }
                else if(c != ' ' && c != '\t' && c != '\n')
                {
                    *(output++) = c;
                }
                
            }break;
        }
    }
    *output = 0;
    return b;
}

extern "C" GET_EXTENSION(GetExtension)
{
    return "css";
}