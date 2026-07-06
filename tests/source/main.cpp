#include <stdio.h>

#include <gccore.h>
#include <wiiuse/wpad.h>

#include "tests.h"
#include "../../include/Revo/api.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void sys_init() {
    VIDEO_Init();
    WPAD_Init();
    
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight,
			rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) {
		VIDEO_WaitVSync();
	}
}

bool sys_exit_check() {
    WPAD_ScanPads();
    return (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) != 0;
}

VIRTUALIZE static void runGroup(const TestGroup &group) {
    constexpr std::string_view ANSI_GREEN = "\x1b[42;37m";
    constexpr std::string_view ANSI_RED   = "\x1b[41;37m";
    constexpr std::string_view ANSI_RESET = "\x1b[40;37m";

    printf("%s:", group.label.data());
    for (const Test &test : group.tests) {
        TestResult result = execute_test(test.id);

        if (result.allPassed()) {
            printf("\n\t%sPASS%s %s", ANSI_GREEN.data(), ANSI_RESET.data(),
                    test.name.data());
            continue;
        }

        printf("\n\t%sFAIL%s %s", ANSI_RED.data(), ANSI_RESET.data(),
                test.name.data());

        for (u32 i = 0; i < result.count; ++i) {
            const SubResult &subresult = result.subresults[i];
            std::string_view color = subresult.passed ? ANSI_RESET : ANSI_RED;
            printf("\n\t\t%s[%u]%s %s - Expected %u, got %u", color.data(), i,
                    ANSI_RESET.data(), test.name.data(), subresult.expected,
                    subresult.got);
        }
    }

    printf("\n\n");
}

VIRTUALIZE int main(int argc, char **argv) {
    sys_init();

    for (const auto &group : testGroups) {
        runGroup(group);
    }

    printf("Press HOME to exit...");
    while(!sys_exit_check()) {
        VIDEO_WaitVSync();
    }

    return 0;
}
