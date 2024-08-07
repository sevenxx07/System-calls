int isnum(char ch)
{
    return ch >= '0' && ch <= '9';
}
 
int isspc(char ch)
{
    return ch == ' ' || ch == '\n';
}
 
int strlen(char *str)
{
    int len = 0;
     
    for (int i = 0; str[i] != 0; i++) {
        len++;
    }
    return(len);
}
 
void hexconv(char *buf, unsigned num)
{
    buf[0] = '0';
    buf[1] = 'x';
     
    if(num == 0){
        buf[2] = '0';
        buf[3] = '\0';
        buf[4] = '\n';
    }else{
        char hexnum[21];
        int i = 0;
        while(num != 0){
            unsigned temp = 0;
            temp = num % 16;
            hexnum[i++] = (char)(temp + (temp < 10 ? 48 : 87));
            num = num / 16;
        }
        int index = 2;
        for(int j = i - 1; j >= 0; j--){
            buf[index++] = hexnum[j];
        }
        buf[index] = '\0';
        buf[index+1] = '\n';
    }
    return;
 
}
 
int sys_call3(int no, int b, int c, int d){
    int ret;
    asm volatile ("int $0x80": "=a"(ret) : "a"(no), "b"(b), "c"(c), "d"(d) : "memory");
    return ret;
}
 
int sys_call1(int no, int b){
    int ret;
    asm volatile ("int $0x80": "=a"(ret) : "a"(no), "b"(b) : "memory");
    return ret;
}
 
static void print(unsigned num)
{
    char buf[20];
    /* DONE: Get rid of sprintf() and strlen() */
    hexconv(buf, num);
    int ret = sys_call3(4, 1, (int)buf, strlen(buf)+2); 
    if (ret == -1)
        sys_call1(1, -1); // DONE your new exit
}
 
/* DONE: main() is called by libc. Real entry point is _start(). */
int _start()
{
 
    char buf[20];
    unsigned num = 0;
    int i;
    int num_digits = 0;
    unsigned chars_in_buffer = 0;
 
    for (/* no init */; /* no end condition */; i++, chars_in_buffer--) {
        if (chars_in_buffer == 0) {
            //DONE: replace read
            int ret = sys_call3(3, 0, (int)buf, sizeof(buf));
            if (ret < 0)
                sys_call1(1, 1); // DONE replace by exit
            i = 0;
            chars_in_buffer = ret;
        }
        if (
            num_digits > 0
            && (chars_in_buffer == 0 /* EOF */ || !isnum(buf[i]))
        ) {
            print(num);
            num_digits = 0;
            num = 0;
        }
        if (
            chars_in_buffer == 0 /* EOF */
            || (!isspc(buf[i]) && !isnum(buf[i]))
        )
            sys_call1(1, 0); // DONE: replace by exit
 
        if (isnum(buf[i])) {
            num = num * 10 + buf[i] - '0';
            num_digits++;
        }
    }
}
