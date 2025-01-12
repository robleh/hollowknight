#pragma once
#include <string_view>
#include <span>
#include <phnt.h>

using pic_t = void (*)();

class hollower {
    HMODULE           m_ntdll = nullptr;
    HMODULE           m_kernel32 = nullptr;
    HANDLE            m_transaction = nullptr;
    HANDLE            m_file = INVALID_HANDLE_VALUE;
    void*             m_buf = nullptr;
    unsigned long     m_buf_size = 0;
    HANDLE            m_section = nullptr;
    void*             m_view = nullptr;
    uintptr_t         m_va = 0;
    PIMAGE_DOS_HEADER m_dos = nullptr;
    unsigned long     m_pic_offset = 0;

    unsigned long open_file_transaction(std::wstring_view path) noexcept;
    unsigned long read_file() noexcept;
    unsigned long write_file(std::span<uint8_t> pic) noexcept;
    unsigned long map() noexcept;

public:
    hollower() noexcept;
    ~hollower() noexcept;
    unsigned long phantom_hollow(std::wstring_view path, std::span<uint8_t> pic) noexcept;
};
