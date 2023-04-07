#pragma once

// #include <boost>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <array>
#include <map>
#include "IOHelper.h"

class Parser {
public:
    class exception : public std::exception {  // NOLINT
    public:
        explicit exception(const std::string specification);
        std::string specification_;
    };
    enum class Operation { PACK, UNPACK, HELP };

    void Parse(std::string str);
    Operation GetOperation();

protected:
    Operation operation_;
    std::string archive_filename_;
};

class Trie {
protected:
    struct Node {
        Node *l, *r, *parent;  // l is for 0, r is for 1
        uint64_t sz;
        IOHelperBaseClass::WordType terminate_char;
        IOHelperBaseClass::WordType cmp_char;
        Node();
        explicit Node(IOHelperBaseClass::WordType c);
        Node(Node* l, Node* r);
        ~Node();

        IOHelperBaseClass::BitSequence GetBitSequence();

        // friend bool operator< (const Node& const lhs, const Node& const rhs);
    };

public:
    // Trie();

    void AddCharToTrie(IOHelperBaseClass::WordType c);
    // void AddStringToTrie(const std::string& str);
    void AddFileToTrie(const std::string& file_path, InputHelper& file_input);

    void DoHaffman();

    void WriteEncodedFile(const std::string& file_name, InputHelper& file_input, OutputHelper& archive_output) const;
    void WriteEncodedWord(IOHelperBaseClass::WordType c, OutputHelper& archive_output) const;
    void WriteTrieData(OutputHelper& archive_output) const;

    void ReadTrieData(InputHelper& archive_input);
    IOHelperBaseClass::WordType ReadEncodedSymbol(InputHelper& archive_input) const;

    void Print() const;

    void clear();  // NOLINT

    ~Trie();

protected:
    std::vector<std::pair<IOHelperBaseClass::BitSequence, IOHelperBaseClass::WordType>> ToCanonicalForm(
        const std::vector<std::pair<IOHelperBaseClass::BitSequence, IOHelperBaseClass::WordType>>& order);
    void MakeTrieFromOrder();
    void AddSequence(const IOHelperBaseClass::BitSequence sequence, IOHelperBaseClass::WordType c);

    Node* root = nullptr;
    std::map<IOHelperBaseClass::WordType, Node*> node_buf_;  // Updated in Add(*)ToTrie
    std::map<IOHelperBaseClass::WordType, IOHelperBaseClass::BitSequence>
        encoded_symbols_;  // Encoding is greatest to least
    std::vector<std::pair<IOHelperBaseClass::BitSequence, IOHelperBaseClass::WordType>> order_;
};

class HuffmanBaseClass {
public:
    using WordType = IOHelperBaseClass::WordType;
    // static constexpr size_t AllWordsNumber = IOHelperBaseClass::AllWordsNumber;
    static constexpr WordType FILENAME_END = 256;   // NOLINT
    static constexpr WordType ONE_MORE_FILE = 257;  // NOLINT
    static constexpr WordType ARCHIVE_END = 258;    // NOLINT

    static void PrintHelp(std::ostream& out);

protected:
    Trie trie_;
};

class HuffmanPacker : public HuffmanBaseClass {
public:
    explicit HuffmanPacker(const std::vector<std::string>& file_names);

    void Pack(const std::string& archive_name);

protected:
    void SetupTrie(InputHelper& file_input, const std::string& file_name);
    void ExecuteTrie();
    void WriteArchive(InputHelper& file_input, const std::string& file_name, OutputHelper& archive_output);

    std::vector<std::string> file_names_;
};

class HuffmanUnpacker : public HuffmanBaseClass {
public:
    HuffmanUnpacker();

    void Unpack(const std::string& archive_name);

protected:
    void SetupTrie(const std::string& archive_name, InputHelper& archive_input);
    bool ExecuteTrie(const std::string& archive_name, InputHelper& archive_input);
};
