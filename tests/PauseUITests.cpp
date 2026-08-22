#include "Homestead/UI/PauseUI.hpp"
#include "Homestead/UI/StatusUI.hpp"

int main() {
    if (Homestead::PauseItemAt(68, 48) != 0 ||
        Homestead::PauseItemAt(251, 145) != 6 ||
        Homestead::PauseItemAt(67, 48) != -1 ||
        Homestead::PauseItemAt(252, 48) != -1 ||
        Homestead::PauseItemAt(68, 47) != -1 ||
        Homestead::PauseItemAt(68, 146) != -1) return 1;
    if (!Homestead::CompletionContinueAt(126, 92) ||
        !Homestead::CompletionContinueAt(193, 103) ||
        Homestead::CompletionContinueAt(125, 92) ||
        Homestead::CompletionContinueAt(194, 103) ||
        Homestead::CompletionContinueAt(126, 104)) return 2;
    return 0;
}
