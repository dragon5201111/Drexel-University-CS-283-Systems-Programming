#include <stdio.h>
#define DECIMAL_DIGITS_BOUND(t) (241 * sizeof(t) / 100 + 1)
#define INT_DIGITS_BOUND (DECIMAL_DIGITS_BOUND(int))
#define DRAGON_LINES (sizeof(DRAGON_ASCII) / sizeof(DRAGON_ASCII[0]))

const char * DRAGON_ASCII[] = {
  "72 1@4%23 ",
  "69 6%25 ",
  "68 6%26 ",
  "65 1%1 7%11 1@14 ",
  "64 10%8 7%11 ",
  "39 7%2 4%1@9 12%1@4 6%2 1@4%8 ",
  "34 22%6 28%10 ",
  "32 26%3 12%1 15%11 ",
  "31 29%1 19%5 3%12 ",
  "29 28%1@1 1@18%8 2%12 ",
  "28 33%1 22%16 ",
  "28 58%14 ",
  "28 50%1@6%1@14 ",
  "6 8%1@11 16%8 26%6 2%16 ",
  "4 13%9 2%1@12%11 11%1 12%6 1@1%16 ",
  "2 10%3 3%8 14%12 24%24 ",
  "1 9%7 1%9 13%13 12%1@11%23 ",
  "9%1@16 1%1 13%12 1@25%21 ",
  "8%1@17 2%1@12%12 1@28%18 ",
  "7%1@19 15%11 33%14 ",
  "10%18 15%10 35%6 4%2 ",
  "9%1@19 1@14%9 12%1@1 4%1 17%3 8%",
  "10%18 17%8 13%6 18%1 9%",
  "9%1@2%1@16 16%1@7 14%5 24%2 2%",
  "1 10%18 1%1 14%1@8 14%3 26%1 2%",
  "2 12%2 1@11 18%8 40%2 3%1 ",
  "3 13%1 2%2 1%2 1%1@1 18%10 37%4 3%1 ",
  "4 18%1 22%11 1@31%4 7%1 ",
  "5 39%14 28%8 3%3 ",
  "6 1@35%18 25%15 ",
  "8 32%22 19%2 7%10 ",
  "11 26%27 15%2 1@9%9 ",
  "14 20%11 1@1%1@1%18 1@18%3 3%8 ",
  "18 15%8 10%20 15%4 1%9 ",
  "16 36%22 14%12 ",
  "16 26%2 4%1 3%22 10%2 3%1@10 ",
  "21 19%1 6%1 2%26 13%1@10 ",
  "81 7%1@7 "
};



// EXTRA CREDIT - print the drexel dragon from the readme.md
extern void print_dragon(){
    const char * current_line;
    
    for (int i = 0; i < DRAGON_LINES; i++) {
        current_line = DRAGON_ASCII[i];

        while (*current_line) {
            char int_s[INT_DIGITS_BOUND];
            long unsigned k = 0;

            while (isdigit(*current_line) && k < INT_DIGITS_BOUND) {
                int_s[k++] = *current_line;
                current_line++;
            }

            int_s[k] = '\0';
            int p_times = atoi(int_s);

            if (*current_line) {
                char c = *(current_line++);

                for (int j = 0; j < p_times; j++) {
                  putchar(c);
                }
            }
        }
        
        putchar('\n');
    }
}
