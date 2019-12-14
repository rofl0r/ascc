/* The Computer Language Benchmarks Game
 * http://benchmarksgame.alioth.debian.org/
 *
 * converted to C by Joseph Piché
 * from Java version by Oleg Mazurov and Isaac Gouy
 *
 * converted to AGS C dialect by rofl0r
 */

#include "funcdecls.h"

#define LIBRARY
#include "itoa.c"
#include "atoi.c"
#undef LIBRARY

STATIC void print_int(int x) {
	char buf[32];
	itoa(x, buf);
	print(buf);
}

#define MAX_N 64

STATIC int max(int a, int b)
{
    if (a > b) return a;
    return b;
}

int fannkuchredux(int n)
{
    if (n > MAX_N) {
        puts("n too big!");
        return -1;
    }
    int perm[MAX_N];
    int perm1[MAX_N];
    int count[MAX_N];
    int maxFlipsCount = 0;
    int permCount = 0;
    int checksum = 0;

    int i;

    for (i=0; i<n; i+=1)
        perm1[i] = i;
    int r = n;

    while (1) {
        while (r != 1) {
            count[r-1] = r;
            r -= 1;
        }

        for (i=0; i<n; i+=1)
            perm[i] = perm1[i];
        int flipsCount = 0;
        int k;

        while (1) {
            k = perm[0];
            if(k == 0) break;
            int k2 = (k+1) >> 1;
            for (i=0; i<k2; i++) {
                int temp = perm[i]; perm[i] = perm[k-i]; perm[k-i] = temp;
            }
            flipsCount += 1;
        }

        maxFlipsCount = max(maxFlipsCount, flipsCount);
        int tmpflip;
        if (permCount % 2 == 0) tmpflip = flipsCount;
        else tmpflip = -flipsCount;
        checksum += tmpflip;

        /* Use incremental change to generate another permutation */
        while (1) {
            if (r == n) {
                print_int(checksum);
                puts("");
                return maxFlipsCount;
            }

            int perm0 = perm1[0];
            i = 0;
            while (i < r) {
                int j = i + 1;
                perm1[i] = perm1[j];
                i = j;
            }
            perm1[r] = perm0;
            count[r] = count[r] - 1;
            if (count[r] > 0) break;
            r++;
        }
        permCount++;
    }
}

int main(int argc, ARGV argv)
{
    int n = 7;
    if (argc > 1)
        n = atoi(argv[1]);
    print("Pfannkuchen(");
    print_int(n);
    print(") = ");
    print_int(fannkuchredux(n));
    puts("");
    return 0;
}

