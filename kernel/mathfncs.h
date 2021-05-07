#ifndef _MATHFNCS_H_
#define _MATHFNCS_H_

int abs(int x) {
    if (x < 0)
        return -x;
    else
        return x;
}

// x < 0 -> -1
// x == 0 -> 0
// x > 0 -> 1
int sgn(int x) {
    if (x < 0)
        return -1;
    else
        return x == 0 ? 0 : 1;
}

#endif