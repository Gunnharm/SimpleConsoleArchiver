#include "archiver.h"
#include "IOHelper.h"
#include "my_priority_queue.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <queue>
#include <sstream>
#include <filesystem>
#include <climits>

// void CreateDirToFile(const std::string& file_name) {
//     std::string path = file_name.substr(0, file_name.rfind('\\') + 1);
//     if (!path.empty()) {
//         std::filesystem::create_directory(path);
//     }
// }

template <typename T>
void CheckOpen(const T& file, const std::string& file_name) {
    if (!file.is_open()) {
        throw IOHelperBaseClass::exception("Cannot open file " + file_name);
    }
}

HuffmanPacker::HuffmanPacker(const std::vector<std::string>& file_names) : file_names_(file_names) {
    // SetFileInputs();
}

void HuffmanPacker::Pack(const std::string& archive_name) {
    OutputHelper archive_output(archive_name, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    CheckOpen(archive_output, archive_name);
    for (size_t id = 0; id < file_names_.size(); ++id) {
        InputHelper file_input(file_names_[id], std::ios_base::in | std::ios_base::binary);
        CheckOpen(file_input, file_names_[id]);
        SetupTrie(file_input, file_names_[id]);
        ExecuteTrie();
        file_input.clear();
        file_input.seekg(0, std::ios::beg);
        assert(file_input.is_open());
        WriteArchive(file_input, file_names_[id], archive_output);
        if (id + 1 != file_names_.size()) {
            trie_.WriteEncodedWord(HuffmanBaseClass::ONE_MORE_FILE, archive_output);
        } else {
            trie_.WriteEncodedWord(HuffmanBaseClass::ARCHIVE_END, archive_output);
        }
        file_input.close();
        trie_.clear();
    }
    archive_output.Close();
}

void HuffmanPacker::SetupTrie(InputHelper& file_input, const std::string& file_name) {
    trie_.AddFileToTrie(file_name, file_input);
    trie_.AddCharToTrie(FILENAME_END);
    trie_.AddCharToTrie(ONE_MORE_FILE);
    trie_.AddCharToTrie(ARCHIVE_END);
}
void HuffmanPacker::WriteArchive(InputHelper& file_input, const std::string& file_name, OutputHelper& archive_output) {
    trie_.WriteTrieData(archive_output);
    trie_.WriteEncodedFile(file_name, file_input, archive_output);
}
void HuffmanPacker::ExecuteTrie() {
    trie_.DoHaffman();
}

Parser::exception::exception(const std::string specification) : specification_(specification) {
}

void Parser::Parse(std::string str) {
    assert(!str.empty());
    if (str[0] != '-') {
        throw exception("First parameter does not define the operation_ (does not start with \"-\")");
    }
    if (str.size() != 2) {
        throw exception("First parameter does not define the operation_ (parameter length is not 2)");
    }
    switch (str[1]) {
        case 'c': {
            operation_ = Operation::PACK;
            break;
        }
        case 'd': {
            operation_ = Operation::UNPACK;
            break;
        }
        case 'h': {
            operation_ = Operation::HELP;
            break;
        }
        default: {
            throw exception("Unknown operation_");
        }
    }
}
Parser::Operation Parser::GetOperation() {
    return operation_;
}

// void HuffmanUnpacker::Unpack(std::string archive_name, IOHelperBaseClass& io) {
//     SetupTrie(archive_name, io);
//     ExecuteTrie(archive_name, io);
// }
void Trie::DoHaffman() {
    auto cmp = [](const Node* const lhs, const Node* const rhs) {
        if (lhs->sz == rhs->sz) {
            return lhs->cmp_char > rhs->cmp_char;
        }
        return lhs->sz > rhs->sz;
    };
    MyPriorityQueue<Node*, std::vector<Node*>, decltype(cmp)> pq(cmp);
    for (auto& [c, v] : node_buf_) {
        // std::cout << c << ", sz = " << v->sz << '\n';
        pq.push(v);
    }
    while (pq.size() > 1) {
        Node* vl = pq.top();
        pq.pop();
        Node* vr = pq.top();
        pq.pop();
        Node* u = new Node(vl, vr);
        vl->parent = u;
        vr->parent = u;
        pq.push(u);
    }
    root = pq.top();
    order_.clear();
    for (auto& [c, v] : node_buf_) {
        order_.push_back({v->GetBitSequence(), c});
    }
    //    std::stable_sort(order_.begin(), order_.end(),
    //                     [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
    std::sort(order_.begin(), order_.end());
    // Print();
    order_ = ToCanonicalForm(order_);
    // Print();
    for (auto& [sequence, c] : order_) {
        encoded_symbols_[c] = sequence;
    }
}

// order should be sorted
std::vector<std::pair<IOHelperBaseClass::BitSequence, IOHelperBaseClass::WordType>> Trie::ToCanonicalForm(
    const std::vector<std::pair<IOHelperBaseClass::BitSequence, IOHelperBaseClass::WordType>>& order) {
    std::vector<std::pair<IOHelperBaseClass::BitSequence, IOHelperBaseClass::WordType>> result;
    IOHelperBaseClass::BitSequence now(order[0].first.size());
    for (size_t ind = 0; ind < order.size(); ++ind) {
        while (now.size() < order[ind].first.size()) {
            now.push_back(IOHelperBaseClass::BitSequence::BIT0);
        }
        result.push_back({now, order[ind].second});
        now.Add1();
    }
    return result;
}
IOHelperBaseClass::BitSequence::BitSequence() {
}
size_t IOHelperBaseClass::BitSequence::size() const {
    return buf_.size();
}
void IOHelperBaseClass::BitSequence::push_back(IOHelperBaseClass::BitSequence::Bit f) {
    buf_.push_back(f);
}
void IOHelperBaseClass::BitSequence::Reverse() {
    std::reverse(buf_.begin(), buf_.end());
}
IOHelperBaseClass::BitSequence::BitSequence(size_t sz) : buf_(sz, false) {
}

bool operator<(const IOHelperBaseClass::BitSequence& lhs, const IOHelperBaseClass::BitSequence& rhs) {
    //    if (lhs.buf_.size() != rhs.buf_.size()) {
    //        return lhs.buf_.size() < rhs.buf_.size();
    //    }
    //    return lhs.buf_ < rhs.buf_;
    return lhs.buf_.size() < rhs.buf_.size();
}
void OutputHelper::PutSequence(const IOHelperBaseClass::BitSequence& sequence) {
    if (sequence.size() == 0) {
        return;
    }
    for (size_t ind = 0; ind < sequence.size(); ++ind) {
        PutBit(sequence[ind]);
    }
}
void OutputHelper::Close() {
    if (!is_open()) {
        return;
    }
    if (used_bits_output_ > 0) {
        assert(used_bits_output_ <= BUF_SIZE);
        output_buf_ >>= (BUF_SIZE - used_bits_output_);
        output_buf_ <<= (BUF_SIZE - used_bits_output_);
        PutChar(static_cast<char>(output_buf_));
    }
    close();
    if (is_open()) {
        throw std::ios_base::failure("Cannot close file");
    }
}
void OutputHelper::PutChar(char c) {
    for (size_t ind = 0; ind < CHAR_BIT; ++ind) {
        PutBit((c >> (ind)) & 1);
    }
}
void OutputHelper::PutBit(bool b) {
    output_buf_ += (b << (BUF_SIZE - 1 - used_bits_output_));
    ++used_bits_output_;
    assert(used_bits_output_ <= BUF_SIZE);
    if (used_bits_output_ == BUF_SIZE) {
        char signed_output_buf = static_cast<char>(output_buf_);
        // put(signed_output_buf);
        write(&signed_output_buf, 1);
        output_buf_ = 0;
        used_bits_output_ = 0;
    }
}
void OutputHelper::PutWord(IOHelperBaseClass::WordType c) {
    PutSequence(BitSequence(c));
}
IOHelperBaseClass::WordType InputHelper::GetWord() {
    while (used_bits_input_ < WordSize) {
        ReadCharToBuf();
    }
    WordType result = 0;
    for (size_t ind = 0; ind < WordSize; ++ind) {
        result ^= static_cast<WordType>(((input_buf_ >> (used_bits_input_ - 1 - ind)) & 1) << (WordSize - 1 - ind));
    }
    used_bits_input_ -= WordSize;
    return result;
}
void InputHelper::ReadCharToBuf() {
    char c = 0;
    if (eof()) {
        throw IOHelperBaseClass::exception("Cannot unzip file");
    }

    // get(c);
    read(&c, 1);
    unsigned char uc = c;
    input_buf_ <<= CHAR_BIT;
    input_buf_ ^= uc;
    used_bits_input_ += CHAR_BIT;
}
IOHelperBaseClass::BitSequence::Bit InputHelper::GetBit() {
    if (used_bits_input_ == 0) {
        ReadCharToBuf();
    }
    BitSequence::Bit result = (input_buf_ >> (used_bits_input_ - 1)) & 1;
    --used_bits_input_;
    return result;
}
void InputHelper::Close() {
    close();
}
void Trie::AddCharToTrie(IOHelperBaseClass::WordType c) {
    auto it = node_buf_.find(c);
    if (it != node_buf_.end()) {
        ++it->second->sz;
    } else {
        node_buf_.insert({c, new Node(c)});
    }
}
void Trie::AddFileToTrie(const std::string& file_path, InputHelper& file_input) {
    size_t slash_pos = file_path.rfind('\\');
    size_t backslash_pos = file_path.rfind('/');
    size_t first_delimiter = 0;
    if (slash_pos != std::string::npos) {
        first_delimiter = std::max(first_delimiter, slash_pos);
    }
    if (backslash_pos != std::string::npos) {
        first_delimiter = std::max(first_delimiter, backslash_pos);
    }

    std::string file_name = file_path.substr(first_delimiter, file_path.size() - first_delimiter);
    for (IOHelperBaseClass::WordType c : file_path) {
        AddCharToTrie(c);
    }
    // AddCharToTrie(HuffmanBaseClass::FILENAME_END);
    char input_byte = 0;
    // file_input >> input_byte;
    // file_input.get(input_byte);
    file_input.read(&input_byte, 1);
    while (!file_input.eof()) {
        unsigned char unsigned_input_byte = input_byte;
        IOHelperBaseClass::WordType c = static_cast<IOHelperBaseClass::WordType>(unsigned_input_byte);
        AddCharToTrie(c);
        // file_input >> input_byte;
        // file_input.get(input_byte);
        file_input.read(&input_byte, 1);
    }
}
void Trie::WriteTrieData(OutputHelper& archive_output) const {
    if (order_.empty()) {
        return;
    }
    size_t symbols_count = encoded_symbols_.size();
    archive_output.PutWord(static_cast<IOHelperBaseClass::WordType>(symbols_count));
    for (const auto& [sequence, c] : order_) {
        archive_output.PutWord(c);
    }
    size_t last_sz = 0;
    size_t cnt_words = 0;
    for (const auto& [sequence, c] : order_) {
        while (sequence.size() != last_sz) {
            if (last_sz > 0) {
                archive_output.PutWord(cnt_words);
            }
            cnt_words = 0;
            ++last_sz;
        }
        ++cnt_words;
    }
    archive_output.PutWord(cnt_words);
    //    for (const auto& [sequence, c] : order_) {
    //        archive_output.PutSequence(sequence);
    //    }
}
void Trie::WriteEncodedWord(IOHelperBaseClass::WordType c, OutputHelper& archive_output) const {
    IOHelperBaseClass::BitSequence sequence(encoded_symbols_.at(c));
    // sequence.Reverse();
    archive_output.PutSequence(sequence);
}
void Trie::WriteEncodedFile(const std::string& file_name, InputHelper& file_input, OutputHelper& archive_output) const {
    for (IOHelperBaseClass::WordType c : file_name) {
        WriteEncodedWord(c, archive_output);
    }
    WriteEncodedWord(HuffmanBaseClass::FILENAME_END, archive_output);
    char c = 0;
    // file_input.get(c);
    file_input.read(&c, 1);
    while (!file_input.eof()) {
        unsigned char uc = c;
        WriteEncodedWord(uc, archive_output);
        // file_input.get(c);
        file_input.read(&c, 1);
    }
}
void Trie::ReadTrieData(InputHelper& archive_input) {
    size_t symbols_count = archive_input.GetWord();
    std::vector<IOHelperBaseClass::WordType> word_order(symbols_count);
    for (size_t i = 0; i < symbols_count; ++i) {
        word_order[i] = archive_input.GetWord();
    }
    size_t sym_cnt_copy = symbols_count;
    std::vector<IOHelperBaseClass::WordType> word_number;
    while (sym_cnt_copy > 0) {
        word_number.push_back(archive_input.GetWord());
        sym_cnt_copy -= word_number.back();
    }
    IOHelperBaseClass::BitSequence now(static_cast<size_t>(1));
    size_t all_words_id = 0;
    for (size_t word_len = 0; word_len < word_number.size(); ++word_len) {
        for (size_t word_id = 0; word_id < word_number[word_len]; ++word_id) {
            order_.push_back({now, word_order[all_words_id]});
            encoded_symbols_[word_order[all_words_id]] = now;
            ++all_words_id;
            now.Add1();
        }
        now.push_back(IOHelperBaseClass::BitSequence::BIT0);
    }
    MakeTrieFromOrder();
}
Trie::~Trie() {
    clear();
}
void Trie::MakeTrieFromOrder() {
    if (root) {
        delete root;
    }
    root = new Node();
    for (const auto& [sequence, c] : order_) {
        AddSequence(sequence, c);
    }
}
void Trie::AddSequence(const IOHelperBaseClass::BitSequence sequence, IOHelperBaseClass::WordType c) {
    Node* v = root;
    ++v->sz;
    for (size_t ind = 0; ind < sequence.size(); ++ind) {
        bool bit = sequence[ind];
        if (bit) {
            if (!v->r) {
                v->r = new Node();
                v->r->parent = v;
            }
            v = v->r;
        } else {
            if (!v->l) {
                v->l = new Node();
                v->l->parent = v;
            }
            v = v->l;
        }
    }
    assert(v->terminate_char == IOHelperBaseClass::AllWordsNumber || v->terminate_char == c);
    v->terminate_char = c;
}
IOHelperBaseClass::WordType Trie::ReadEncodedSymbol(InputHelper& archive_input) const {
    const Node* v = root;
    while (v->terminate_char == IOHelperBaseClass::AllWordsNumber) {
        IOHelperBaseClass::BitSequence::Bit f = archive_input.GetBit();
        if (f) {
            v = v->r;
        } else {
            v = v->l;
        }
    }
    return v->terminate_char;
}
// Trie::Trie() {
//     AddCharToTrie(HuffmanBaseClass::ONE_MORE_FILE);
//     AddCharToTrie(HuffmanBaseClass::ARCHIVE_END);
//     AddCharToTrie(HuffmanBaseClass::FILENAME_END);
//     for (auto& [c, v] : node_buf_) {
//         v->sz = 0;
//     }
// }
void Trie::Print() const {
    for (const auto& [seq, c] : order_) {
        std::cout << c << "(" << static_cast<char>(c) << ")"
                  << " -> ";
        seq.Print();
        std::cout << std::endl;
    }
    std::cout << "!!!!!!!!!!!!\n\n" << std::endl;
}
void Trie::clear() {
    delete root;
    root = nullptr;
    node_buf_.clear();
    encoded_symbols_.clear();
    order_.clear();
}
bool IOHelperBaseClass::BitSequence::Add1() {
    if (buf_.empty()) {
        return false;
    }
    for (ssize_t ind = static_cast<ssize_t>(buf_.size()) - 1; ind >= 0; --ind) {
        if (buf_[ind] == 0) {
            buf_[ind] = BIT1;
            return true;
        } else {
            buf_[ind] = BIT0;
        }
    }
    return false;
}
bool IOHelperBaseClass::BitSequence::operator[](size_t ind) const {
    return buf_[ind];
}
IOHelperBaseClass::BitSequence::BitSequence(IOHelperBaseClass::WordType c) : buf_(WordSize) {
    for (size_t ind = 0; ind < WordSize; ++ind) {
        buf_[ind] = (c >> (WordSize - 1 - ind)) & 1;
    }
}
void IOHelperBaseClass::BitSequence::Print() const {
    std::cout << std::ios_base::boolalpha;
    for (Bit f : buf_) {
        std::cout << f;
    }
}
Trie::Node::Node()
    : l(nullptr),
      r(nullptr),
      parent(nullptr),
      sz(0),
      terminate_char(IOHelperBaseClass::AllWordsNumber),
      cmp_char(IOHelperBaseClass::AllWordsNumber) {
}
Trie::Node::Node(IOHelperBaseClass::WordType c)
    : l(nullptr), r(nullptr), parent(nullptr), sz(1), terminate_char(c), cmp_char(c) {
}
Trie::Node::Node(Trie::Node* l, Trie::Node* r)
    : l(l),
      r(r),
      parent(nullptr),
      sz(l->sz + r->sz),
      terminate_char(IOHelperBaseClass::AllWordsNumber),
      cmp_char(std::min(l->cmp_char, r->cmp_char)) {
}
IOHelperBaseClass::BitSequence Trie::Node::GetBitSequence() {
    // There is at least 2 characters, so leaf is not root and there is a sequence
    Node* u = this;
    IOHelperBaseClass::BitSequence result;
    while (u->parent) {
        Node* par = u->parent;
        if (par->l == u) {
            result.push_back(IOHelperBaseClass::BitSequence::BIT1);
        } else {
            assert(par->r == u);
            result.push_back(IOHelperBaseClass::BitSequence::BIT0);
        }
        u = par;
    }
    result.Reverse();  // Because we add bits from leaf to root
    return result;
}
Trie::Node::~Node() {
    if (l) {
        delete l;
    }
    if (r) {
        delete r;
    }
}
HuffmanUnpacker::HuffmanUnpacker() {
}
void HuffmanUnpacker::Unpack(const std::string& archive_name) {
    InputHelper archive_input(archive_name, std::ios_base::in | std::ios_base::binary);
    CheckOpen(archive_input, archive_name);
    bool flag = true;
    while (flag) {
        trie_.clear();
        SetupTrie(archive_name, archive_input);
        flag = ExecuteTrie(archive_name, archive_input);
    }
    archive_input.close();
}
void HuffmanUnpacker::SetupTrie(const std::string& archive_name, InputHelper& archive_input) {
    trie_.ReadTrieData(archive_input);
}
bool HuffmanUnpacker::ExecuteTrie(const std::string& archive_name, InputHelper& archive_input) {
    WordType input_word = trie_.ReadEncodedSymbol(archive_input);
    std::stringstream file_name_stream;
    while (input_word != FILENAME_END) {
        file_name_stream << static_cast<char>(input_word);
        input_word = trie_.ReadEncodedSymbol(archive_input);
    }
    std::string file_name = file_name_stream.str();
    file_name_stream.clear();
    // CreateDirToFile(file_name);
    OutputHelper file_output(file_name, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    CheckOpen(file_output, file_name);
    input_word = trie_.ReadEncodedSymbol(archive_input);
    while (input_word != ONE_MORE_FILE && input_word != ARCHIVE_END) {
        file_output.put(static_cast<char>(input_word));
        input_word = trie_.ReadEncodedSymbol(archive_input);
    }
    file_output.close();
    return input_word == ONE_MORE_FILE;
}
void HuffmanBaseClass::PrintHelp(std::ostream& out) {
    out << "archiver -c archive_name file1 [file2 ...] : Zip files 'file1', 'file2', ... and save them to a file "
           "'archive_name'\n";
    out << "archiver -d archive_name : Unzip files from 'archive_name' file and store them in the current directory\n";
    out << "archiver -h : Output help on using the program\n";
}
IOHelperBaseClass::exception::exception(const std::string specification) : specification_(specification) {
}
