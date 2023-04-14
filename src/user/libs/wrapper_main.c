#include "system.h"

int main(void);

void wrapper_main(void)
{
    int ret = main();
    exit(ret);
}
