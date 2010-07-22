#include <stdlib.h>
#include <check.h>

#include "../agi.h"
#include "../ui/string.h"

#define TMPBUF_SIZE (0xC)

// test string to int
START_TEST (test_str_to_u16)
{
    fail_unless( str_to_u16("-16") == 0 ); // doesn't handle -

    fail_unless( str_to_u16("") == 0 );
    fail_unless( str_to_u16("0") == 0 );
    fail_unless( str_to_u16("1") == 1 );
    fail_unless( str_to_u16("2") == 2 );
    fail_unless( str_to_u16("3") == 3 );
    fail_unless( str_to_u16("4") == 4 );
    fail_unless( str_to_u16("5") == 5 );
    fail_unless( str_to_u16("5") == 5 );
    fail_unless( str_to_u16("10") == 10 );
    fail_unless( str_to_u16("   10") == 10 );
    fail_unless( str_to_u16("   10aahungrycataaaa") == 10 );  // ignores characters
    fail_unless( str_to_u16("2") == 2 );
    fail_unless( str_to_u16("4") == 4 );
    fail_unless( str_to_u16("8") == 8 );
    fail_unless( str_to_u16("16") == 16 );
    fail_unless( str_to_u16("32") == 32 );
    fail_unless( str_to_u16("64") == 64 );
    fail_unless( str_to_u16("128") == 128 );
    fail_unless( str_to_u16("256") == 256 );
    fail_unless( str_to_u16("512") == 512 );
    fail_unless( str_to_u16("1024") == 1024 );
    fail_unless( str_to_u16("2048") == 2048 );
    fail_unless( str_to_u16("4096") == 4096 );
    fail_unless( str_to_u16("8192") == 8192 );
    fail_unless( str_to_u16("16384") == 16384 );
    fail_unless( str_to_u16("32768") == 32768 );

    fail_unless( str_to_u16("65535") == 65535 );  // edge case
}
END_TEST


// check int to string
START_TEST (test_int_to_string)
{
    char tmpbuf[TMPBUF_SIZE];
    ck_assert_str_eq(  u16_to_str(tmpbuf, TMPBUF_SIZE, 0), "0" );
    ck_assert_str_eq(  u16_to_str(tmpbuf, TMPBUF_SIZE, 1), "1" );
    ck_assert_str_eq(  u16_to_str(tmpbuf, TMPBUF_SIZE, 123), "123" );
    ck_assert_str_eq(  u16_to_str(tmpbuf, TMPBUF_SIZE, 65535), "65535" );
}
END_TEST


// check string zero pad
START_TEST (test_string_zero_pad)
{
    char tmpbuf[TMPBUF_SIZE];
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 0),  "1" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 1),  "1" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 2),  "01" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 3),  "001" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 4),  "0001" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 5),  "00001" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 6),  "000001" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 7),  "0000001" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 8),  "00000001" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 9),  "000000001" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 10), "0000000001" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 11), "00000000001" );
    //ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "1", 12), "000000000001" );

    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "983462", 4), "983462" ); // don't add zeros if number > pad size

    // empty strings
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "", 0),  "" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "", 1),  "0" );
    ck_assert_str_eq(  string_zero_pad(tmpbuf, TMPBUF_SIZE, "", 2),  "00" );
}
END_TEST


// check int to hex string
START_TEST (test_int_to_hex_string)
{
    char tmpbuf[TMPBUF_SIZE];
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 0), "0" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 1), "1" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 2), "2" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 3), "3" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 4), "4" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 5), "5" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 6), "6" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 7), "7" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 8), "8" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 9), "9" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 10), "A" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 11), "B" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 12), "C" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 13), "D" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 14), "E" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 15), "F" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 16), "10" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 17), "11" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 18), "12" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 19), "13" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 20), "14" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 21), "15" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 22), "16" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 23), "17" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 24), "18" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 25), "19" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 26), "1A" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 27), "1B" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 28), "1C" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 29), "1D" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 30), "1E" );
    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 31), "1F" );

    ck_assert_str_eq( u16_to_hex(tmpbuf, TMPBUF_SIZE, 7345), "1CB1" );
    ck_assert_str_ne( u16_to_hex(tmpbuf, TMPBUF_SIZE, 7345), "1cb1" );  // bad caps
}
END_TEST


// check string reverse
START_TEST (test_string_reverse)
{
    char test_buffer[128];
    char *result;

    strcpy(test_buffer, "catinthehat");
    result = string_reverse(test_buffer);
    ck_assert_str_eq(result, "tahehtnitac");

    strcpy(test_buffer, "");
    result = string_reverse(test_buffer);
    ck_assert_str_eq( result, "");

    strcpy(test_buffer, "a");
    result = string_reverse(test_buffer);
    ck_assert_str_eq( result, "a");

    strcpy(test_buffer, "ab");
    result = string_reverse(test_buffer);
    ck_assert_str_eq( result, "ba");

    strcpy(test_buffer, "abc");
    result = string_reverse(test_buffer);
    ck_assert_str_eq( result, "cba");
}
END_TEST


// check string lower
START_TEST (test_string_lower)
{
    char test_buffer[128];
    char *result;

    strcpy(test_buffer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    result = string_lower(test_buffer);
    ck_assert_str_eq(result, "abcdefghijklmnopqrstuvwxyz");

    strcpy(test_buffer, "abc");
    result = string_lower(test_buffer);
    ck_assert_str_eq(result, "abc");

    strcpy(test_buffer, "");
    result = string_lower(test_buffer);
    ck_assert_str_eq(result, "");
}
END_TEST


// check strtok_r
// ??


Suite *string_suite (void)
{
  Suite *s = suite_create ("String");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_str_to_u16);
  tcase_add_test (tc_core, test_int_to_string);
  tcase_add_test (tc_core, test_string_zero_pad);
  tcase_add_test (tc_core, test_int_to_hex_string);
  tcase_add_test (tc_core, test_string_reverse);
  tcase_add_test (tc_core, test_string_lower);
  suite_add_tcase (s, tc_core);

  return s;
}


int main(void)
{
    int number_failed;
    Suite *s = string_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

