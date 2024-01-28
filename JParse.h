#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _DEBUG
#define VERBOSE_VALIDATION
#endif

#define GET_VALUE_IMPL(type, alias) \
template<> inline type Object::GetValue<type>(std::string const& name) { return Get<alias>(name)->m_value; } \
template<> inline type const Object::GetValue<type>(std::string const& name) const {return Get<alias>(name)->m_value; } \
template<> inline type Object::TryGetValue<type>(std::string const& name) { if (Has(name)) { return GetValue<type>(name); } return type (); } \
template<> inline type const Object::TryGetValue<type>(std::string const& name) const { if (Has(name)) { return GetValue<type>(name); } return type (); } 

namespace JParse
{
    struct Item;

    struct Root
    {
    public:
        void Parse(std::string const& filePath);
        void SaveToFile(std::string const& filePath);
        void CreateNew();

        template <typename T>
        std::shared_ptr<T> GetRootItem();

    private:
        std::shared_ptr<Item> m_item = nullptr;
    };

     // Item is anything in the json file
    struct Item : public std::enable_shared_from_this<Item> // Allows us to get this as a shared_ptr
    { 
        virtual const bool PrintOnNewObjectLine() = 0;
        virtual void Parse(std::string const& contents, int& offset) = 0;
        virtual void BuildContents(std::string& outContents, int& tabLevel) = 0;

        template <typename T>
        std::shared_ptr<T> GetAs();
        template <typename T>
        std::shared_ptr<T const> const GetAs() const;
    }; 

    struct Object : public Item
    {
        const bool PrintOnNewObjectLine() override { return true; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        void Set(std::string const& name, std::shared_ptr<Item> const item);

        template<typename T>
        std::shared_ptr<T> Get(std::string const& name);
        template<typename T>
        std::shared_ptr<T const> const Get(std::string const& name) const;

        template<typename T>
        T GetValue(std::string const& name);
        template<typename T>
        T const GetValue(std::string const& name) const;

        template<typename T>
        std::shared_ptr<T> TryGet(std::string const& name);
        template<typename T>
        std::shared_ptr<T const> const TryGet(std::string const& name) const;

        template<typename T>
        T TryGetValue(std::string const& name);
        template<typename T>
        T const TryGetValue(std::string const& name) const;

        bool const Has(std::string const& name) const;

        std::unordered_map<std::string, std::shared_ptr<Item>> m_contents;
    };

    struct Array : public Item
    {
        const bool PrintOnNewObjectLine() override { return true; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        void Add(std::shared_ptr<Item> const item) { m_contents.push_back(item); }

        template<typename T>
        std::shared_ptr<T> Get(int const name) const;
        template<typename T>
        std::shared_ptr<T const> const Get(int const name) const;

        std::vector<std::shared_ptr<Item>> m_contents;
    };

    struct Integer : public Item
    {
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
        const bool PrintOnNewObjectLine() override { return false; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        std::string m_value;
    };

    struct Boolean : public Item
    {
        const bool PrintOnNewObjectLine() override { return false; }
        void Parse(std::string const& contents, int& offset) override;
        void BuildContents(std::string& outContents, int& tabLevel) override;

        bool m_value;
    };

    std::shared_ptr<Item> CreateNextItem(std::string const& contents, int const offset);

    // Root template definitions
    template<typename T>
    inline std::shared_ptr<T> Root::GetRootItem()
    {
        return m_item->GetAs<T>();
    }

    // Item template definitions
    template<typename T>
    inline std::shared_ptr<T> Item::GetAs()
    {
        return std::static_pointer_cast<T>(shared_from_this());
    }

    template<typename T>
    inline std::shared_ptr<T const> const Item::GetAs() const
    {
        return std::static_pointer_cast<T const>(shared_from_this());
    }

    // Object template definitions
    template<typename T>
    inline std::shared_ptr<T> Object::Get(std::string const & name)
    {
        return m_contents.at(name)->GetAs<T>();
    }

    template<typename T>
    inline std::shared_ptr<T const> const Object::Get(std::string const & name) const
    {
        return m_contents.at(name)->GetAs<T>();
    }

    // GetValue returns the value of our supported types, otherwise just the default of the requested type
    template<typename T>
    inline T Object::GetValue(std::string const & name) { return T(); }

    template<typename T>
    inline T const Object::GetValue(std::string const & name) const { return T(); }

    template<typename T>
    inline T Object::TryGetValue(std::string const& name) { return T(); }

    template<typename T>
    inline T const Object::TryGetValue(std::string const& name) const { return T(); }

    GET_VALUE_IMPL(int, Integer)
    GET_VALUE_IMPL(float, Float)
    GET_VALUE_IMPL(bool, Boolean)

    template<typename T>
    inline std::shared_ptr<T> Object::TryGet(std::string const& name)
    {
        if (Has(name))
        {
            return Get<T>(name);
        }
        return nullptr;
    }

    template<typename T>
    inline std::shared_ptr<T const> const Object::TryGet(std::string const& name) const
    {
        if (Has(name))
        {
            return Get<T>(name);
        }
        return nullptr;
    }

    template<typename T>
    inline std::shared_ptr<T const> const Array::Get(int const index) const
    {
        return m_contents[index]->GetAs<T>();
    }

}

#undef GET_VALUE_IMPL