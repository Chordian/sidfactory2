#pragma once

#include <streambuf>

namespace Editor
{
    class ComponentConsole;

    class ConsoleOStreamBuffer : public std::streambuf 
    {
        ComponentConsole* m_ComponentConsole;
    public:
        ConsoleOStreamBuffer();
        ConsoleOStreamBuffer(ComponentConsole* inComponentConsole);

        int sync() override;
        int overflow(int ch) override;
        std::streamsize xsputn(const char_type* s, std::streamsize count) override;
    private:
        int write_to_console(const char* inBuffer, const int inLength);
    };
}