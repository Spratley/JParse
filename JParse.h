#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#ifdef _DEBUG
#define VERBOSE_VALIDATION
#endif

namespace JParse
{
     // Item is anything in the json file
    struct Item 
    { 
        ~Item() {}

        virtual const bool PrintOnNewObjectLine() = 0;
        virtual void Parse(std::string const& contents, int& offset) = 0;
        virtual void BuildContents(std::string& outContents, int& tabLevel) = 0;

        template <typename T>
        T* GetAs();
        template <typename T>
        T const* const GetAs() const;
    }; 
    
    struct Root
    {
        ~Root() { delete m_item; }

        void Parse(std::string const& filePath);
        void SaveToFile(std::string const& filePath);

        Item* m_item = nullptr;
    };

    struct Object : public Item
    {
        ~Object();

        const bool PrintOnNewObjectLine() override { return true; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        void Set(std::string const& name, Item* const item);

        template<typename T>
        T* const Get(std::string const& name) const;

        template<typename T>
        T* const TryGet(std::string const& name) const;

        bool const Has(std::string const& name) const;

        std::unordered_map<std::string, Item*> m_contents;
    };

    struct Array : public Item
    {
        ~Array(); 

        const bool PrintOnNewObjectLine() override { return true; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        void Add(Item* const item) { m_contents.push_back(item); }

        template<typename T>
        T* const Get(int const name) const;

        std::vector<Item*> m_contents;
    };

    struct Integer : public Item
    {
        ~Integer() {}

        const bool PrintOnNewObjectLine() override { return false; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        int m_value;
    };

    struct Float : public Item
    {
        ~Float() {}

        const bool PrintOnNewObjectLine() override { return false; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        float m_value;
    };

    struct String : public Item
    {
        ~String() {}

        const bool PrintOnNewObjectLine() override { return false; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        std::string m_value;
    };

    struct Boolean : public Item
    {
        ~Boolean() {}

        const bool PrintOnNewObjectLine() override { return false; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        bool m_value;
    };

    Item* CreateNextItem(std::string const& contents, int const offset);

    template<typename T>
    inline T * Item::GetAs()
    {
        return static_cast<T*>(this);
    }

    template<typename T>
    inline T const* const Item::GetAs() const
    {
        return static_cast<T const* const>(this);
    }

    template<typename T>
    inline T * const Object::Get(std::string const & name) const
    {
        return m_contents.at(name)->GetAs<T>();
    }

    template<typename T>
    inline T* const Object::TryGet(std::string const& name) const
    {
        if (Has(name))
        {
            return Get<T>(name);
        }
        return nullptr;
    }

    template<typename T>
    inline T * const Array::Get(int const index) const
    {
        return m_contents[index]->GetAs<T>();
    }

}