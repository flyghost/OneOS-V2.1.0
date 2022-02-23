#define acw_util_isdigit(c)             (((c) <= '9' && (c) >= '0') ? (1) : (0))

static inline unsigned char hexval_of_char(char hex)
{
    if (acw_util_isdigit(hex)) {
        return (hex - '0');
    }
    if (hex >= 'a' && hex <= 'f') {
        return (hex - 'a' + 10);
    }
    if (hex >= 'A' && hex <= 'F') {
        return (hex - 'A' + 10);
    }

    return 0;
}

static inline void acw_util_hexstr_convert_char(char *input, int input_len, unsigned char *output, int output_len)
{
    unsigned char ch0;
    unsigned char ch1;
    int i;

    i = 0;

    if (input_len % 2 != 0)
    {
        return;
    }

    while (i < input_len / 2 && i < output_len)
    {
        ch0 = hexval_of_char((char)input[2 * i]);
        ch1 = hexval_of_char((char)input[2 * i + 1]);
        output[i] = (ch0 << 4 | ch1);
        i++;
    }

    return;
}

static inline void acw_util_macstr_convert_mac(char *input, unsigned char *mac)
{
    unsigned char ch0;
    unsigned char ch1;
    int index;

    //TODO
    for (index = 0; index < 6; index++)
    {
        ch0 = hexval_of_char(input[3 * index]);
        ch1 = hexval_of_char(input[3 * index + 1]);

        mac[index] = (ch0 << 4 | ch1);
    }

    return;
}