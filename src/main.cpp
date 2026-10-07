#include <iostream>

#include "upp8m/arm7tdmi.h"

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in
// the gutter.

int main() {
    Arm7tdmi cpu{};
    std::cout << "ARM7TDMI Emulator\n" << "!\n";
    std::cout << showRegisterState(cpu) << "\n";
}