//
// Created by GANVAS on 20.10.2022.
//

#include <iostream>
#include "IOHelper.h"

int main() {
    InputHelper fin("multiple_files.arc", std::ios_base::in | std::ios_base::binary);
    for (int i = 0; i < 260 + 15; ++i) {
        try {
            auto c = fin.GetWord();
            // if (i >= 260) {
            std::cout << c << " ";
            //}
        } catch (...) {
            break;
        }
    }
    std::cout << std::endl;
    InputHelper fin2("multiple_files_ans.arc", std::ios_base::in | std::ios_base::binary);
    for (int i = 0; i < 260 + 15; ++i) {
        try {
            auto c = fin2.GetWord();
            // if (i >= 260) {
            std::cout << c << " ";
            //}
        } catch (...) {
            break;
        }
    }
}