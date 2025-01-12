#include "hollowknight.hpp"
#include "phantom.hpp"
#include "payload.hpp"

extern"C"
int entry() {
    hollower hk{};
    return hk.phantom_hollow(LR"()" HOST_DLL, embedded::payload);
}