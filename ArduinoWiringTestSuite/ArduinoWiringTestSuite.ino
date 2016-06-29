
// Arduino Wiring Test Suite
// 
// Prerequisites: Windows IoT Core Project Templates need be installed from here:
// https://visualstudiogallery.msdn.microsoft.com/55b357e1-a533-43ad-82a5-a88ac4b01dec
// The extension is required to enable code highlighting compilation of .ino files in VS
//
// Run this test suite by deploying it to a device, similar to any other Arduino Wiring sketch.
// The test must be run under the debugger to get test results.
// Unlike other Arduino Wiring sketches however, this one does not depend on the Lightning 
// SDK Nuget package. Instead, it depends directly on the Lightning project of this solution.
// All tests are expected to Succeed.

#include "spi.h"

unsigned int test_count = 0;
unsigned int success_count = 0;

void PostTestResult(bool succeeded, const wchar_t* function)
{
    Log(L"**** %s **** - %s\n", (succeeded ? L"SUCCEEDED" : L"FAILED"), function);
}

void Test_memchr_P(void) {
    ::test_count++;
    bool success = false;

    char * result;
    char str[] = "Test String";
    result = (char*)memchr_P(str, 'r', strlen(str));
    if (result != NULL && (result - str + 1) == 8)
        success = true;
    ::success_count += (success ? 1 : 0);
    PostTestResult(success, __FUNCTIONW__);

    ::test_count++;
    result = (char*)memchr_P(str, 'x', strlen(str));
    success = result == NULL;

    ::success_count += (success ? 1 : 0);
    PostTestResult(success, __FUNCTIONW__);
}

void Test_memmem_P(void) {
    ::test_count++;
    bool success = false;

    char * result;
    char str[] = "Test String";
    char str2[] = "String";
    char str3[] = "Invalid";
    result = (char*)memmem_P(str, strlen(str), str2, strlen(str2));
    if (result != NULL && strcmp(result, str2) == 0)
        success = true;

    ::success_count += (success ? 1 : 0);
    PostTestResult(success, __FUNCTIONW__);

    ::test_count++;
    result = (char*)memmem_P(str, strlen(str), str3, strlen(str3));
    success = result == NULL;

    ::success_count += (success ? 1 : 0);
    PostTestResult(success, __FUNCTIONW__);
}

void Test_strchrnul_P(void) {
    ::test_count++;
    bool success = false;

    // The strchrnul() function is like strchr() except that if ch is not
    // found in str, then it returns a pointer to the null byte at the end of
    // str, rather than NULL.

    const char * result;
    char str[] = "Test String";
    result = strchrnul_P(str, 'r');
    if (result != NULL && (result - str + 1) == 8)
        success = true;

    ::success_count += (success ? 1 : 0);
    PostTestResult(success, __FUNCTIONW__);

    ::test_count++;
    result = strchrnul_P(str, 'L');
    if (result[0] == '\x0' && result == str + strlen(str))
        success = true;
    else
        success = false;

    ::success_count += (success ? 1 : 0);
    PostTestResult(success, __FUNCTIONW__);
}

void Test_strcasestr_P(void) {
    ::test_count++;
    bool success = false;

    char * result;
    char str[] = "test String";
    char str2[] = "sTrInG";
    result = (char*)strcasestr_P(str, str2);
    if (result != NULL && strcmp(result, "String") == 0)
        success = true;

    ::success_count += (success ? 1 : 0);
    PostTestResult(success, __FUNCTIONW__);

    ::test_count++;
    result = (char*)strcasestr_P(str, "Not here");
    success = result == NULL;

    ::success_count += (success ? 1 : 0);
    PostTestResult(success, __FUNCTIONW__);
}

void setup(void) {

    Test_memchr_P();
    Test_memmem_P();
    Test_strchrnul_P();
    Test_strcasestr_P();

    Log(L"\n%u/%u TEST PASSED\n", ::success_count, ::test_count);
}

void loop(void) {
    // exit() works correctly in DEBUG
    exit(0);
}
