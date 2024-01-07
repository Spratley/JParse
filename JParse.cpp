#include "JParse.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace JParse
{
    Item* CreateNextItem(std::string const& contents, int const offset)
    {
        switch (contents[offset])
        {
        case '{':
            return new Object;
        case '[':
            return new Array;
        case '"':
            return new String;
        case 't':
        case 'f':
            return new Boolean;
        }

        // Remaining tokens must be an int or a float
        size_t delim = contents.find(',');
        delim = std::min(delim, contents.find('}'));
        std::string token = contents.substr(0, delim);
        if (token.find('.') != std::string::npos)
        {
            return new Float;
        }
        return new Integer;
    }

    void JParse::Root::Parse(std::string const& filePath)
    {
        std::ifstream file;
        std::string contents;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            file.open(filePath);
            std::stringstream fileContents;
            fileContents << file.rdbuf();
            file.close();

            contents = fileContents.str();
        }
        catch (std::ifstream::failure const e)
        {
#ifdef _DEBUG
            printf("Failed to read file '%s'", filePath.c_str());
#endif
        }

        // Strip whitespace
        contents.erase(std::remove_if(contents.begin(), contents.end(), ::isspace), contents.end());

        int offset = 0;
        m_item = CreateNextItem(contents, offset);
        m_item->Parse(contents, offset);

    }

    void JParse::Root::SaveToFile(std::string const& filePath)
    {
        std::string contents;
        int tabLevel = 0;
        m_item->BuildContents(contents, tabLevel);

        std::ofstream file;
        file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        try
        {
            file.open(filePath);
            file << contents;
            file.close();
        }
        catch (std::ofstream::failure const e)
        {
#ifdef _DEBUG
            printf("Failed to write file '%s'", filePath.c_str());
#endif
        }
    }

    void JParse::Object::Parse(std::string const & contents, int& offset)
    {
        // Skip open bracket
        offset++;
        do
        {
            if (contents[offset] == '}')
            {
                // End of object, pop
                return;
            }

            if (contents[offset] == ',')
            {
                offset++;
            }

            String str;
            str.Parse(contents, offset);

#ifdef VERBOSE_VALIDATION
            if (contents[offset] != ':')
            {
                printf("Error: Expected a colon!");
            }
#endif
            offset++;
            Item* child = CreateNextItem(contents, offset);
            child->Parse(contents, offset);

            m_contents[str.m_value] = child;

        } while (contents[offset] == ',');
#ifdef VERBOSE_VALIDATION
        if (contents[offset] != '}')
        {
            printf("Error: Expecting to close an object, but the next character is not }!");
        }
#endif
        // Skip end bracket
        offset++;
    }

    void JParse::Array::Parse(std::string const & contents, int & offset)
    {
#ifdef VERBOSE_VALIDATION
        if (contents[offset] != '[')
        {
            printf("Error: Expecting to open an array but the next character is not [!");
        }
#endif

        do
        {
            if (contents[offset + 1] == ']')
            {
                // Empty array, pop
                offset += 2;
                return;
            }

            offset++;
            Item* child = CreateNextItem(contents, offset);
            child->Parse(contents, offset);

            m_contents.push_back(child);

        } while (contents[offset] == ',');

#ifdef VERBOSE_VALIDATION
        if (contents[offset] != ']')
        {
            printf("Error: Expecting to close an array but the next character is not ]!");
        }
#endif
        // Offset past the last ]
        offset++;
    }

    void JParse::Integer::Parse(std::string const & contents, int & offset)
    {
        size_t digitCount;
        std::string substr = contents.substr(offset);
        m_value = std::stoi(substr, &digitCount);
        offset += (int)digitCount;
    }

    void JParse::Float::Parse(std::string const & contents, int & offset)
    {
        size_t digitCount;
        std::string substr = contents.substr(offset);
        m_value = std::stof(substr, &digitCount);
        offset += (int)digitCount;
    }

    void JParse::String::Parse(std::string const & contents, int & offset)
    {
#ifdef VERBOSE_VALIDATION
        if (contents[offset] != '"')
        {
            printf("Error: Expecting a string, but the next character in the file is not a quotation mark!");
        }
#endif
        offset++;
        std::string substr = contents.substr(offset);
        size_t delim = substr.find('"');
        m_value = substr.substr(0, delim);
        offset += (int)delim;
        offset++;
    }

    void JParse::Boolean::Parse(std::string const & contents, int & offset)
    {
#ifdef VERBOSE_VALIDATION
        if (contents.substr(offset, 4) != "true" && contents.substr(offset, 5) != "false")
        {
            printf("Error: Expecting a boolean but the next token is not \"true\" or \"false\"!");
        }
#endif

        if (contents.substr(offset, 4) == "true")
        {
            m_value = true;
            offset += 4;
        }
        else
        {
            m_value = false;
            offset += 5;
        }
    }

    inline JParse::Object::~Object()
    {
        for (auto& item : m_contents)
        {
            delete item.second;
        }
    }

    inline JParse::Array::~Array()
    {
        for (auto& item : m_contents)
        {
            delete item;
        }
    }

    void Append(std::string& str, std::string const& append, int const tabLevel)
    {
        for (int i = 0; i < tabLevel; i++)
        {
            str += "\t";
        }
        str += append;
    }

    void JParse::Object::BuildContents(std::string& outContents, int& tabLevel)
    {
        // Print opening brace
        Append(outContents, "{\n", tabLevel);
        tabLevel++;

        // Print all contents
        for (auto const& content : m_contents)
        {
            // Print quoted name with :
            std::string name = "\"" + content.first + "\":";
            Append(outContents, name, tabLevel);

            // Print child
            if (content.second->PrintOnNewObjectLine())
            {
                outContents += "\n";
            }
            content.second->BuildContents(outContents, tabLevel);

            // Print comma
            outContents += ",\n";
        }

        // Overwrite comma (Since there shouldn't be one on the last item)
        outContents[outContents.size() - 2] = ' ';
        tabLevel--;
        Append(outContents, "}", tabLevel);
    }

    void JParse::Array::BuildContents(std::string& outContents, int& tabLevel)
    {
        // Print opening brace
        Append(outContents, "[\n", tabLevel);
        tabLevel++;

        // Print all contents
        for (auto const& content : m_contents)
        {
            // Print child
            content->BuildContents(outContents, tabLevel);

            // Print comma
            outContents += ",\n";
        }

        // Overwrite comma (Since there shouldn't be one on the last item)
        outContents[outContents.size() - 2] = ' ';
        tabLevel--;
        Append(outContents, "]", tabLevel);
    }

    void JParse::Integer::BuildContents(std::string& outContents, int& tabLevel)
    {
        if (outContents[outContents.size() - 1] != ':')
        {
            // We must be in an array, print indents
            Append(outContents, std::to_string(m_value), tabLevel);
        }
        else
        {
            outContents += std::to_string(m_value);
        }
    }

    void JParse::Float::BuildContents(std::string& outContents, int& tabLevel)
    {
        if (outContents[outContents.size() - 1] != ':')
        {
            // We must be in an array, print indents
            Append(outContents, std::to_string(m_value), tabLevel);
        }
        else
        {
            outContents += std::to_string(m_value);
        }
    }

    void JParse::String::BuildContents(std::string& outContents, int& tabLevel)
    {
        if (outContents[outContents.size() - 1] != ':')
        {
            // We must be in an array, print indents
            Append(outContents, "\"" + m_value + "\"", tabLevel);
        }
        else
        {
            outContents += "\"" + m_value + "\"";
        }
    }

    void JParse::Boolean::BuildContents(std::string& outContents, int& tabLevel)
    {
        std::string str;
        if (m_value)
        {
            str = "true";
        }
        else
        {
            str = "false";
        }

        if (outContents[outContents.size() - 1] != ':')
        {
            // We must be in an array, print indents
            Append(outContents, str, tabLevel);
        }
        else
        {
            outContents += str;
        }
    }
}