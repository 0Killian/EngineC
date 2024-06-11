#include <platform/platform.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // TODO: Make a proper entry point
    printf("INFO: Initializing platform... ");
    u64 size_requirement = 0;
    platform_init(NULL, &size_requirement);
    void *state = malloc(size_requirement);
    platform_init(state, &size_requirement);
    printf("Done\n");
}