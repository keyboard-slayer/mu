#include <munix-api/api.h>

void puts(char const *str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }

    mu_log(str, len);
}

int mu_main(MuArgs args)
{
    if (args.arg1 == 0xb00b1e5)
    {
        puts("It works !");
    }
    else
    {
        puts("It doesn't work :(");
    }

    return 0;
}