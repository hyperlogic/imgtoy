#include "log.h"
#include <cstdio>
#include <cstdarg>

// Console stub
class Console {
public:
    void Write(const char* buffer, size_t size) {}
};

Console* Log::console = nullptr;

int Log::printf(const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (console)
    {
        // convert all \n to \r\n.
        char newBuffer[4096];
        char* c = newBuffer;
        char* cend = newBuffer + 4096;
        for (int i = 0; (i < rc) && c < cend; i++)
        {
            if (buffer[i] == '\n')
            {
                *(c++) = '\r';
            }
            *(c++) = buffer[i];
        }
        console->Write(newBuffer, c - newBuffer);
    }

    fwrite(buffer, rc, 1, stdout);

    return rc;
}

int Log::printf_ansi(AnsiColor color, const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (console)
    {
        char colorSeq[5];
        colorSeq[0] = '\u001b';
        colorSeq[1] = '[';
        colorSeq[2] = '3';
        colorSeq[3] = '0' + (char)color;
        colorSeq[4] = 'm';
        console->Write(colorSeq, 5);

        // convert all \n to \r\n.
        char newBuffer[4096];

        char* c = newBuffer;
        char* cend = newBuffer + 4096;
        for (int i = 0; (i < rc) && c < cend; i++)
        {
            if (buffer[i] == '\n')
            {
                *(c++) = '\r';
            }
            *(c++) = buffer[i];
        }
        console->Write(newBuffer, c - newBuffer);

        char resetSeq[4];
        resetSeq[0] = '\u001b';
        resetSeq[1] = '[';
        resetSeq[2] = '0';
        resetSeq[3] = 'm';
        console->Write(resetSeq, 4);
    }

    fwrite(buffer, rc, 1, stdout);

    return rc;
}
