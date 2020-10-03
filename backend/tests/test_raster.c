/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2020 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"

static int is_row_column_black(struct zint_symbol *symbol, int row, int column) {
    int i;
    if (symbol->output_options & OUT_BUFFER_INTERMEDIATE) {
        i = row * symbol->bitmap_width + column;
        return symbol->bitmap[i] == '1'; // Black
    }
    i = (row * symbol->bitmap_width + column) * 3;
    return symbol->bitmap[i] == 0 && symbol->bitmap[i + 1] == 0 && symbol->bitmap[i + 2] == 0; // Black
}

static void test_options(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        char *fgcolour;
        char *bgcolour;
        int rotate_angle;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123456", "7890AB", 0, "A", 0, 1, 46, 92, 116 },
        /*  1*/ { BARCODE_CODE128, "123456", "7890ab", 90, "A", 0, 1, 46, 116, 92 },
        /*  2*/ { BARCODE_CODE128, NULL, NULL, 180, "A", 0, 1, 46, 92, 116 },
        /*  3*/ { BARCODE_CODE128, NULL, NULL, 270, "A", 0, 1, 46, 116, 92 },
        /*  4*/ { BARCODE_CODE128, NULL, NULL, 181, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  5*/ { BARCODE_CODE128, "12345", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  6*/ { BARCODE_CODE128, NULL, "1234567", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  7*/ { BARCODE_CODE128, "12345 ", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  8*/ { BARCODE_CODE128, NULL, "EEFFGG", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_CODE128, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        if (data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) data[i].data, length, data[i].rotate_angle);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d symbol->bitmap_width %d != %d\n", i, symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d symbol->bitmap_height %d != %d\n", i, symbol->bitmap_height, data[i].expected_bitmap_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_buffer(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        char *data;
        char *composite;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, "1234567890", "", 50, 1, 108, 216, 116 },
        /*  1*/ { BARCODE_C25STANDARD, "1234567890", "", 50, 1, 117, 234, 116 },
        /*  2*/ { BARCODE_C25INTER, "1234567890", "", 50, 1, 99, 198, 116 },
        /*  3*/ { BARCODE_C25IATA, "1234567890", "", 50, 1, 149, 298, 116 },
        /*  4*/ { BARCODE_C25LOGIC, "1234567890", "", 50, 1, 109, 218, 116 },
        /*  5*/ { BARCODE_C25IND, "1234567890", "", 50, 1, 159, 318, 116 },
        /*  6*/ { BARCODE_CODE39, "1234567890", "", 50, 1, 155, 310, 116 },
        /*  7*/ { BARCODE_EXCODE39, "1234567890", "", 50, 1, 155, 310, 116 },
        /*  8*/ { BARCODE_EANX, "123456789012", "", 50, 1, 95, 226, 116 },
        /*  9*/ { BARCODE_EANX_CHK, "1234567890128", "", 50, 1, 95, 226, 116 },
        /* 10*/ { BARCODE_EANX, "123456789012+12", "", 50, 1, 122, 276, 116 },
        /* 11*/ { BARCODE_EANX_CHK, "1234567890128+12", "", 50, 1, 122, 276, 116 },
        /* 12*/ { BARCODE_EANX, "123456789012+12345", "", 50, 1, 149, 330, 116 },
        /* 13*/ { BARCODE_EANX_CHK, "1234567890128+12345", "", 50, 1, 149, 330, 116 },
        /* 14*/ { BARCODE_EANX, "1234567", "", 50, 1, 67, 162, 116 },
        /* 15*/ { BARCODE_EANX_CHK, "12345670", "", 50, 1, 67, 162, 116 },
        /* 16*/ { BARCODE_EANX, "1234567+12", "", 50, 1, 94, 216, 116 },
        /* 17*/ { BARCODE_EANX_CHK, "12345670+12", "", 50, 1, 94, 216, 116 },
        /* 18*/ { BARCODE_EANX, "1234567+12345", "", 50, 1, 121, 270, 116 },
        /* 19*/ { BARCODE_EANX_CHK, "12345670+12345", "", 50, 1, 121, 270, 116 },
        /* 20*/ { BARCODE_EANX, "1234", "", 50, 1, 47, 118, 116 },
        /* 21*/ { BARCODE_EANX_CHK, "1234", "", 50, 1, 47, 118, 116 },
        /* 22*/ { BARCODE_EANX, "12", "", 50, 1, 20, 64, 116 },
        /* 23*/ { BARCODE_EANX_CHK, "12", "", 50, 1, 20, 64, 116 },
        /* 24*/ { BARCODE_GS1_128, "[01]12345678901234", "", 50, 1, 134, 268, 116 },
        /* 25*/ { BARCODE_CODABAR, "A00000000B", "", 50, 1, 102, 204, 116 },
        /* 26*/ { BARCODE_CODE128, "1234567890", "", 50, 1, 90, 180, 116 },
        /* 27*/ { BARCODE_DPLEIT, "1234567890123", "", 50, 1, 135, 270, 116 },
        /* 28*/ { BARCODE_DPIDENT, "12345678901", "", 50, 1, 117, 234, 116 },
        /* 29*/ { BARCODE_CODE16K, "1234567890", "", 20, 2, 70, 162, 44 },
        /* 30*/ { BARCODE_CODE49, "1234567890", "", 20, 2, 70, 162, 44 },
        /* 31*/ { BARCODE_CODE93, "1234567890", "", 50, 1, 127, 254, 116 },
        /* 32*/ { BARCODE_FLAT, "1234567890", "", 50, 1, 90, 180, 100 },
        /* 33*/ { BARCODE_DBAR_OMN, "1234567890123", "", 50, 1, 96, 192, 116 },
        /* 34*/ { BARCODE_DBAR_LTD, "1234567890123", "", 50, 1, 79, 158, 116 },
        /* 35*/ { BARCODE_DBAR_EXP, "[01]12345678901234", "", 34, 1, 134, 268, 84 },
        /* 36*/ { BARCODE_TELEPEN, "1234567890", "", 50, 1, 208, 416, 116 },
        /* 37*/ { BARCODE_UPCA, "12345678901", "", 50, 1, 95, 226, 116 },
        /* 38*/ { BARCODE_UPCA_CHK, "123456789012", "", 50, 1, 95, 226, 116 },
        /* 39*/ { BARCODE_UPCA, "12345678901+12", "", 50, 1, 124, 276, 116 },
        /* 40*/ { BARCODE_UPCA_CHK, "123456789012+12", "", 50, 1, 124, 276, 116 },
        /* 41*/ { BARCODE_UPCA, "12345678901+12345", "", 50, 1, 151, 330, 116 },
        /* 42*/ { BARCODE_UPCA_CHK, "123456789012+12345", "", 50, 1, 151, 330, 116 },
        /* 43*/ { BARCODE_UPCE, "1234567", "", 50, 1, 51, 134, 116 },
        /* 44*/ { BARCODE_UPCE_CHK, "12345670", "", 50, 1, 51, 134, 116 },
        /* 45*/ { BARCODE_UPCE, "1234567+12", "", 50, 1, 78, 184, 116 },
        /* 46*/ { BARCODE_UPCE_CHK, "12345670+12", "", 50, 1, 78, 184, 116 },
        /* 47*/ { BARCODE_UPCE, "1234567+12345", "", 50, 1, 105, 238, 116 },
        /* 48*/ { BARCODE_UPCE_CHK, "12345670+12345", "", 50, 1, 105, 238, 116 },
        /* 49*/ { BARCODE_POSTNET, "12345678901", "", 12, 2, 123, 246, 24 },
        /* 50*/ { BARCODE_MSI_PLESSEY, "1234567890", "", 50, 1, 127, 254, 116 },
        /* 51*/ { BARCODE_FIM, "A", "", 50, 1, 17, 34, 100 },
        /* 52*/ { BARCODE_LOGMARS, "1234567890", "", 50, 1, 191, 382, 116 },
        /* 53*/ { BARCODE_PHARMA, "123456", "", 50, 1, 58, 116, 100 },
        /* 54*/ { BARCODE_PZN, "123456", "", 50, 1, 142, 284, 116 },
        /* 55*/ { BARCODE_PHARMA_TWO, "12345678", "", 10, 2, 29, 58, 20 },
        /* 56*/ { BARCODE_PDF417, "1234567890", "", 21, 7, 103, 206, 42 },
        /* 57*/ { BARCODE_PDF417COMP, "1234567890", "", 21, 7, 69, 138, 42 },
        /* 58*/ { BARCODE_MAXICODE, "1234567890", "", 165, 33, 30, 300, 300 },
        /* 59*/ { BARCODE_QRCODE, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /* 60*/ { BARCODE_CODE128B, "1234567890", "", 50, 1, 145, 290, 116 },
        /* 61*/ { BARCODE_AUSPOST, "12345678901234567890123", "", 8, 3, 133, 266, 16 },
        /* 62*/ { BARCODE_AUSREPLY, "12345678", "", 8, 3, 73, 146, 16 },
        /* 63*/ { BARCODE_AUSROUTE, "12345678", "", 8, 3, 73, 146, 16 },
        /* 64*/ { BARCODE_AUSREDIRECT, "12345678", "", 8, 3, 73, 146, 16 },
        /* 65*/ { BARCODE_ISBNX, "123456789", "", 50, 1, 95, 226, 116 },
        /* 66*/ { BARCODE_ISBNX, "123456789+12", "", 50, 1, 122, 276, 116 },
        /* 67*/ { BARCODE_ISBNX, "123456789+12345", "", 50, 1, 149, 330, 116 },
        /* 68*/ { BARCODE_RM4SCC, "1234567890", "", 8, 3, 91, 182, 16 },
        /* 69*/ { BARCODE_DATAMATRIX, "ABC", "", 10, 10, 10, 20, 20 },
        /* 70*/ { BARCODE_EAN14, "1234567890123", "", 50, 1, 134, 268, 116 },
        /* 71*/ { BARCODE_VIN, "12345678701234567", "", 50, 1, 246, 492, 116 },
        /* 72*/ { BARCODE_CODABLOCKF, "1234567890", "", 20, 2, 101, 242, 44 },
        /* 73*/ { BARCODE_NVE18, "12345678901234567", "", 50, 1, 156, 312, 116 },
        /* 74*/ { BARCODE_JAPANPOST, "1234567890", "", 8, 3, 133, 266, 16 },
        /* 75*/ { BARCODE_KOREAPOST, "123456", "", 50, 1, 167, 334, 116 },
        /* 76*/ { BARCODE_DBAR_STK, "1234567890123", "", 13, 3, 50, 100, 26 },
        /* 77*/ { BARCODE_DBAR_OMNSTK, "1234567890123", "", 69, 5, 50, 100, 138 },
        /* 78*/ { BARCODE_DBAR_EXPSTK, "[01]12345678901234", "", 71, 5, 102, 204, 142 },
        /* 79*/ { BARCODE_PLANET, "12345678901", "", 12, 2, 123, 246, 24 },
        /* 80*/ { BARCODE_MICROPDF417, "1234567890", "", 12, 6, 82, 164, 24 },
        /* 81*/ { BARCODE_USPS_IMAIL, "12345678901234567890", "", 8, 3, 129, 258, 16 },
        /* 82*/ { BARCODE_PLESSEY, "1234567890", "", 50, 1, 227, 454, 116 },
        /* 83*/ { BARCODE_TELEPEN_NUM, "1234567890", "", 50, 1, 128, 256, 116 },
        /* 84*/ { BARCODE_ITF14, "1234567890", "", 50, 1, 135, 330, 136 },
        /* 85*/ { BARCODE_KIX, "123456ABCDE", "", 8, 3, 87, 174, 16 },
        /* 86*/ { BARCODE_AZTEC, "1234567890AB", "", 15, 15, 15, 30, 30 },
        /* 87*/ { BARCODE_DAFT, "DAFTDAFTDAFTDAFT", "", 8, 3, 31, 62, 16 },
        /* 88*/ { BARCODE_DPD, "0123456789012345678901234567", "", 50, 1, 189, 378, 116 },
        /* 89*/ { BARCODE_MICROQR, "12345", "", 11, 11, 11, 22, 22 },
        /* 90*/ { BARCODE_HIBC_128, "1234567890", "", 50, 1, 123, 246, 116 },
        /* 91*/ { BARCODE_HIBC_39, "1234567890", "", 50, 1, 223, 446, 116 },
        /* 92*/ { BARCODE_HIBC_DM, "ABC", "", 12, 12, 12, 24, 24 },
        /* 93*/ { BARCODE_HIBC_QR, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /* 94*/ { BARCODE_HIBC_PDF, "1234567890", "", 24, 8, 103, 206, 48 },
        /* 95*/ { BARCODE_HIBC_MICPDF, "1234567890", "", 28, 14, 38, 76, 56 },
        /* 96*/ { BARCODE_HIBC_BLOCKF, "1234567890", "", 30, 3, 101, 242, 64 },
        /* 97*/ { BARCODE_HIBC_AZTEC, "1234567890AB", "", 19, 19, 19, 38, 38 },
        /* 98*/ { BARCODE_DOTCODE, "ABC", "", 11, 11, 16, 32, 22 },
        /* 99*/ { BARCODE_HANXIN, "1234567890AB", "", 23, 23, 23, 46, 46 },
        /*100*/ { BARCODE_MAILMARK, "01000000000000000AA00AA0A", "", 10, 3, 155, 310, 20 },
        /*101*/ { BARCODE_AZRUNE, "255", "", 11, 11, 11, 22, 22 },
        /*102*/ { BARCODE_CODE32, "12345678", "", 50, 1, 103, 206, 116 },
        /*103*/ { BARCODE_EANX_CC, "123456789012", "[20]01", 50, 7, 99, 234, 116 },
        /*104*/ { BARCODE_EANX_CC, "123456789012+12", "[20]01", 50, 7, 126, 284, 116 },
        /*105*/ { BARCODE_EANX_CC, "123456789012+12345", "[20]01", 50, 7, 153, 338, 116 },
        /*106*/ { BARCODE_EANX_CC, "1234567", "[20]01", 50, 8, 72, 172, 116 },
        /*107*/ { BARCODE_EANX_CC, "1234567+12", "[20]01", 50, 8, 99, 226, 116 },
        /*108*/ { BARCODE_EANX_CC, "1234567+12345", "[20]01", 50, 8, 126, 280, 116 },
        /*109*/ { BARCODE_GS1_128_CC, "[01]12345678901234", "[20]01", 50, 5, 145, 290, 116 },
        /*110*/ { BARCODE_DBAR_OMN_CC, "1234567890123", "[20]01", 21, 5, 100, 200, 58 },
        /*111*/ { BARCODE_DBAR_LTD_CC, "1234567890123", "[20]01", 19, 6, 79, 158, 54 },
        /*112*/ { BARCODE_DBAR_EXP_CC, "[01]12345678901234", "[20]01", 41, 5, 134, 268, 98 },
        /*113*/ { BARCODE_UPCA_CC, "12345678901", "[20]01", 50, 7, 99, 234, 116 },
        /*114*/ { BARCODE_UPCA_CC, "12345678901+12", "[20]01", 50, 7, 128, 284, 116 },
        /*115*/ { BARCODE_UPCA_CC, "12345678901+12345", "[20]01", 50, 7, 155, 338, 116 },
        /*116*/ { BARCODE_UPCE_CC, "1234567", "[20]01", 50, 9, 55, 142, 116 },
        /*117*/ { BARCODE_UPCE_CC, "1234567+12", "[20]01", 50, 9, 82, 192, 116 },
        /*118*/ { BARCODE_UPCE_CC, "1234567+12345", "[20]01", 50, 9, 109, 246, 116 },
        /*119*/ { BARCODE_DBAR_STK_CC, "1234567890123", "[20]01", 24, 9, 56, 112, 48 },
        /*120*/ { BARCODE_DBAR_OMNSTK_CC, "1234567890123", "[20]01", 80, 11, 56, 112, 160 },
        /*121*/ { BARCODE_DBAR_EXPSTK_CC, "[01]12345678901234", "[20]01", 78, 9, 102, 204, 156 },
        /*122*/ { BARCODE_CHANNEL, "01", "", 50, 1, 19, 38, 116 },
        /*123*/ { BARCODE_CODEONE, "12345678901234567890", "", 22, 22, 22, 44, 44 },
        /*124*/ { BARCODE_GRIDMATRIX, "ABC", "", 18, 18, 18, 36, 36 },
        /*125*/ { BARCODE_UPNQR, "1234567890AB", "", 77, 77, 77, 154, 154 },
        /*126*/ { BARCODE_ULTRA, "1234567890", "", 13, 13, 18, 36, 26 },
        /*127*/ { BARCODE_RMQR, "12345", "", 11, 11, 27, 54, 22 },
    };
    int data_size = ARRAY_SIZE(data);

    char *text;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d\n", i);

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int length = strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", \"%s\", %d, %d, %d, %d, %d },\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].data, data[i].composite,
                    symbol->height, symbol->rows, symbol->width, symbol->bitmap_width, symbol->bitmap_height);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_upcean_hrt(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int show_hrt;
        char *data;
        int ret;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_text_row;
        int expected_text_col;
        int expected_text_len;
        int expected_addon_text_row;
        int expected_addon_text_col;
        int expected_addon_text_len;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  1*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 226, 110, 102 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  2*/ { BARCODE_EANX_CHK, -1, "1234567890128", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  3*/ { BARCODE_EANX_CHK, 0, "1234567890128", 0, 50, 1, 95, 226, 110, 102 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  4*/ { BARCODE_EANX_CHK, -1, "1234567890128+12", 0, 50, 1, 122, 276, 116, 102 /*text_row*/, 0, 20, 5, 212, 64 }, // EAN-13 + EAN-2
        /*  5*/ { BARCODE_EANX_CHK, 0, "1234567890128+12", 0, 50, 1, 122, 276, 110, 102 /*text_row*/, 0, 20, 5, 212, 64 }, // EAN-13 + EAN-2
        /*  6*/ { BARCODE_EANX, -1, "1234567890128+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 20, 5, 212, 118 }, // EAN-13 + EAN-5
        /*  7*/ { BARCODE_EANX, 0, "1234567890128+12345", 0, 50, 1, 149, 330, 110, 102 /*text_row*/, 0, 20, 5, 212, 118 }, // EAN-13 + EAN-5
        /*  8*/ { BARCODE_ISBNX, -1, "9784567890120+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 20, 5, 212, 118 }, // ISBNX + EAN-5
        /*  9*/ { BARCODE_ISBNX, 0, "9784567890120+12345", 0, 50, 1, 149, 330, 110, 102 /*text_row*/, 0, 20, 5, 212, 118 }, // ISBNX + EAN-5
        /* 10*/ { BARCODE_EANX, -1, "123456", 0, 50, 1, 67, 162, 116, 102 /*text_row*/, 20, 58, -1, -1, -1 }, // EAN-8
        /* 11*/ { BARCODE_EANX, 0, "123456", 0, 50, 1, 67, 162, 110, 102 /*text_row*/, 20, 58, -1, -1, -1 }, // EAN-8
        /* 12*/ { BARCODE_EANX, -1, "123456+12", 0, 50, 1, 94, 216, 116, 102 /*text_row*/, 20, 58, 5, 148, 68 }, // EAN-8 + EAN-2
        /* 13*/ { BARCODE_EANX, 0, "123456+12", 0, 50, 1, 94, 216, 110, 102 /*text_row*/, 20, 58, 5, 148, 68 }, // EAN-8 + EAN-2
        /* 14*/ { BARCODE_EANX, -1, "123456+12345", 0, 50, 1, 121, 270, 116, 102 /*text_row*/, 20, 58, 5, 148, 122 }, // EAN-8 + EAN-5
        /* 15*/ { BARCODE_EANX, 0, "123456+12345", 0, 50, 1, 121, 270, 110, 102 /*text_row*/, 20, 58, 5, 148, 122 }, // EAN-8 + EAN-5
        /* 16*/ { BARCODE_EANX, -1, "1234", 0, 50, 1, 47, 118, 116, 102 /*text_row*/, 40, 36, -1, -1, -1 }, // EAN-5
        /* 17*/ { BARCODE_EANX, 0, "1234", 0, 50, 1, 47, 118, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 }, // EAN-5
        /* 18*/ { BARCODE_EANX, -1, "12", 0, 50, 1, 20, 64, 116, 102 /*text_row*/, 20, 20, -1, -1, -1 }, // EAN-2
        /* 19*/ { BARCODE_EANX, 0, "12", 0, 50, 1, 20, 64, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 }, // EAN-2
        /* 20*/ { BARCODE_UPCA, -1, "123456789012", 0, 50, 1, 95, 226, 116, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 21*/ { BARCODE_UPCA, 0, "123456789012", 0, 50, 1, 95, 226, 110, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 22*/ { BARCODE_UPCA, -1, "123456789012+12", 0, 50, 1, 124, 276, 116, 104 /*text_row*/, 0, 18, 5, 208, 68 },
        /* 23*/ { BARCODE_UPCA, 0, "123456789012+12", 0, 50, 1, 124, 276, 110, 104 /*text_row*/, 0, 18, 5, 208, 68 },
        /* 24*/ { BARCODE_UPCA_CHK, -1, "123456789012+12345", 0, 50, 1, 151, 330, 116, 104 /*text_row*/, 0, 18, 5, 208, 122 },
        /* 25*/ { BARCODE_UPCA_CHK, 0, "123456789012+12345", 0, 50, 1, 151, 330, 110, 104 /*text_row*/, 0, 18, 5, 208, 122 },
        /* 26*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 134, 116, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 27*/ { BARCODE_UPCE, 0, "1234567", 0, 50, 1, 51, 134, 110, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 28*/ { BARCODE_UPCE_CHK, -1, "12345670+12", 0, 50, 1, 78, 184, 116, 104 /*text_row*/, 0, 18, 5, 120, 64 },
        /* 29*/ { BARCODE_UPCE_CHK, 0, "12345670+12", 0, 50, 1, 78, 184, 110, 104 /*text_row*/, 0, 18, 5, 120, 64 },
        /* 30*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 105, 238, 116, 104 /*text_row*/, 0, 18, 5, 120, 118 },
        /* 31*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 105, 238, 110, 104 /*text_row*/, 0, 18, 5, 120, 118 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);

        assert_nonzero(symbol->bitmap_height > data[i].expected_text_row, "i:%d symbol->bitmap_height %d <= data[i].expected_text_row %d\n", i, symbol->bitmap_height, data[i].expected_text_row);
        if (data[i].expected_text_row == -1) { /* EAN-2/5 just truncates bitmap if no text */
            assert_zero(data[i].show_hrt, "i:%d Expected text row -1 but show_hrt set\n", i);
            if (i && data[i - 1].symbology == symbol->symbology && data[i - 1].show_hrt && data[i - 1].expected_text_row != -1) {
                assert_nonzero(data[i].expected_bitmap_height < data[i - 1].expected_text_row, "i:%d (%s) expected_bitmap_height %d >= previous expected_text_row %d\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].expected_bitmap_height, data[i - 1].expected_text_row);
            }
        } else {
            int text_bits_set = 0;
            int row = data[i].expected_text_row;
            for (int column = data[i].expected_text_col; column < data[i].expected_text_col + data[i].expected_text_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    text_bits_set = 1;
                    break;
                }
            }
            if (symbol->show_hrt) {
                assert_nonzero(text_bits_set, "i:%d (%s) text_bits_set zero\n", i, testUtilBarcodeName(data[i].symbology));
            } else {
                assert_zero(text_bits_set, "i:%d (%s) text_bits_set non-zero\n", i, testUtilBarcodeName(data[i].symbology));
            }
        }

        if (data[i].expected_addon_text_row != -1) {
            int addon_text_bits_set = 0;
            int row = data[i].expected_addon_text_row;
            for (int column = data[i].expected_addon_text_col; column < data[i].expected_addon_text_col + data[i].expected_addon_text_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    addon_text_bits_set = 1;
                    break;
                }
            }
            if (symbol->show_hrt) {
                assert_nonzero(addon_text_bits_set, "i:%d (%s) addon_text_bits_set zero\n", i, testUtilBarcodeName(data[i].symbology));
            } else {
                assert_zero(addon_text_bits_set, "i:%d (%s) addon_text_bits_set non-zero\n", i, testUtilBarcodeName(data[i].symbology));
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_row_separator(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int border_width;
        int option_1;
        int option_3;
        char *data;
        int ret;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_separator_row;
        int expected_separator_col;
        int expected_separator_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, -1, -1, -1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 },
        /*  1*/ { BARCODE_CODABLOCKF, -1, -1, 0, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // Same as default
        /*  2*/ { BARCODE_CODABLOCKF, -1, -1, 1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // Same as default
        /*  3*/ { BARCODE_CODABLOCKF, -1, -1, 2, "A", 0, 20, 2, 101, 242, 44, 20, 42, 4 },
        /*  4*/ { BARCODE_CODABLOCKF, -1, -1, 3, "A", 0, 20, 2, 101, 242, 44, 19, 42, 6 },
        /*  5*/ { BARCODE_CODABLOCKF, -1, -1, 4, "A", 0, 20, 2, 101, 242, 44, 18, 42, 8 },
        /*  6*/ { BARCODE_CODABLOCKF, -1, -1, 5, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // > 4 ignored, same as default
        /*  7*/ { BARCODE_CODABLOCKF, -1, 1, -1, "A", 0, 5, 1, 46, 132, 14, 0, 20 + 2, 2 }, // CODE128 top separator, add 2 to skip over end of start char; note no longer includes HRT
        /*  8*/ { BARCODE_CODABLOCKF, 0, -1, -1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // Border width zero, same as default
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }

        ret = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%s) symbol->bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n", i, testUtilBarcodeName(data[i].symbology),
                    symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n", i, testUtilBarcodeName(data[i].symbology),
                    symbol->bitmap_height, data[i].expected_bitmap_height);

        int j, separator_bits_set;

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol);

        for (j = data[i].expected_separator_row; j < data[i].expected_separator_row + data[i].expected_separator_height; j++) {
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
            assert_nonzero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);
        }

        if (symbol->rows > 1) {
            j = data[i].expected_separator_row - 1;
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col + 2); // Need to add 2 to skip to 1st blank of start row character
            assert_zero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) before non-zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);
        }

        j = data[i].expected_separator_row + data[i].expected_separator_height;
        separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col + 2); // Need to add 2 to skip to 1st blank of start row character
        assert_zero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) after non-zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_stacking(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int output_options;
        int option_1;
        int option_3;
        char *data;
        char *data2;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_separator_row;
        int expected_separator_col;
        int expected_separator_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, "A", "B", 50, 2, 46, 92, 116, -1, -1, -1 },
        /*  1*/ { BARCODE_CODE128, BARCODE_BIND, -1, -1, "A", "B", 50, 2, 46, 92, 116, 49, 4, 2 },
        /*  2*/ { BARCODE_CODE128, BARCODE_BIND, -1, 2, "A", "B", 50, 2, 46, 92, 116, 48, 4, 4 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, data[i].output_options, data[i].data, -1, debug);
        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);

        int length2 = strlen(data[i].data2);
        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data2, length2);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        int j, separator_bits_set;

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol);

        if (data[i].expected_separator_row != -1) {
            for (j = data[i].expected_separator_row; j < data[i].expected_separator_row + data[i].expected_separator_height; j++) {
                separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
                assert_nonzero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
            }

            if (symbol->rows > 1) {
                j = data[i].expected_separator_row - 1;
                separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
                assert_zero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) before non-zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
            }

            j = data[i].expected_separator_row + data[i].expected_separator_height;
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
            assert_zero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) after non-zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_output_options(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int whitespace_width;
        int border_width;
        int output_options;
        int rotate_angle;
        char *data;
        int ret;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_set;
        int expected_set_row;
        int expected_set_col;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  1*/ { BARCODE_CODE128, -1, -1, -1, 180, "A123", 0, 50, 1, 79, 158, 116, 0, 115, 4 },
        /*  2*/ { BARCODE_CODE128, -1, 2, -1, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  3*/ { BARCODE_CODE128, -1, 2, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 158, 124, 1, 0, 4 },
        /*  4*/ { BARCODE_CODE128, -1, 2, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 158, 124, 0, 4, 4 },
        /*  5*/ { BARCODE_CODE128, -1, 2, BARCODE_BOX, 0, "A123", 0, 50, 1, 79, 166, 124, 1, 4, 4 },
        /*  6*/ { BARCODE_CODE128, -1, 0, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  7*/ { BARCODE_CODE128, -1, 0, BARCODE_BOX, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 4, 4 },
        /*  8*/ { BARCODE_CODE128, -1, -1, -1, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 8 },
        /*  9*/ { BARCODE_CODE128, 3, -1, -1, 0, "A123", 0, 50, 1, 79, 170, 116, 1, 0, 8 },
        /* 10*/ { BARCODE_CODE128, 3, 4, -1, 0, "A123", 0, 50, 1, 79, 170, 116, 1, 0, 8 },
        /* 11*/ { BARCODE_CODE128, 3, 4, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 170, 132, 1, 0, 0 },
        /* 12*/ { BARCODE_CODE128, 3, 4, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 170, 132, 0, 8, 0 },
        /* 13*/ { BARCODE_CODE128, 3, 4, BARCODE_BOX, 0, "A123", 0, 50, 1, 79, 186, 132, 1, 8, 0 },
        /* 14*/ { BARCODE_CODE128, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 15*/ { BARCODE_CODE128, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /* 16*/ { BARCODE_CODE128, -1, -1, OUT_BUFFER_INTERMEDIATE, 180, "A123", 0, 50, 1, 79, 158, 116, 0, 115, 4 },
        /* 17*/ { BARCODE_CODE128, 3, 4, BARCODE_BOX | OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 50, 1, 79, 186, 132, 1, 8, 0 },
        /* 18*/ { BARCODE_QRCODE, -1, -1, -1, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 19*/ { BARCODE_QRCODE, -1, -1, -1, 180, "A123", 0, 21, 21, 21, 42, 42, 0, 39, 2 },
        /* 20*/ { BARCODE_QRCODE, -1, 3, -1, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 21*/ { BARCODE_QRCODE, -1, 3, BARCODE_BIND, 0, "A123", 0, 21, 21, 21, 42, 54, 1, 2, 2 },
        /* 22*/ { BARCODE_QRCODE, -1, 3, BARCODE_BIND, 0, "A123", 0, 21, 21, 21, 42, 54, 0, 20, 0 },
        /* 23*/ { BARCODE_QRCODE, -1, 3, BARCODE_BOX, 0, "A123", 0, 21, 21, 21, 54, 54, 1, 20, 0 },
        /* 24*/ { BARCODE_QRCODE, -1, -1, -1, 0, "A123", 0, 21, 21, 21, 42, 42, 1, 0, 0 },
        /* 25*/ { BARCODE_QRCODE, 5, -1, -1, 0, "A123", 0, 21, 21, 21, 62, 42, 0, 0, 0 },
        /* 26*/ { BARCODE_QRCODE, 5, 6, -1, 0, "A123", 0, 21, 21, 21, 62, 42, 0, 0, 0 },
        /* 27*/ { BARCODE_QRCODE, 5, 6, BARCODE_BIND, 0, "A123", 0, 21, 21, 21, 62, 66, 1, 0, 0 },
        /* 28*/ { BARCODE_QRCODE, 5, 6, BARCODE_BIND, 0, "A123", 0, 21, 21, 21, 62, 66, 0, 12, 0 },
        /* 29*/ { BARCODE_QRCODE, 5, 6, BARCODE_BOX, 0, "A123", 0, 21, 21, 21, 86, 66, 1, 12, 0 },
        /* 30*/ { BARCODE_QRCODE, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 42, 42, 1, 0, 0 },
        /* 31*/ { BARCODE_QRCODE, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 1, 1 },
        /* 32*/ { BARCODE_QRCODE, -1, 4, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 42, 42, 1, 0, 0 },
        /* 33*/ { BARCODE_QRCODE, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 42, 58, -1, -1, -1 }, // TODO: fix (bind/box in dotty mode)
        /* 34*/ { BARCODE_QRCODE, 1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 62, 58, -1, -1, -1 }, // TODO: fix (bind/box in dotty mode)
        /* 35*/ { BARCODE_QRCODE, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 36*/ { BARCODE_QRCODE, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 21, 21, 21, 42, 42, 1, 0, 0 },
        /* 37*/ { BARCODE_QRCODE, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 180, "A123", 0, 21, 21, 21, 42, 42, 0, 40, 1 },
        /* 38*/ { BARCODE_MAXICODE, -1, -1, -1, 0, "A123", 0, 165, 33, 30, 300, 300, 0, 0, 0 },
        /* 39*/ { BARCODE_MAXICODE, -1, -1, -1, 270, "A123", 0, 165, 33, 30, 300, 300, 0, 0, 0 },
        /* 40*/ { BARCODE_MAXICODE, -1, 5, -1, 0, "A123", 0, 165, 33, 30, 300, 300, 0, 0, 0 },
        /* 41*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 300, 320, 1, 0, 0 },
        /* 42*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 300, 320, 0, 10, 0 },
        /* 43*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BOX, 0, "A123", 0, 165, 33, 30, 320, 320, 1, 10, 0 },
        /* 44*/ { BARCODE_MAXICODE, -1, -1, -1, 0, "A123", 0, 165, 33, 30, 300, 300, 1, 0, 14 },
        /* 45*/ { BARCODE_MAXICODE, 6, -1, -1, 0, "A123", 0, 165, 33, 30, 324, 300, 0, 0, 14 },
        /* 46*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 324, 320, 1, 10, 25 },
        /* 47*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 324, 320, 0, 10, 9 },
        /* 48*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BOX, 0, "A123", 0, 165, 33, 30, 344, 320, 1, 10, 9 },
        /* 49*/ { BARCODE_MAXICODE, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 50*/ { BARCODE_MAXICODE, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 165, 33, 30, 300, 300, 0, 0, 0 },
        /* 51*/ { BARCODE_MAXICODE, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 165, 33, 30, 300, 300, 1, 0, 14 },
        /* 52*/ { BARCODE_MAXICODE, -1, -1, OUT_BUFFER_INTERMEDIATE, 270, "A123", 0, 165, 33, 30, 300, 300, 0, 0, 0 },
        /* 53*/ { BARCODE_ITF14, -1, -1, -1, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 54*/ { BARCODE_ITF14, -1, -1, -1, 90, "123", 0, 50, 1, 135, 136, 330, 1, 0, 110 },
        /* 55*/ { BARCODE_ITF14, -1, 0, -1, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 56*/ { BARCODE_ITF14, -1, 0, BARCODE_BOX, 0, "123", 0, 50, 1, 135, 310, 116, 0, 100, 0 },
        /* 57*/ { BARCODE_ITF14, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 58*/ { BARCODE_ITF14, -1, -1, OUT_BUFFER_INTERMEDIATE, 90, "123", 0, 50, 1, 135, 136, 330, 1, 0, 110 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, data[i].rotate_angle);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_nonnull(symbol->bitmap, "i:%d (%s) symbol->bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

            if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol);

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n", i, testUtilBarcodeName(data[i].symbology),
                        symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n", i, testUtilBarcodeName(data[i].symbology),
                        symbol->bitmap_height, data[i].expected_bitmap_height);

            if (data[i].expected_set != -1) {
                assert_nonzero(data[i].expected_set_row < data[i].expected_bitmap_height, "i:%d (%s) expected_set_row %d >= expected_bitmap_height %d\n",
                        i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_row, data[i].expected_bitmap_height);
                ret = is_row_column_black(symbol, data[i].expected_set_row, data[i].expected_set_col);
                if (data[i].expected_set) {
                    assert_nonzero(ret, "i:%d (%s) is_row_column_black(%d, %d) zero\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_row, data[i].expected_set_col);
                } else {
                    assert_zero(ret, "i:%d (%s) is_row_column_black(%d, %d) non-zero\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_row, data[i].expected_set_col);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_draw_string_wrap(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int output_options;
        char *data;
        char* text;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_no_text_row;
        int expected_no_text_col;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, "12", "              E", 50, 1, 46, 92, 116, 104, 0 },
        /*  1*/ { BARCODE_CODE128, BOLD_TEXT, "12", "           E", 50, 1, 46, 92, 116, 104, 0 },
        /*  2*/ { BARCODE_CODE128, SMALL_TEXT, "12", "                   E", 50, 1, 46, 92, 112, 103, 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        // Cheat by overwriting text
        strcpy((char *) symbol->text, data[i].text);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol);

        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Print(%d) ret %d != 0\n", i, data[i].symbology, ret);

        int text_bits_set = 0;
        int row = data[i].expected_no_text_row;
        for (int column = data[i].expected_no_text_col; column < data[i].expected_no_text_col + 16; column++) {
            if (is_row_column_black(symbol, row, column)) {
                text_bits_set = 1;
                break;
            }
        }
        assert_zero(text_bits_set, "i:%d (%d) text_bits_set non-zero\n", i, data[i].symbology);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_code128_utf8(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        char *data;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_text_row;
        int expected_text_col;
        int expected_text_len;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "é", 50, 1, 57, 114, 116, 110, 53, 6 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_CODE128, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, BARCODE_CODE128, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0\n", i, BARCODE_CODE128, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, BARCODE_CODE128);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, BARCODE_CODE128, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, BARCODE_CODE128, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, BARCODE_CODE128, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, BARCODE_CODE128, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, BARCODE_CODE128, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol);

        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Print(%d) ret %d != 0\n", i, BARCODE_CODE128, ret);

        int text_bits_set = 0;
        int row = data[i].expected_text_row;
        for (int column = data[i].expected_text_col; column < data[i].expected_text_col + data[i].expected_text_len; column++) {
            if (is_row_column_black(symbol, row, column)) {
                text_bits_set++;
            }
        }
        assert_equal(text_bits_set, data[i].expected_text_len, "i:%d (%d) text_bits_set %d != expected_text_len %d\n", i, BARCODE_CODE128, text_bits_set, data[i].expected_text_len);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_scale(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int option_2;
        int border_width;
        int output_options;
        float scale;
        char *data;
        char *composite;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_set_row;
        int expected_set_rows;
        int expected_set_col;
        int expected_set_len;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, 0, "1", "", 18, 6, 103, 206, 36, 0, 36, 170, 14 }, // With no scaling
        /*  1*/ { BARCODE_PDF417, -1, -1, -1, 0.6, "1", "", 18, 6, 103, 206 * 0.6, 36 * 0.6, 0 /*set_row*/, 36 * 0.6, 170 * 0.6 + 1, 14 * 0.6 }, // +1 set_col due to some scaling inversion difference
        /*  2*/ { BARCODE_PDF417, -1, -1, -1, 1.2, "1", "", 18, 6, 103, 206 * 1.2, 36 * 1.2, 0 /*set_row*/, 36 * 1.2, 170 * 1.2 + 1, 14 * 1.2 }, // +1 set_col due to some scaling inversion difference
        /*  3*/ { BARCODE_PDF417, -1, -1, -1, 0.5, "1", "", 18, 6, 103, 206 * 0.5, 36 * 0.5, 0 /*set_row*/, 36 * 0.5, 170 * 0.5, 14 * 0.5 },
        /*  4*/ { BARCODE_PDF417, -1, -1, -1, 1.0, "1", "", 18, 6, 103, 206 * 1.0, 36 * 1.0, 0 /*set_row*/, 36 * 1.0, 170 * 1.0, 14 * 1.0 },
        /*  5*/ { BARCODE_PDF417, -1, -1, -1, 1.5, "1", "", 18, 6, 103, 206 * 1.5, 36 * 1.5, 0 /*set_row*/, 36 * 1.5, 170 * 1.5, 14 * 1.5 },
        /*  6*/ { BARCODE_PDF417, -1, -1, -1, 2.0, "1", "", 18, 6, 103, 206 * 2.0, 36 * 2.0, 0 /*set_row*/, 36 * 2.0, 170 * 2.0, 14 * 2.0 },
        /*  7*/ { BARCODE_PDF417, -1, -1, -1, 2.5, "1", "", 18, 6, 103, 206 * 2.5, 36 * 2.5, 0 /*set_row*/, 36 * 2.5, 170 * 2.5, 14 * 2.5 },
        /*  8*/ { BARCODE_PDF417, -1, -1, -1, 3.0, "1", "", 18, 6, 103, 206 * 3.0, 36 * 3.0, 0 /*set_row*/, 36 * 3.0, 170 * 3.0, 14 * 3.0 },
        /*  9*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, "1", "", 18, 6, 103, 218, 48, 0 /*set_row*/, 48, 176, 14 }, // With no scaling
        /* 10*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0.6, "1", "", 18, 6, 103, 218 * 0.6, 48 * 0.6, 0 /*set_row*/, 48 * 0.6, 176 * 0.6 + 1, 14 * 0.6 }, // +1 set_col due to some scaling inversion difference
        /* 11*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 1.6, "1", "", 18, 6, 103, 218 * 1.6, 48 * 1.6, 0 /*set_row*/, 48 * 1.6, 176 * 1.6 + 1, 14 * 1.6 }, // +1 set_col due to some scaling inversion difference
        /* 12*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 1.5, "1", "", 18, 6, 103, 218 * 1.5, 48 * 1.5, 0 /*set_row*/, 48 * 1.5, 176 * 1.5, 14 * 1.5 },
        /* 13*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 2.5, "1", "", 18, 6, 103, 218 * 2.5, 48 * 2.5, 0 /*set_row*/, 48 * 2.5, 176 * 2.5, 14 * 2.5 },
        /* 14*/ { BARCODE_PDF417, -1, 3, OUT_BUFFER_INTERMEDIATE, 1.3, "1", "", 18, 6, 103, 206 * 1.3, 36 * 1.3, 0 /*set_row*/, 36 * 1.3, 170 * 1.3 + 1, 14 * 1.3 }, // +1 set_col due to some scaling inversion difference
        /* 15*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, "123456789012", "", 50, 1, 79, 158, 116, 104 /*set_row*/, 114, 20, 2 }, // With no scaling
        /* 16*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 1.5, "123456789012", "", 50, 1, 79, 158 * 1.5, 116 * 1.5, 104 * 1.5 /*set_row*/, 114 * 1.5, 20 * 1.5, 1 * 1.5 },
        /* 17*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 2.0, "123456789012", "", 50, 1, 79, 158 * 2.0, 116 * 2.0, 104 * 2.0 /*set_row*/, 114 * 2.0, 20 * 2.0, 1 * 2.0 },
        /* 18*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 3.5, "123456789012", "", 50, 1, 79, 158 * 3.5, 116 * 3.5, 104 * 3.5 /*set_row*/, 114 * 3.5, 20 * 3.5, 1 * 3.5 },
        /* 19*/ { BARCODE_UPCA, -1, -1, -1, 0, "12345678904", "", 50, 1, 95, 226, 116, 104 /*set_row*/, 114, 5, 2 }, // With no scaling
        /* 20*/ { BARCODE_UPCA, -1, -1, -1, 2.5, "12345678904", "", 50, 1, 95, 226 * 2.5, 116 * 2.5, 104 * 2.5 /*set_row*/, 114 * 2.5, 5 * 2.5, 2 * 2.5 },
        /* 21*/ { BARCODE_UPCA, -1, -1, -1, 4.5, "12345678904", "", 50, 1, 95, 226 * 4.5, 116 * 4.5, 104 * 4.5 /*set_row*/, 114 * 4.5, 5 * 4.5, 2 * 4.5 },
        /* 22*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, "1234567", "[17]010615[10]A123456\"", 50, 10, 55, 142, 116, 104 /*set_row*/, 115, 11, 2 }, // With no scaling
        /* 23*/ { BARCODE_UPCE_CC, -1, -1, -1, 2.0, "1234567", "[17]010615[10]A123456\"", 50, 10, 55, 142 * 2, 116 * 2, 104 * 2 + 1 /*set_row*/, 115 * 2, 11 * 2, 2 * 2 }, // +1 set_row
    };
    int data_size = ARRAY_SIZE(data);

    char *text;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }
        if (data[i].scale) {
            symbol->scale = data[i].scale;
        }
        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int length = strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Print(%d) ret %d != 0\n", i, data[i].symbology, ret);

        assert_nonzero(symbol->bitmap_height >= data[i].expected_set_rows, "i:%d (%d) symbol->bitmap_height %d < expected_set_rows %d\n",
                i, data[i].symbology, symbol->bitmap_height, data[i].expected_set_rows);
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row, "i:%d (%d) expected_set_rows %d < expected_set_row %d\n",
                i, data[i].symbology, data[i].expected_set_rows, data[i].expected_set_row);
        for (int row = data[i].expected_set_row; row < data[i].expected_set_rows; row++) {
            int bits_set = 0;
            for (int column = data[i].expected_set_col; column < data[i].expected_set_col + data[i].expected_set_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    bits_set++;
                }
            }
            assert_equal(bits_set, data[i].expected_set_len, "i:%d (%d) row %d bits_set %d != expected_set_len %d\n", i, data[i].symbology, row, bits_set, data[i].expected_set_len);
        }
        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_options", test_options, 1, 0, 1 },
        { "test_buffer", test_buffer, 1, 1, 1 },
        { "test_upcean_hrt", test_upcean_hrt, 1, 0, 1 },
        { "test_row_separator", test_row_separator, 1, 0, 1 },
        { "test_stacking", test_stacking, 1, 0, 1 },
        { "test_output_options", test_output_options, 1, 0, 1 },
        { "test_draw_string_wrap", test_draw_string_wrap, 1, 0, 1 },
        { "test_code128_utf8", test_code128_utf8, 1, 0, 1 },
        { "test_scale", test_scale, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
