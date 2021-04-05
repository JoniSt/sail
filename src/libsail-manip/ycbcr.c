/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "sail-common.h"

#include "ycbcr.h"

/*
 * Pre-compiled like that:
 *
 *    for (int i = 0; i < 256; i++) {
 *        R_CR[i] = (int)round(1.40200 * (i - 128));
 *        G_CB[i] = (int)round(0.34414 * (i - 128));
 *        G_CR[i] = (int)round(0.71414 * (i - 128));
 *        B_CB[i] = (int)round(1.77200 * (i - 128));
 *    }
 */
static const int R_CR[256] = { -179, -178, -177, -175, -174, -172, -171, -170, -168, -167, -165, -164, -163, -161, -160, -158, -157, -156, -154, -153, -151, -150, -149, -147, -146, -144, -143, -142, -140, -139, -137, -136, -135, -133, -132, -130, -129, -128, -126, -125, -123, -122, -121, -119, -118, -116, -115, -114, -112, -111, -109, -108, -107, -105, -104, -102, -101, -100,  -98,  -97,  -95,  -94,  -93,  -91,  -90,  -88,  -87,  -86,  -84,  -83,  -81,  -80,  -79,  -77,  -76,  -74,  -73,  -72,  -70,  -69,  -67,  -66,  -64,  -63,  -62,  -60,  -59,  -57,  -56,  -55,  -53,  -52,  -50,  -49,  -48,  -46,  -45,  -43,  -42,  -41,  -39,  -38,  -36,  -35,  -34,  -32,  -31,  -29,  -28,  -27,  -25,  -24,  -22,  -21,  -20,  -18,  -17,  -15,  -14,  -13,  -11,  -10,   -8,   -7,   -6,   -4,   -3,   -1,    0,    1,    3,    4,    6,    7,    8,   10,   11,   13,   14,   15,   17,   18,   20,   21,   22,   24,   25,   27,   28,   29,   31,   32,   34,   35,   36,   38,   39,   41,   42,   43,   45,   46,   48,   49,   50,   52,   53,   55,   56,   57,   59,   60,   62,   63,   64,   66,   67,   69,   70,   72,   73,   74,   76,   77,   79,   80,   81,   83,   84,   86,   87,   88,   90,   91,   93,   94,   95,   97,   98,  100,  101,  102,  104,  105,  107,  108,  109,  111,  112,  114,  115,  116,  118,  119,  121,  122,  123,  125,  126,  128,  129,  130,  132,  133,  135,  136,  137,  139,  140,  142,  143,  144,  146,  147,  149,  150,  151,  153,  154,  156,  157,  158,  160,  161,  163,  164,  165,  167,  168,  170,  171,  172,  174,  175,  177,  178 };
static const int G_CB[256] = {  -44,  -44,  -43,  -43,  -43,  -42,  -42,  -42,  -41,  -41,  -41,  -40,  -40,  -40,  -39,  -39,  -39,  -38,  -38,  -38,  -37,  -37,  -36,  -36,  -36,  -35,  -35,  -35,  -34,  -34,  -34,  -33,  -33,  -33,  -32,  -32,  -32,  -31,  -31,  -31,  -30,  -30,  -30,  -29,  -29,  -29,  -28,  -28,  -28,  -27,  -27,  -26,  -26,  -26,  -25,  -25,  -25,  -24,  -24,  -24,  -23,  -23,  -23,  -22,  -22,  -22,  -21,  -21,  -21,  -20,  -20,  -20,  -19,  -19,  -19,  -18,  -18,  -18,  -17,  -17,  -17,  -16,  -16,  -15,  -15,  -15,  -14,  -14,  -14,  -13,  -13,  -13,  -12,  -12,  -12,  -11,  -11,  -11,  -10,  -10,  -10,   -9,   -9,   -9,   -8,   -8,   -8,   -7,   -7,   -7,   -6,   -6,   -6,   -5,   -5,   -4,   -4,   -4,   -3,   -3,   -3,   -2,   -2,   -2,   -1,   -1,   -1,    0,    0,    0,    1,    1,    1,    2,    2,    2,    3,    3,    3,    4,    4,    4,    5,    5,    6,    6,    6,    7,    7,    7,    8,    8,    8,    9,    9,    9,   10,   10,   10,   11,   11,   11,   12,   12,   12,   13,   13,   13,   14,   14,   14,   15,   15,   15,   16,   16,   17,   17,   17,   18,   18,   18,   19,   19,   19,   20,   20,   20,   21,   21,   21,   22,   22,   22,   23,   23,   23,   24,   24,   24,   25,   25,   25,   26,   26,   26,   27,   27,   28,   28,   28,   29,   29,   29,   30,   30,   30,   31,   31,   31,   32,   32,   32,   33,   33,   33,   34,   34,   34,   35,   35,   35,   36,   36,   36,   37,   37,   38,   38,   38,   39,   39,   39,   40,   40,   40,   41,   41,   41,   42,   42,   42,   43,   43,   43,   44 };
static const int G_CR[256] = {  -91,  -91,  -90,  -89,  -89,  -88,  -87,  -86,  -86,  -85,  -84,  -84,  -83,  -82,  -81,  -81,  -80,  -79,  -79,  -78,  -77,  -76,  -76,  -75,  -74,  -74,  -73,  -72,  -71,  -71,  -70,  -69,  -69,  -68,  -67,  -66,  -66,  -65,  -64,  -64,  -63,  -62,  -61,  -61,  -60,  -59,  -59,  -58,  -57,  -56,  -56,  -55,  -54,  -54,  -53,  -52,  -51,  -51,  -50,  -49,  -49,  -48,  -47,  -46,  -46,  -45,  -44,  -44,  -43,  -42,  -41,  -41,  -40,  -39,  -39,  -38,  -37,  -36,  -36,  -35,  -34,  -34,  -33,  -32,  -31,  -31,  -30,  -29,  -29,  -28,  -27,  -26,  -26,  -25,  -24,  -24,  -23,  -22,  -21,  -21,  -20,  -19,  -19,  -18,  -17,  -16,  -16,  -15,  -14,  -14,  -13,  -12,  -11,  -11,  -10,   -9,   -9,   -8,   -7,   -6,   -6,   -5,   -4,   -4,   -3,   -2,   -1,   -1,    0,    1,    1,    2,    3,    4,    4,    5,    6,    6,    7,    8,    9,    9,   10,   11,   11,   12,   13,   14,   14,   15,   16,   16,   17,   18,   19,   19,   20,   21,   21,   22,   23,   24,   24,   25,   26,   26,   27,   28,   29,   29,   30,   31,   31,   32,   33,   34,   34,   35,   36,   36,   37,   38,   39,   39,   40,   41,   41,   42,   43,   44,   44,   45,   46,   46,   47,   48,   49,   49,   50,   51,   51,   52,   53,   54,   54,   55,   56,   56,   57,   58,   59,   59,   60,   61,   61,   62,   63,   64,   64,   65,   66,   66,   67,   68,   69,   69,   70,   71,   71,   72,   73,   74,   74,   75,   76,   76,   77,   78,   79,   79,   80,   81,   81,   82,   83,   84,   84,   85,   86,   86,   87,   88,   89,   89,   90,   91 };
static const int B_CB[256] = { -227, -225, -223, -222, -220, -218, -216, -214, -213, -211, -209, -207, -206, -204, -202, -200, -198, -197, -195, -193, -191, -190, -188, -186, -184, -183, -181, -179, -177, -175, -174, -172, -170, -168, -167, -165, -163, -161, -159, -158, -156, -154, -152, -151, -149, -147, -145, -144, -142, -140, -138, -136, -135, -133, -131, -129, -128, -126, -124, -122, -120, -119, -117, -115, -113, -112, -110, -108, -106, -105, -103, -101,  -99,  -97,  -96,  -94,  -92,  -90,  -89,  -87,  -85,  -83,  -82,  -80,  -78,  -76,  -74,  -73,  -71,  -69,  -67,  -66,  -64,  -62,  -60,  -58,  -57,  -55,  -53,  -51,  -50,  -48,  -46,  -44,  -43,  -41,  -39,  -37,  -35,  -34,  -32,  -30,  -28,  -27,  -25,  -23,  -21,  -19,  -18,  -16,  -14,  -12,  -11,   -9,   -7,   -5,   -4,   -2,    0,    2,    4,    5,    7,    9,   11,   12,   14,   16,   18,   19,   21,   23,   25,   27,   28,   30,   32,   34,   35,   37,   39,   41,   43,   44,   46,   48,   50,   51,   53,   55,   57,   58,   60,   62,   64,   66,   67,   69,   71,   73,   74,   76,   78,   80,   82,   83,   85,   87,   89,   90,   92,   94,   96,   97,   99,  101,  103,  105,  106,  108,  110,  112,  113,  115,  117,  119,  120,  122,  124,  126,  128,  129,  131,  133,  135,  136,  138,  140,  142,  144,  145,  147,  149,  151,  152,  154,  156,  158,  159,  161,  163,  165,  167,  168,  170,  172,  174,  175,  177,  179,  181,  183,  184,  186,  188,  190,  191,  193,  195,  197,  198,  200,  202,  204,  206,  207,  209,  211,  213,  214,  216,  218,  220,  222,  223,  225 };

void convert_ycbcr_to_rgb(uint8_t y, uint8_t cb, uint8_t cr, uint8_t *r, uint8_t *g, uint8_t *b) {

    *r = (uint8_t)(SAIL_MAX(0, SAIL_MIN(255, y            + R_CR[cr])));
    *g = (uint8_t)(SAIL_MAX(0, SAIL_MIN(255, y - G_CB[cb] - G_CR[cr])));
    *b = (uint8_t)(SAIL_MAX(0, SAIL_MIN(255, y + B_CB[cb])));
}
