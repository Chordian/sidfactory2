#include "runtime/editor/converters/utils/consoleostreambuffer.h"
#include "runtime/editor/components/component_console.h"

namespace Editor
{
    ConsoleOStreamBuffer::ConsoleOStreamBuffer()
        : m_ComponentConsole(nullptr)
    {
    }

    ConsoleOStreamBuffer::ConsoleOStreamBuffer(ComponentConsole* inComponentConsole)
        : m_ComponentConsole(inComponentConsole)
    {
    }

    int ConsoleOStreamBuffer::sync()
    {
        int n = static_cast<int>(pptr() - pbase());
        return (n && write_to_console(pbase(), n) != n) ? EOF : 0;
    }

    int ConsoleOStreamBuffer::overflow(int ch)
    {
        int n = static_cast<int>(pptr() - pbase());
        if (n && sync())
            return EOF;
        if (ch != EOF)
        {
            char cbuf[1];
            cbuf[0] = static_cast<char>(ch);
            if (write_to_console(cbuf, 1) != 1)
                return EOF;
        }
        pbump(-n);  // Reset pptr().
        return 0;
    }

    std::streamsize ConsoleOStreamBuffer::xsputn(const char_type* s, std::streamsize count)
    {
        return sync() == EOF ? 0 : write_to_console(s, static_cast<int>(count));
    }

    int ConsoleOStreamBuffer::write_to_console(const char* inBuffer, const int inLength)
    {
        std::string string = std::string(inBuffer, inLength);
        m_ComponentConsole->operator<<(string);

        return inLength;
    }
}