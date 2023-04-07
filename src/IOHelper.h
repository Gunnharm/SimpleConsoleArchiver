#pragma once

#include <fstream>
#include <vector>

class IOHelperBaseClass {
public:
    using WordType = uint16_t;
    static constexpr size_t WordSize = 9;
    static constexpr size_t AllWordsNumber = 259;
    using DoubleWordType = uint32_t;
#define BUF_SIZE CHAR_BIT

    struct BitSequence {
    public:
        using Bit = bool;
        static constexpr Bit BIT1 = true;
        static constexpr Bit BIT0 = false;

        BitSequence();
        // explicit BitSequence(Trie::Node* leaf);
        explicit BitSequence(size_t sz);  // Sequence of sz zeroes
        explicit BitSequence(WordType c);

        void push_back(Bit f);  // NOLINT
        bool Add1();            // Does not increase size, returns false if overflow happened
        void Reverse();
        size_t size() const;  // NOLINT

        bool operator[](size_t ind) const;
        bool friend operator<(const BitSequence& lhs, const BitSequence& rhs);

        void Print() const;

    protected:
        std::vector<Bit> buf_;  // Greatest to smallest
    };

    class exception : std::exception {  // NOLINT
    public:
        explicit exception(const std::string specification);
        std::string specification_;
    };
};

class InputHelper : public IOHelperBaseClass, public std::ifstream {
public:
    using std::ifstream::ifstream;

    WordType GetWord();
    BitSequence::Bit GetBit();

    void Close();

protected:
    void ReadCharToBuf();
    WordType input_buf_ = 0;
    DoubleWordType used_bits_input_ = 0;
};

class OutputHelper : public IOHelperBaseClass, public std::ofstream {
public:
    using std::ofstream::ofstream;

    void PutBit(bool b);
    void PutChar(char c);
    void PutSequence(const BitSequence& sequence);
    void PutWord(WordType c);

    void Close();

protected:
    unsigned char output_buf_ = 0;
    uint8_t used_bits_output_ = 0;
};