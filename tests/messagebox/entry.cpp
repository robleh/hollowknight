#include "messagebox.hpp"
#include <al/al.hpp>

using namespace al;

unsigned int entry() {
    auto kernel32 = GM(L"KERNEL32.DLL", by_ror13);
    if (!kernel32) {
        return EXIT_FAILURE;
    }

    auto llw = GP(kernel32, LoadLibraryW, by_djb2);
    if (!llw) {
        return EXIT_FAILURE;
    }

    HMODULE user32 = llw(L"user32.dll"_xor);
    if (!user32) {
        return EXIT_FAILURE;
    }

    auto mbw = GP(user32, MessageBoxW, by_ror13);
    if (!mbw) {
        return EXIT_FAILURE;
    }

    return mbw(nullptr, L"Silk Song 2025", L"Hollow Knight", MB_OKCANCEL);
}