#include "archiver.h"

void InputManager(size_t argc, char** argv) {
    Parser parser;
    if (argc <= 1) {
        throw Parser::exception("Run with -h for usage information\n");
    }
    parser.Parse(argv[1]);
    switch (parser.GetOperation()) {
        case Parser::Operation::PACK: {
            std::vector<std::string> file_names;
            if (argc <= 2) {
                throw Parser::exception("archive_name not set\n");
            }
            if (argc <= 3) {
                throw Parser::exception("No file_names to archive\n");
            }
            for (size_t i = 3; i < argc; ++i) {
                file_names.emplace_back(argv[i]);
            }
            std::vector<std::ifstream> file_inputs;
            HuffmanPacker packer(file_names);
            packer.Pack(argv[2]);
            break;
        }
        case Parser::Operation::UNPACK: {
            if (argc <= 2) {
                throw Parser::exception("archive_name not set\n");
            }
            HuffmanUnpacker unpacker;
            unpacker.Unpack(argv[2]);
            break;
        }
        case Parser::Operation::HELP: {
            HuffmanBaseClass::PrintHelp(std::cout);
            break;
        }
    }
}

int main(int argc, char** argv) {
    try {
        InputManager(argc, argv);
    } catch (const Parser::exception& exc) {
        std::cout << exc.specification_;
        return 111;
    } catch (const IOHelperBaseClass::exception& exc) {
        std::cout << exc.specification_;
    } catch (const std::exception& exc) {
        std::cerr << exc.what();
        return 111;
    } catch (...) {
        std::cerr << "Unknown exception";
        return 111;
    }
    return 0;
}
