#include <Windows.h>
#include <array>

#pragma section(".text")
__declspec(allocate(".text"))
std::array<uint8_t, 0xfffffff> buf{ 0 };

bool DllMain(HINSTANCE base, unsigned long reason, void*) {
    return true;
}