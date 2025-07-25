// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/********************************************************************
 * COPYRIGHT:
 * Copyright (c) 1999-2016, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/************************************************************************
*   Date        Name        Description
*   12/15/99    Madhu        Creation.
*   01/12/2000  Madhu        Updated for changed API and added new tests
************************************************************************/

#include "unicode/utypes.h"
#if !UCONFIG_NO_BREAK_ITERATION

#include <algorithm>
#include <array>
#include <cinttypes>
#include <list>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <vector>
#include <string_view>

#include "unicode/brkiter.h"
#include "unicode/localpointer.h"
#include "unicode/numfmt.h"
#include "unicode/rbbi.h"
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
#include "unicode/regex.h"
#endif
#include "unicode/schriter.h"
#include "unicode/uchar.h"
#include "unicode/utf16.h"
#include "unicode/ucnv.h"
#include "unicode/uniset.h"
#include "unicode/uscript.h"
#include "unicode/ustring.h"
#include "unicode/utext.h"
#include "unicode/utrace.h"

#include "charstr.h"
#include "cmemory.h"
#include "cstr.h"
#include "cstring.h"
#include "intltest.h"
#include "lstmbe.h"
#include "rbbitst.h"
#include "rbbidata.h"
#include "utypeinfo.h"  // for 'typeid' to work
#include "uvector.h"
#include "uvectr32.h"


#if !UCONFIG_NO_FILTERED_BREAK_ITERATION
#include "unicode/filteredbrk.h"
#endif // !UCONFIG_NO_FILTERED_BREAK_ITERATION

#define TEST_ASSERT(x) UPRV_BLOCK_MACRO_BEGIN { \
    if (!(x)) { \
        errln("Failure in file %s, line %d", __FILE__, __LINE__); \
    } \
} UPRV_BLOCK_MACRO_END

#define TEST_ASSERT_SUCCESS(errcode) UPRV_BLOCK_MACRO_BEGIN { \
    if (U_FAILURE(errcode)) { \
        errcheckln(errcode, "Failure in file %s, line %d, status = \"%s\"", __FILE__, __LINE__, u_errorName(errcode)); \
    } \
} UPRV_BLOCK_MACRO_END

#define MONKEY_ERROR(msg, fRuleFileName, index, engineState) { \
    IntlTest::gTest->errln("\n%s:%d %s at index %d. Parameters to reproduce: @\"type=%s engineState=[%s] loop=1\"", \
                    __FILE__, __LINE__, msg, index, fRuleFileName, engineState.c_str()); \
}

//---------------------------------------------
// runIndexedTest
//---------------------------------------------


//  Note:  Before adding new tests to this file, check whether the desired test data can
//         simply be added to the file testdata/rbbitest.txt.  In most cases it can,
//         it's much less work than writing a new test, diagnostic output in the event of failures
//         is good, and the test data file will is shared with ICU4J, so eventually the test
//         will run there as well, without additional effort.

void RBBITest::runIndexedTest( int32_t index, UBool exec, const char* &name, char* params )
{
    if (exec) logln("TestSuite RuleBasedBreakIterator: ");
    fTestParams = params;

    TESTCASE_AUTO_BEGIN;
#if !UCONFIG_NO_FILE_IO
    TESTCASE_AUTO(TestBug4153072);
#endif
#if !UCONFIG_NO_FILE_IO
    TESTCASE_AUTO(TestUnicodeFiles);
#endif
    TESTCASE_AUTO(TestGetAvailableLocales);
    TESTCASE_AUTO(TestGetDisplayName);
#if !UCONFIG_NO_FILE_IO
    TESTCASE_AUTO(TestEndBehaviour);
    TESTCASE_AUTO(TestPreceding_NegativeIndex);
    TESTCASE_AUTO(TestWordBreaks);
    TESTCASE_AUTO(TestWordBoundary);
    TESTCASE_AUTO(TestLineBreaks);
    TESTCASE_AUTO(TestSentBreaks);
    TESTCASE_AUTO(TestExtended);
#endif
#if !UCONFIG_NO_REGULAR_EXPRESSIONS && !UCONFIG_NO_FILE_IO
    TESTCASE_AUTO(TestMonkey);
#endif
#if !UCONFIG_NO_FILE_IO
    TESTCASE_AUTO(TestBug3818);
#endif
    TESTCASE_AUTO(TestDebug);
#if !UCONFIG_NO_FILE_IO
    TESTCASE_AUTO(TestBug5775);
#endif
    TESTCASE_AUTO(TestBug9983);
    TESTCASE_AUTO(TestDictRules);
    TESTCASE_AUTO(TestBug5532);
    TESTCASE_AUTO(TestBug7547);
    TESTCASE_AUTO(TestBug12797);
    TESTCASE_AUTO(TestBug12918);
    TESTCASE_AUTO(TestBug12932);
    TESTCASE_AUTO(TestEmoji);
    TESTCASE_AUTO(TestBug12519);
    TESTCASE_AUTO(TestBug12677);
    TESTCASE_AUTO(TestTableRedundancies);
    TESTCASE_AUTO(TestBug13447);
    TESTCASE_AUTO(TestReverse);
    TESTCASE_AUTO(TestBug13692);
    TESTCASE_AUTO(TestDebugRules);
    TESTCASE_AUTO(Test8BitsTrieWith8BitStateTable);
    TESTCASE_AUTO(Test8BitsTrieWith16BitStateTable);
    TESTCASE_AUTO(Test16BitsTrieWith8BitStateTable);
    TESTCASE_AUTO(Test16BitsTrieWith16BitStateTable);
    TESTCASE_AUTO(TestTable_8_16_Bits);
    TESTCASE_AUTO(TestBug13590);
    TESTCASE_AUTO(TestUnpairedSurrogate);
    TESTCASE_AUTO(TestLSTMThai);
    TESTCASE_AUTO(TestLSTMBurmese);
    TESTCASE_AUTO(TestRandomAccess);
    TESTCASE_AUTO(TestExternalBreakEngineWithFakeTaiLe);
    TESTCASE_AUTO(TestExternalBreakEngineWithFakeYue);
    TESTCASE_AUTO(TestBug22579);
    TESTCASE_AUTO(TestBug22581);
    TESTCASE_AUTO(TestBug22584);
    TESTCASE_AUTO(TestBug22585);
    TESTCASE_AUTO(TestBug22602);
    TESTCASE_AUTO(TestBug22636);

#if U_ENABLE_TRACING
    TESTCASE_AUTO(TestTraceCreateCharacter);
    TESTCASE_AUTO(TestTraceCreateWord);
    TESTCASE_AUTO(TestTraceCreateSentence);
    TESTCASE_AUTO(TestTraceCreateTitle);
    TESTCASE_AUTO(TestTraceCreateLine);
    TESTCASE_AUTO(TestTraceCreateLineNormal);
    TESTCASE_AUTO(TestTraceCreateLineLoose);
    TESTCASE_AUTO(TestTraceCreateLineStrict);
    TESTCASE_AUTO(TestTraceCreateLineNormalPhrase);
    TESTCASE_AUTO(TestTraceCreateLineLoosePhrase);
    TESTCASE_AUTO(TestTraceCreateLineStrictPhrase);
    TESTCASE_AUTO(TestTraceCreateLinePhrase);
    TESTCASE_AUTO(TestTraceCreateBreakEngine);
#endif

    TESTCASE_AUTO_END;
}


//--------------------------------------------------------------------------------------
//
//    RBBITest    constructor and destructor
//
//--------------------------------------------------------------------------------------

RBBITest::RBBITest() {
    fTestParams = nullptr;
}


RBBITest::~RBBITest() {
}


static void printStringBreaks(UText *tstr, int expected[], int expectedCount) {
    UErrorCode status = U_ZERO_ERROR;
    char name[100];
    printf("code    alpha extend alphanum type word sent line name\n");
    int nextExpectedIndex = 0;
    utext_setNativeIndex(tstr, 0);
    for (int j = 0; j < static_cast<int>(utext_nativeLength(tstr)); j=static_cast<int>(utext_getNativeIndex(tstr))) {
        if (nextExpectedIndex < expectedCount && j >= expected[nextExpectedIndex] ) {
            printf("------------------------------------------------ %d\n", j);
            ++nextExpectedIndex;
        }

        UChar32 c = utext_next32(tstr);
        u_charName(c, U_UNICODE_CHAR_NAME, name, 100, &status);
        printf("%7x %5d %6d %8d %4s %4s %4s %4s %s\n", static_cast<int>(c),
                           u_isUAlphabetic(c),
                           u_hasBinaryProperty(c, UCHAR_GRAPHEME_EXTEND),
                           u_isalnum(c),
                           u_getPropertyValueName(UCHAR_GENERAL_CATEGORY,
                                                  u_charType(c),
                                                  U_SHORT_PROPERTY_NAME),
                           u_getPropertyValueName(UCHAR_WORD_BREAK,
                                                  u_getIntPropertyValue(c,
                                                          UCHAR_WORD_BREAK),
                                                  U_SHORT_PROPERTY_NAME),
                           u_getPropertyValueName(UCHAR_SENTENCE_BREAK,
                                   u_getIntPropertyValue(c,
                                           UCHAR_SENTENCE_BREAK),
                                   U_SHORT_PROPERTY_NAME),
                           u_getPropertyValueName(UCHAR_LINE_BREAK,
                                   u_getIntPropertyValue(c,
                                           UCHAR_LINE_BREAK),
                                   U_SHORT_PROPERTY_NAME),
                           name);
    }
}


static void printStringBreaks(const UnicodeString &ustr, int expected[], int expectedCount) {
   UErrorCode status = U_ZERO_ERROR;
   UText *tstr = nullptr;
   tstr = utext_openConstUnicodeString(nullptr, &ustr, &status);
   if (U_FAILURE(status)) {
       printf("printStringBreaks, utext_openConstUnicodeString() returns %s\n", u_errorName(status));
       return;
    }
   printStringBreaks(tstr, expected, expectedCount);
   utext_close(tstr);
}


void RBBITest::TestBug3818() {
    UErrorCode  status = U_ZERO_ERROR;

    // Four Thai words...
    static const char16_t thaiWordData[] = {  0x0E43,0x0E2B,0x0E0D,0x0E48, 0x0E43,0x0E2B,0x0E0D,0x0E48,
                                           0x0E43,0x0E2B,0x0E0D,0x0E48, 0x0E43,0x0E2B,0x0E0D,0x0E48, 0 };
    UnicodeString  thaiStr(thaiWordData);

    BreakIterator* bi = BreakIterator::createWordInstance(Locale("th"), status);
    if (U_FAILURE(status) || bi == nullptr) {
        errcheckln(status, "Fail at file %s, line %d, status = %s", __FILE__, __LINE__, u_errorName(status));
        return;
    }
    bi->setText(thaiStr);

    int32_t  startOfSecondWord = bi->following(1);
    if (startOfSecondWord != 4) {
        errln("Fail at file %s, line %d expected start of word at 4, got %d",
            __FILE__, __LINE__, startOfSecondWord);
    }
    startOfSecondWord = bi->following(0);
    if (startOfSecondWord != 4) {
        errln("Fail at file %s, line %d expected start of word at 4, got %d",
            __FILE__, __LINE__, startOfSecondWord);
    }
    delete bi;
}


//---------------------------------------------
//
//     other tests
//
//---------------------------------------------

void RBBITest::TestGetAvailableLocales()
{
    int32_t locCount = 0;
    const Locale* locList = BreakIterator::getAvailableLocales(locCount);

    if (locCount == 0)
        dataerrln("getAvailableLocales() returned an empty list!");
    // Just make sure that it's returning good memory.
    int32_t i;
    for (i = 0; i < locCount; ++i) {
        logln(locList[i].getName());
    }
}

//Testing the BreakIterator::getDisplayName() function
void RBBITest::TestGetDisplayName()
{
    UnicodeString   result;

    BreakIterator::getDisplayName(Locale::getUS(), result);
    if (Locale::getDefault() == Locale::getUS() && result != "English (United States)")
        dataerrln("BreakIterator::getDisplayName() failed: expected \"English (United States)\", got \""
                + result);

    BreakIterator::getDisplayName(Locale::getFrance(), Locale::getUS(), result);
    if (result != "French (France)")
        dataerrln("BreakIterator::getDisplayName() failed: expected \"French (France)\", got \""
                + result);
}
/**
 * Test End Behaviour
 * @bug 4068137
 */
void RBBITest::TestEndBehaviour()
{
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString testString("boo.");
    BreakIterator *wb = BreakIterator::createWordInstance(Locale::getDefault(), status);
    if (U_FAILURE(status))
    {
        errcheckln(status, "Failed to create the BreakIterator for default locale in TestEndBehaviour. - %s", u_errorName(status));
        return;
    }
    wb->setText(testString);

    if (wb->first() != 0)
        errln("Didn't get break at beginning of string.");
    if (wb->next() != 3)
        errln("Didn't get break before period in \"boo.\"");
    if (wb->current() != 4 && wb->next() != 4)
        errln("Didn't get break at end of string.");
    delete wb;
}
/*
 * @bug 4153072
 */
void RBBITest::TestBug4153072() {
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator *iter = BreakIterator::createWordInstance(Locale::getDefault(), status);
    if (U_FAILURE(status))
    {
        errcheckln(status, "Failed to create the BreakIterator for default locale in TestBug4153072 - %s", u_errorName(status));
        return;
    }
    UnicodeString str("...Hello, World!...");
    int32_t begin = 3;
    int32_t end = str.length() - 3;
    UBool onBoundary;

    StringCharacterIterator* textIterator = new StringCharacterIterator(str, begin, end, begin);
    iter->adoptText(textIterator);
    int index;
    // Note: with the switch to UText, there is no way to restrict the
    //       iteration range to begin at an index other than zero.
    //       String character iterators created with a non-zero bound are
    //         treated by RBBI as being empty.
    for (index = -1; index < begin + 1; ++index) {
        onBoundary = iter->isBoundary(index);
        if (index == 0?  !onBoundary : onBoundary) {
            errln(UnicodeString("Didn't handle isBoundary correctly with offset = ") + index +
                            " and begin index = " + begin);
        }
    }
    delete iter;
}


//
// Test for problem reported by Ashok Matoria on 9 July 2007
//    One.<kSoftHyphen><kSpace>Two.
//
//    Sentence break at start (0) and then on calling next() it breaks at
//   'T' of "Two". Now, at this point if I do next() and
//    then previous(), it breaks at <kSOftHyphen> instead of 'T' of "Two".
//
void RBBITest::TestBug5775() {
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator *bi = BreakIterator::createSentenceInstance(Locale::getEnglish(), status);
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }
// Check for status first for better handling of no data errors.
    TEST_ASSERT(bi != nullptr);
    if (bi == nullptr) {
        return;
    }

    UnicodeString s("One.\\u00ad Two.", -1, US_INV);
    //               01234      56789
    s = s.unescape();
    bi->setText(s);
    int pos = bi->next();
    TEST_ASSERT(pos == 6);
    pos = bi->next();
    TEST_ASSERT(pos == 10);
    pos = bi->previous();
    TEST_ASSERT(pos == 6);
    delete bi;
}



//------------------------------------------------------------------------------
//
//   RBBITest::Extended    Run  RBBI Tests from an external test data file
//
//------------------------------------------------------------------------------

struct TestParams {
    BreakIterator   *bi;                   // Break iterator is set while parsing test source.
                                           //   Changed out whenever test data changes break type.

    UnicodeString    dataToBreak;          // Data that is built up while parsing the test.
    UVector32       *expectedBreaks;       // Expected break positions, matches dataToBreak UnicodeString.
    UVector32       *srcLine;              // Positions in source file, indexed same as dataToBreak.
    UVector32       *srcCol;

    UText           *textToBreak;          // UText, could be UTF8 or UTF16.
    UVector32       *textMap;              // Map from UTF-16 dataToBreak offsets to UText offsets.
    CharString       utf8String;           // UTF-8 form of text to break.

    TestParams(UErrorCode &status) : dataToBreak() {
        bi               = nullptr;
        expectedBreaks   = new UVector32(status);
        srcLine          = new UVector32(status);
        srcCol           = new UVector32(status);
        textToBreak      = nullptr;
        textMap          = new UVector32(status);
    }

    ~TestParams() {
        delete bi;
        delete expectedBreaks;
        delete srcLine;
        delete srcCol;
        utext_close(textToBreak);
        delete textMap;
    }

    int32_t getSrcLine(int32_t bp);
    int32_t getExpectedBreak(int32_t bp);
    int32_t getSrcCol(int32_t bp);

    void setUTF16(UErrorCode &status);
    void setUTF8(UErrorCode &status);
};

// Append a UnicodeString to a CharString with UTF-8 encoding.
// Substitute any invalid chars.
//   Note: this is used with test data that includes a few unpaired surrogates in the UTF-16 that will be substituted.
static void CharStringAppend(CharString &dest, const UnicodeString &src, UErrorCode &status) {
    if (U_FAILURE(status)) {
        return;
    }
    int32_t utf8Length;
    u_strToUTF8WithSub(nullptr, 0, &utf8Length,         // Output Buffer, nullptr for preflight.
                       src.getBuffer(), src.length(),   // UTF-16 data
                       0xfffd, nullptr,                 // Substitution char, number of subs.
                       &status);
    if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
        return;
    }
    status = U_ZERO_ERROR;
    int32_t capacity;
    char *buffer = dest.getAppendBuffer(utf8Length, utf8Length, capacity, status);
    u_strToUTF8WithSub(buffer, utf8Length, nullptr,
                       src.getBuffer(), src.length(),
                       0xfffd, nullptr, &status);
    dest.append(buffer, utf8Length, status);
}


void TestParams::setUTF16(UErrorCode &status) {
    textToBreak = utext_openUnicodeString(textToBreak, &dataToBreak, &status);
    textMap->removeAllElements();
    for (int32_t i=0; i<dataToBreak.length(); i++) {
        if (i == dataToBreak.getChar32Start(i)) {
            textMap->addElement(i, status);
        } else {
            textMap->addElement(-1, status);
        }
    }
    textMap->addElement(dataToBreak.length(), status);
    U_ASSERT(dataToBreak.length() + 1 == textMap->size());
}


void TestParams::setUTF8(UErrorCode &status) {
    if (U_FAILURE(status)) {
        return;
    }
    utf8String.clear();
    CharStringAppend(utf8String, dataToBreak, status);
    textToBreak = utext_openUTF8(textToBreak, utf8String.data(), utf8String.length(), &status);
    if (U_FAILURE(status)) {
        return;
    }

    textMap->removeAllElements();
    int32_t utf16Index = 0;
    for (;;) {
        textMap->addElement(utf16Index, status);
        UChar32 c32 = utext_current32(textToBreak);
        if (c32 < 0) {
            break;
        }
        utf16Index += U16_LENGTH(c32);
        utext_next32(textToBreak);
        while (textMap->size() < utext_getNativeIndex(textToBreak)) {
            textMap->addElement(-1, status);
        }
    }
    U_ASSERT(utext_nativeLength(textToBreak) + 1 == textMap->size());
}


int32_t TestParams::getSrcLine(int32_t bp) {
    if (bp >= textMap->size()) {
        bp = textMap->size() - 1;
    }
    int32_t i = 0;
    for(; bp >= 0 ; --bp) {
        // Move to a character boundary if we are not on one already.
        i = textMap->elementAti(bp);
        if (i >= 0) {
            break;
        }
    }
    return srcLine->elementAti(i);
}


int32_t TestParams::getExpectedBreak(int32_t bp) {
    if (bp >= textMap->size()) {
        return 0;
    }
    int32_t i = textMap->elementAti(bp);
    int32_t retVal = 0;
    if (i >= 0) {
        retVal = expectedBreaks->elementAti(i);
    }
    return retVal;
}


int32_t TestParams::getSrcCol(int32_t bp) {
    if (bp >= textMap->size()) {
        bp = textMap->size() - 1;
    }
    int32_t i = 0;
    for(; bp >= 0; --bp) {
        // Move bp to a character boundary if we are not on one already.
        i = textMap->elementAti(bp);
        if (i >= 0) {
            break;
        }
    }
    return srcCol->elementAti(i);
}


void RBBITest::executeTest(TestParams *t, UErrorCode &status) {
    int32_t    bp;
    int32_t    prevBP;
    int32_t    i;

    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }

    if (t->bi == nullptr) {
        return;
    }

    t->bi->setText(t->textToBreak, status);
    //
    //  Run the iterator forward
    //
    prevBP = -1;
    for (bp = t->bi->first(); bp != BreakIterator::DONE; bp = t->bi->next()) {
        if (prevBP ==  bp) {
            // Fail for lack of forward progress.
            errln("Forward Iteration, no forward progress.  Break Pos=%4d  File line,col=%4d,%4d",
                bp, t->getSrcLine(bp), t->getSrcCol(bp));
            break;
        }

        // Check that there we didn't miss an expected break between the last one
        //  and this one.
        for (i=prevBP+1; i<bp; i++) {
            if (t->getExpectedBreak(i) != 0) {
                int expected[] = {0, i};
                printStringBreaks(t->dataToBreak, expected, 2);
                errln("Forward Iteration, break expected, but not found.  Pos=%4d  File line,col= %4d,%4d",
                      i, t->getSrcLine(i), t->getSrcCol(i));
            }
        }

        // Check that the break we did find was expected
        if (t->getExpectedBreak(bp) == 0) {
            int expected[] = {0, bp};
            printStringBreaks(t->textToBreak, expected, 2);
            errln("Forward Iteration, break found, but not expected.  Pos=%4d  File line,col= %4d,%4d",
                bp, t->getSrcLine(bp), t->getSrcCol(bp));
        } else {
            // The break was expected.
            //   Check that the {nnn} tag value is correct.
            int32_t expectedTagVal = t->getExpectedBreak(bp);
            if (expectedTagVal == -1) {
                expectedTagVal = 0;
            }
            int32_t line = t->getSrcLine(bp);
            int32_t rs = t->bi->getRuleStatus();
            if (rs != expectedTagVal) {
                errln("Incorrect status for forward break.  Pos=%4d  File line,col= %4d,%4d.\n"
                      "          Actual, Expected status = %4d, %4d",
                    bp, line, t->getSrcCol(bp), rs, expectedTagVal);
            }
        }

        prevBP = bp;
    }

    // Verify that there were no missed expected breaks after the last one found
    for (i=prevBP+1; i<utext_nativeLength(t->textToBreak); i++) {
        if (t->getExpectedBreak(i) != 0) {
            errln("Forward Iteration, break expected, but not found.  Pos=%4d  File line,col= %4d,%4d",
                      i, t->getSrcLine(i), t->getSrcCol(i));
        }
    }

    //
    //  Run the iterator backwards, verify that the same breaks are found.
    //
    prevBP = static_cast<int32_t>(utext_nativeLength(t->textToBreak) + 2); // start with a phony value for the last break pos seen.
    bp = t->bi->last();
    while (bp != BreakIterator::DONE) {
        if (prevBP ==  bp) {
            // Fail for lack of progress.
            errln("Reverse Iteration, no progress.  Break Pos=%4d  File line,col=%4d,%4d",
                bp, t->getSrcLine(bp), t->getSrcCol(bp));
            break;
        }

        // Check that we didn't miss an expected break between the last one
        //  and this one.  (UVector returns zeros for index out of bounds.)
        for (i=prevBP-1; i>bp; i--) {
            if (t->getExpectedBreak(i) != 0) {
                errln("Reverse Iteration, break expected, but not found.  Pos=%4d  File line,col= %4d,%4d",
                      i, t->getSrcLine(i), t->getSrcCol(i));
            }
        }

        // Check that the break we did find was expected
        if (t->getExpectedBreak(bp) == 0) {
            errln("Reverse Itertion, break found, but not expected.  Pos=%4d  File line,col= %4d,%4d",
                   bp, t->getSrcLine(bp), t->getSrcCol(bp));
        } else {
            // The break was expected.
            //   Check that the {nnn} tag value is correct.
            int32_t expectedTagVal = t->getExpectedBreak(bp);
            if (expectedTagVal == -1) {
                expectedTagVal = 0;
            }
            int line = t->getSrcLine(bp);
            int32_t rs = t->bi->getRuleStatus();
            if (rs != expectedTagVal) {
                errln("Incorrect status for reverse break.  Pos=%4d  File line,col= %4d,%4d.\n"
                      "          Actual, Expected status = %4d, %4d",
                    bp, line, t->getSrcCol(bp), rs, expectedTagVal);
            }
        }

        prevBP = bp;
        bp = t->bi->previous();
    }

    // Verify that there were no missed breaks prior to the last one found
    for (i=prevBP-1; i>=0; i--) {
        if (t->getExpectedBreak(i) != 0) {
            errln("Forward Itertion, break expected, but not found.  Pos=%4d  File line,col= %4d,%4d",
                      i, t->getSrcLine(i), t->getSrcCol(i));
        }
    }

    // Check isBoundary()
    for (i=0; i < utext_nativeLength(t->textToBreak); i++) {
        UBool boundaryExpected = (t->getExpectedBreak(i) != 0);
        UBool boundaryFound    = t->bi->isBoundary(i);
        if (boundaryExpected != boundaryFound) {
            errln("isBoundary(%d) incorrect. File line,col= %4d,%4d\n"
                  "        Expected, Actual= %s, %s",
                  i, t->getSrcLine(i), t->getSrcCol(i),
                  boundaryExpected ? "true":"false", boundaryFound? "true" : "false");
        }
    }

    // Check following()
    for (i=0; i < static_cast<int32_t>(utext_nativeLength(t->textToBreak)); i++) {
        int32_t actualBreak = t->bi->following(i);
        int32_t expectedBreak = BreakIterator::DONE;
        for (int32_t j=i+1; j <= static_cast<int32_t>(utext_nativeLength(t->textToBreak)); j++) {
            if (t->getExpectedBreak(j) != 0) {
                expectedBreak = j;
                break;
            }
        }
        if (expectedBreak != actualBreak) {
            errln("following(%d) incorrect. File line,col= %4d,%4d\n"
                  "        Expected, Actual= %d, %d",
                  i, t->getSrcLine(i), t->getSrcCol(i), expectedBreak, actualBreak);
        }
    }

    // Check preceding()
    for (i=static_cast<int32_t>(utext_nativeLength(t->textToBreak)); i>=0; i--) {
        int32_t actualBreak = t->bi->preceding(i);
        int32_t expectedBreak = BreakIterator::DONE;

        // For UTF-8 & UTF-16 supplementals, all code units of a character are equivalent.
        // preceding(trailing byte) will return the index of some preceding code point,
        // not the lead byte of the current code point, even though that has a smaller index.
        // Therefore, start looking at the expected break data not at i-1, but at
        // the start of code point index - 1.
        utext_setNativeIndex(t->textToBreak, i);
        int32_t j = static_cast<int32_t>(utext_getNativeIndex(t->textToBreak) - 1);
        for (; j >= 0; j--) {
            if (t->getExpectedBreak(j) != 0) {
                expectedBreak = j;
                break;
            }
        }
        if (expectedBreak != actualBreak) {
            errln("preceding(%d) incorrect. File line,col= %4d,%4d\n"
                  "        Expected, Actual= %d, %d",
                  i, t->getSrcLine(i), t->getSrcCol(i), expectedBreak, actualBreak);
        }
    }
}

void RBBITest::TestPreceding_NegativeIndex() {
    UErrorCode status = U_ZERO_ERROR;
    Locale loc = Locale::getRoot();
    LocalPointer<BreakIterator> bi(BreakIterator::createWordInstance(loc, status));
    UnicodeString source(u"The quick brown fox jumped over the lazy dog.");
    bi->setText(source);

    assertEquals("length of source string", 45, source.length());

    struct TestCase {
        const UnicodeString description;
        const int32_t startIdx;
        const int32_t expected;
    } cases[] = {
        {"negative index",                 -2, BreakIterator::DONE},
        {"zero",                            0, BreakIterator::DONE},
        {"one",                             1, 0},
        {"middle",                         41, 40},
        {"end",                            45, 44},
        {"after the end", source.length() + 2, source.length()}
    };

    for (const auto& cas : cases) {
        int32_t actual = bi->preceding(cas.startIdx);
        assertEquals(cas.description, cas.expected, actual);
    }
}

void RBBITest::TestExtended() {
     // The expectations in this test heavily depends on the Thai dictionary.
     // Therefore, we skip this test under the LSTM configuration.
     if (skipDictionaryTest()) {
         return;
     }
  // Skip test for now when UCONFIG_NO_FILTERED_BREAK_ITERATION is set. This
  // data driven test closely entangles filtered and regular data.
#if !UCONFIG_NO_REGULAR_EXPRESSIONS && !UCONFIG_NO_FILTERED_BREAK_ITERATION
    UErrorCode      status  = U_ZERO_ERROR;
    Locale          locale("");

    TestParams          tp(status);

    RegexMatcher      localeMatcher(UnicodeString(u"<locale *([\\p{L}\\p{Nd}_@&=-]*) *>"), 0, status);
    if (U_FAILURE(status)) {
        dataerrln("Failure in file %s, line %d, status = \"%s\"", __FILE__, __LINE__, u_errorName(status));
    }

    //
    //  Open and read the test data file.
    //
    const char *testDataDirectory = IntlTest::getSourceTestData(status);
    CharString testFileName(testDataDirectory, -1, status);
    testFileName.append("rbbitst.txt", -1, status);

    int    len;
    char16_t *testFile = ReadAndConvertFile(testFileName.data(), len, "UTF-8", status);
    if (U_FAILURE(status)) {
        errln("%s:%d Error %s opening file rbbitst.txt", __FILE__, __LINE__, u_errorName(status));
        return;
    }

    bool skipTest = false; // Skip this test?

    //
    //  Put the test data into a UnicodeString
    //
    UnicodeString testString(false, testFile, len);

    enum EParseState{
        PARSE_COMMENT,
        PARSE_TAG,
        PARSE_DATA,
        PARSE_NUM,
        PARSE_RULES
    }
    parseState = PARSE_TAG;

    EParseState savedState = PARSE_TAG;

    int32_t    lineNum  = 1;
    int32_t    colStart = 0;
    int32_t    column   = 0;
    int32_t    charIdx  = 0;

    int32_t    tagValue = 0;             // The numeric value of a <nnn> tag.

    UnicodeString       rules;           // Holds rules from a <rules> ... </rules> block
    int32_t             rulesFirstLine = 0;  // Line number of the start of current <rules> block

    for (charIdx = 0; charIdx < len; ) {
        status = U_ZERO_ERROR;
        char16_t  c = testString.charAt(charIdx);
        charIdx++;
        if (c == u'\r' && charIdx<len && testString.charAt(charIdx) == u'\n') {
            // treat CRLF as a unit
            c = u'\n';
            charIdx++;
        }
        if (c == u'\n' || c == u'\r') {
            lineNum++;
            colStart = charIdx;
        }
        column = charIdx - colStart + 1;

        switch (parseState) {
        case PARSE_COMMENT:
            if (c == u'\n' || c == u'\r') {
                parseState = savedState;
            }
            break;

        case PARSE_TAG:
            {
            if (c == u'#') {
                parseState = PARSE_COMMENT;
                savedState = PARSE_TAG;
                break;
            }
            if (u_isUWhiteSpace(c)) {
                break;
            }
            if (testString.compare(charIdx-1, 6, u"<word>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createWordInstance(locale,  status);
                skipTest = false;
                charIdx += 5;
                break;
            }
            if (testString.compare(charIdx-1, 6, u"<char>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createCharacterInstance(locale,  status);
                skipTest = false;
                charIdx += 5;
                break;
            }
            if (testString.compare(charIdx-1, 6, u"<line>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createLineInstance(locale,  status);
                skipTest = false;
#if UCONFIG_USE_ML_PHRASE_BREAKING
                if(uprv_strcmp(locale.getName(), "ja@lw=phrase") == 0) {
                    // skip <line> test cases of JP's phrase breaking when ML is enabled.
                    skipTest = true;
                }
#endif
                charIdx += 5;
                break;
            }
            if (testString.compare(charIdx-1, 8, u"<lineML>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createLineInstance(locale,  status);
                skipTest = false;
#if !UCONFIG_USE_ML_PHRASE_BREAKING
                if(uprv_strcmp(locale.getName(), "ja@lw=phrase") == 0) {
                    // skip <lineML> test cases of JP's phrase breaking when ML is disabled.
                    skipTest = true;
                }
#endif
                charIdx += 7;
                break;
            }
            if (testString.compare(charIdx-1, 6, u"<sent>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createSentenceInstance(locale,  status);
                skipTest = false;
                charIdx += 5;
                break;
            }
            if (testString.compare(charIdx-1, 7, u"<title>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createTitleInstance(locale,  status);
                charIdx += 6;
                break;
            }

            if (testString.compare(charIdx-1, 7, u"<rules>") == 0 ||
                testString.compare(charIdx-1, 10, u"<badrules>") == 0) {
                charIdx = testString.indexOf(u'>', charIdx) + 1;
                parseState = PARSE_RULES;
                rules.remove();
                rulesFirstLine = lineNum;
                break;
            }

            // <locale  loc_name>
            localeMatcher.reset(testString);
            if (localeMatcher.lookingAt(charIdx-1, status)) {
                UnicodeString localeName = localeMatcher.group(1, status);
                char localeName8[100];
                localeName.extract(0, localeName.length(), localeName8, sizeof(localeName8), nullptr);
                locale = Locale::createFromName(localeName8);
                charIdx += localeMatcher.group(0, status).length() - 1;
                TEST_ASSERT_SUCCESS(status);
                break;
            }
            if (testString.compare(charIdx-1, 6, u"<data>") == 0) {
                parseState = PARSE_DATA;
                charIdx += 5;
                tp.dataToBreak = "";
                tp.expectedBreaks->removeAllElements();
                tp.srcCol ->removeAllElements();
                tp.srcLine->removeAllElements();
                break;
            }

            errln("line %d: Tag expected in test file.", lineNum);
            parseState = PARSE_COMMENT;
            savedState = PARSE_DATA;
            goto end_test; // Stop the test.
            }
            break;

        case PARSE_RULES:
            if (testString.compare(charIdx-1, 8, u"</rules>") == 0) {
                charIdx += 7;
                parseState = PARSE_TAG;
                delete tp.bi;
                UParseError pe;
                tp.bi = new RuleBasedBreakIterator(rules, pe, status);
                skipTest = U_FAILURE(status);
                if (U_FAILURE(status)) {
                    errln("file rbbitst.txt: %d - Error %s creating break iterator from rules.",
                        rulesFirstLine + pe.line - 1, u_errorName(status));
                }
            } else if (testString.compare(charIdx-1, 11, u"</badrules>") == 0) {
                charIdx += 10;
                parseState = PARSE_TAG;
                UErrorCode ec = U_ZERO_ERROR;
                UParseError pe;
                RuleBasedBreakIterator bi(rules, pe, ec);
                if (U_SUCCESS(ec)) {
                    errln("file rbbitst.txt: %d - Expected, but did not get, a failure creating break iterator from rules.",
                        rulesFirstLine + pe.line - 1);
                }
            } else {
                rules.append(c);
            }
            break;

        case PARSE_DATA:
            if (c == u'•') {
                int32_t  breakIdx = tp.dataToBreak.length();
                if (tp.expectedBreaks->size() > breakIdx) {
                    errln("rbbitst.txt:%d:%d adjacent expected breaks with no intervening test text",
                          lineNum, column);
                }
                tp.expectedBreaks->setSize(breakIdx+1);
                tp.expectedBreaks->setElementAt(-1, breakIdx);
                tp.srcLine->setSize(breakIdx+1);
                tp.srcLine->setElementAt(lineNum, breakIdx);
                tp.srcCol ->setSize(breakIdx+1);
                tp.srcCol ->setElementAt(column, breakIdx);
                break;
            }

            if (testString.compare(charIdx-1, 7, u"</data>") == 0) {
                // Add final entry to mappings from break location to source file position.
                //  Need one extra because last break position returned is after the
                //    last char in the data, not at the last char.
                tp.srcLine->addElement(lineNum, status);
                tp.srcCol ->addElement(column, status);

                parseState = PARSE_TAG;
                charIdx += 6;

                if (!skipTest) {
                    // RUN THE TEST!
                    status = U_ZERO_ERROR;
                    tp.setUTF16(status);
                    executeTest(&tp, status);
                    TEST_ASSERT_SUCCESS(status);

                    // Run again, this time with UTF-8 text wrapped in a UText.
                    status = U_ZERO_ERROR;
                    tp.setUTF8(status);
                    TEST_ASSERT_SUCCESS(status);
                    executeTest(&tp, status);
                }
                break;
            }

            if (testString.compare(charIdx-1, 3, u"\\N{") == 0) {
                // Named character, e.g. \N{COMBINING GRAVE ACCENT}
                // Get the code point from the name and insert it into the test data.
                //   (Damn, no API takes names in Unicode  !!!
                //    we've got to take it back to char *)
                int32_t nameEndIdx = testString.indexOf(u'}', charIdx);
                int32_t nameLength = nameEndIdx - (charIdx+2);
                char charNameBuf[200];
                UChar32 theChar = -1;
                if (nameEndIdx != -1) {
                    UErrorCode status = U_ZERO_ERROR;
                    testString.extract(charIdx+2, nameLength, charNameBuf, sizeof(charNameBuf));
                    charNameBuf[sizeof(charNameBuf)-1] = 0;
                    theChar = u_charFromName(U_UNICODE_CHAR_NAME, charNameBuf, &status);
                    if (U_FAILURE(status)) {
                        theChar = -1;
                    }
                }
                if (theChar == -1) {
                    errln("Error in named character in test file at line %d, col %d",
                        lineNum, column);
                } else {
                    // Named code point was recognized.  Insert it
                    //   into the test data.
                    tp.dataToBreak.append(theChar);
                    while (tp.dataToBreak.length() > tp.srcLine->size()) {
                        tp.srcLine->addElement(lineNum, status);
                        tp.srcCol ->addElement(column, status);
                    }
                }
                if (nameEndIdx > charIdx) {
                    charIdx = nameEndIdx+1;

                }
                break;
            }



            if (testString.compare(charIdx-1, 2, u"<>") == 0) {
                charIdx++;
                int32_t  breakIdx = tp.dataToBreak.length();
                tp.expectedBreaks->setSize(breakIdx+1);
                tp.expectedBreaks->setElementAt(-1, breakIdx);
                tp.srcLine->setSize(breakIdx+1);
                tp.srcLine->setElementAt(lineNum, breakIdx);
                tp.srcCol ->setSize(breakIdx+1);
                tp.srcCol ->setElementAt(column, breakIdx);
                break;
            }

            if (c == u'<') {
                tagValue   = 0;
                parseState = PARSE_NUM;
                break;
            }

            if (c == u'#' && column==3) {   // TODO:  why is column off so far?
                parseState = PARSE_COMMENT;
                savedState = PARSE_DATA;
                break;
            }

            if (c == u'\\') {
                // Check for \ at end of line, a line continuation.
                //     Advance over (discard) the newline
                UChar32 cp = testString.char32At(charIdx);
                if (cp == u'\r' && charIdx<len && testString.charAt(charIdx+1) == u'\n') {
                    // We have a CR LF
                    //  Need an extra increment of the input ptr to move over both of them
                    charIdx++;
                }
                if (cp == u'\n' || cp == u'\r') {
                    lineNum++;
                    colStart = charIdx;
                    charIdx++;
                    break;
                }

                // Let unescape handle the back slash.
                cp = testString.unescapeAt(charIdx);
                if (cp != -1) {
                    // Escape sequence was recognized.  Insert the char
                    //   into the test data.
                    tp.dataToBreak.append(cp);
                    while (tp.dataToBreak.length() > tp.srcLine->size()) {
                        tp.srcLine->addElement(lineNum, status);
                        tp.srcCol ->addElement(column, status);
                    }
                    break;
                }


                // Not a recognized backslash escape sequence.
                // Take the next char as a literal.
                //  TODO:  Should this be an error?
                c = testString.charAt(charIdx);
                charIdx = testString.moveIndex32(charIdx, 1);
            }

            // Normal, non-escaped data char.
            tp.dataToBreak.append(c);

            // Save the mapping from offset in the data to line/column numbers in
            //   the original input file.  Will be used for better error messages only.
            //   If there's an expected break before this char, the slot in the mapping
            //     vector will already be set for this char; don't overwrite it.
            if (tp.dataToBreak.length() > tp.srcLine->size()) {
                tp.srcLine->addElement(lineNum, status);
                tp.srcCol ->addElement(column, status);
            }
            break;


        case PARSE_NUM:
            // We are parsing an expected numeric tag value, like <1234>,
            //   within a chunk of data.
            if (u_isUWhiteSpace(c)) {
                break;
            }

            if (c == u'>') {
                // Finished the number.  Add the info to the expected break data,
                //   and switch parse state back to doing plain data.
                parseState = PARSE_DATA;
                if (tagValue == 0) {
                    tagValue = -1;
                }
                int32_t  breakIdx = tp.dataToBreak.length();
                if (tp.expectedBreaks->size() > breakIdx) {
                    errln("rbbitst.txt:%d:%d adjacent expected breaks with no intervening test text",
                          lineNum, column);
                }
                tp.expectedBreaks->setSize(breakIdx+1);
                tp.expectedBreaks->setElementAt(tagValue, breakIdx);
                tp.srcLine->setSize(breakIdx+1);
                tp.srcLine->setElementAt(lineNum, breakIdx);
                tp.srcCol ->setSize(breakIdx+1);
                tp.srcCol ->setElementAt(column, breakIdx);
                break;
            }

            if (u_isdigit(c)) {
                tagValue = tagValue*10 + u_charDigitValue(c);
                break;
            }

            errln("Syntax Error in test file at line %d, col %d",
                lineNum, column);
            parseState = PARSE_COMMENT;
            goto end_test; // Stop the test
            break;
        }


        if (U_FAILURE(status)) {
            dataerrln("ICU Error %s while parsing test file at line %d.",
                u_errorName(status), lineNum);
            status = U_ZERO_ERROR;
            goto end_test; // Stop the test
        }

    }

    // Reached end of test file. Raise an error if parseState indicates that we are
    //   within a block that should have been terminated.

    if (parseState == PARSE_RULES) {
        errln("rbbitst.txt:%d <rules> block beginning at line %d is not closed.",
            lineNum, rulesFirstLine);
    }
    if (parseState == PARSE_DATA) {
        errln("rbbitst.txt:%d <data> block not closed.", lineNum);
    }


end_test:
    delete [] testFile;
#endif
}

//-------------------------------------------------------------------------------
//
//  TestDictRules   create a break iterator from source rules that includes a
//                  dictionary range.   Regression for bug #7130.  Source rules
//                  do not declare a break iterator type (word, line, sentence, etc.
//                  but the dictionary code, without a type, would loop.
//
//-------------------------------------------------------------------------------
void RBBITest::TestDictRules() {
    const char *rules =  "$dictionary = [a-z]; \n"
                         "!!forward; \n"
                         "$dictionary $dictionary; \n"
                         "!!reverse; \n"
                         "$dictionary $dictionary; \n";
    const char *text = "aa";
    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;

    RuleBasedBreakIterator bi(rules, parseError, status);
    if (U_SUCCESS(status)) {
        UnicodeString utext = text;
        bi.setText(utext);
        int32_t position;
        int32_t loops;
        for (loops = 0; loops<10; loops++) {
            position = bi.next();
            if (position == RuleBasedBreakIterator::DONE) {
                break;
            }
        }
        TEST_ASSERT(loops == 1);
    } else {
        dataerrln("Error creating RuleBasedBreakIterator: %s", u_errorName(status));
    }
}



//--------------------------------------------------------------------------------------------
//
//   Run tests from each of the boundary test data files distributed by the Unicode Consortium
//
//-------------------------------------------------------------------------------------------
void RBBITest::TestUnicodeFiles() {
    RuleBasedBreakIterator  *bi;
    UErrorCode               status = U_ZERO_ERROR;

    bi =  dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createCharacterInstance(Locale::getEnglish(), status));
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        runUnicodeTestData("GraphemeBreakTest.txt", bi);
    }
    delete bi;

    bi =  dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createWordInstance(Locale::getEnglish(), status));
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        runUnicodeTestData("WordBreakTest.txt", bi);
    }
    delete bi;

    bi =  dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createSentenceInstance(Locale::getEnglish(), status));
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        runUnicodeTestData("SentenceBreakTest.txt", bi);
    }
    delete bi;

    bi =  dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createLineInstance(Locale::getEnglish(), status));
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        runUnicodeTestData("LineBreakTest.txt", bi);
    }
    delete bi;
}


// Check for test cases from the Unicode test data files that are known to fail
// and should be skipped as known issues because ICU does not fully implement
// the Unicode specifications, or because ICU includes tailorings that differ from
// the Unicode standard.
//
// Test cases are identified by the test data sequence, which tends to be more stable
// across Unicode versions than the test file line numbers.
//
// The test case with ticket "10666" is a dummy, included as an example.

UBool RBBITest::testCaseIsKnownIssue(const UnicodeString &testCase, const char *fileName) {
    static struct TestCase {
        const char *fTicketNum;
        const char *fFileName;
        const char16_t *fString;
    } badTestCases[] = {
        {"10666", "GraphemeBreakTest.txt", u"\u0020\u0020\u0033"},    // Fake example, for illustration.
    };

    for (int n=0; n<UPRV_LENGTHOF(badTestCases); n++) {
        const TestCase &badCase = badTestCases[n];
        if (!strcmp(fileName, badCase.fFileName) &&
                testCase.startsWith(UnicodeString(badCase.fString))) {
            return logKnownIssue(badCase.fTicketNum);
        }
    }
    return false;
}


//--------------------------------------------------------------------------------------------
//
//   Run tests from one of the boundary test data files distributed by the Unicode Consortium
//
//-------------------------------------------------------------------------------------------
void RBBITest::runUnicodeTestData(const char *fileName, RuleBasedBreakIterator *bi) {
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
    UErrorCode  status = U_ZERO_ERROR;

    //
    //  Open and read the test data file, put it into a UnicodeString.
    //
    const char *testDataDirectory = IntlTest::getSourceTestData(status);
    char testFileName[1000];
    if (testDataDirectory == nullptr || strlen(testDataDirectory) >= sizeof(testFileName)) {
        dataerrln("Can't open test data.  Path too long.");
        return;
    }
    strcpy(testFileName, testDataDirectory);
    strcat(testFileName, fileName);

    logln("Opening data file %s\n", fileName);

    int    len;
    char16_t *testFile = ReadAndConvertFile(testFileName, len, "UTF-8", status);
    if (status != U_FILE_ACCESS_ERROR) {
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(testFile != nullptr);
    }
    if (U_FAILURE(status) || testFile == nullptr) {
        return; /* something went wrong, error already output */
    }
    UnicodeString testFileAsString(true, testFile, len);

    //
    //  Parse the test data file using a regular expression.
    //  Each kind of token is recognized in its own capture group; what type of item was scanned
    //     is identified by which group had a match.
    //
    //    Capture Group  #                  1          2            3            4           5
    //    Parses this item:               divide       x      hex digits   comment \n  unrecognized \n
    //
    UnicodeString tokenExpr("[ \t]*(?:(\\u00F7)|(\\u00D7)|([0-9a-fA-F]+)|((?:#.*?)?$.)|(.*?$.))", -1, US_INV);
    RegexMatcher    tokenMatcher(tokenExpr, testFileAsString, UREGEX_MULTILINE | UREGEX_DOTALL, status);
    UnicodeString   testString;
    UVector32       breakPositions(status);
    int             lineNumber = 1;
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }

    //
    //  Scan through each test case, building up the string to be broken in testString,
    //   and the positions that should be boundaries in the breakPositions vector.
    //
    int spin = 0;
    while (tokenMatcher.find()) {
        if(tokenMatcher.hitEnd()) {
          /* Shouldn't Happen(TM).  This means we didn't find the symbols we were looking for.
             This occurred when the text file was corrupt (wasn't marked as UTF-8)
             and caused an infinite loop here on EBCDIC systems!
          */
          fprintf(stderr,"FAIL: hit end of file %s for the %8dth time- corrupt data file?\r", fileName, ++spin);
          //       return;
        }
        if (tokenMatcher.start(1, status) >= 0) {
            // Scanned a divide sign, indicating a break position in the test data.
            if (testString.length()>0) {
                breakPositions.addElement(testString.length(), status);
            }
        }
        else if (tokenMatcher.start(2, status) >= 0) {
            // Scanned an 'x', meaning no break at this position in the test data
            //   Nothing to be done here.
            }
        else if (tokenMatcher.start(3, status) >= 0) {
            // Scanned Hex digits.  Convert them to binary, append to the character data string.
            const UnicodeString &hexNumber = tokenMatcher.group(3, status);
            int length = hexNumber.length();
            if (length<=8) {
                char buf[10];
                hexNumber.extract (0, length, buf, sizeof(buf), US_INV);
                UChar32 c = static_cast<UChar32>(strtol(buf, nullptr, 16));
                if (c<=0x10ffff) {
                    testString.append(c);
                } else {
                    errln("Error: Unicode Character value out of range. \'%s\', line %d.\n",
                       fileName, lineNumber);
                }
            } else {
                errln("Syntax Error: Hex Unicode Character value must have no more than 8 digits at \'%s\', line %d.\n",
                       fileName, lineNumber);
             }
        }
        else if (tokenMatcher.start(4, status) >= 0) {
            // Scanned to end of a line, possibly skipping over a comment in the process.
            //   If the line from the file contained test data, run the test now.
            if (testString.length() > 0 && !testCaseIsKnownIssue(testString, fileName)) {
                checkUnicodeTestCase(fileName, lineNumber, testString, &breakPositions, bi);
            }

            // Clear out this test case.
            //    The string and breakPositions vector will be refilled as the next
            //       test case is parsed.
            testString.remove();
            breakPositions.removeAllElements();
            lineNumber++;
        } else {
            // Scanner catchall.  Something unrecognized appeared on the line.
            char token[16];
            UnicodeString uToken = tokenMatcher.group(0, status);
            uToken.extract(0, uToken.length(), token, static_cast<uint32_t>(sizeof(token)));
            token[sizeof(token)-1] = 0;
            errln("Syntax error in test data file \'%s\', line %d.  Scanning \"%s\"\n", fileName, lineNumber, token);

            // Clean up, in preparation for continuing with the next line.
            testString.remove();
            breakPositions.removeAllElements();
            lineNumber++;
        }
        TEST_ASSERT_SUCCESS(status);
        if (U_FAILURE(status)) {
            break;
        }
    }

    delete [] testFile;
 #endif   // !UCONFIG_NO_REGULAR_EXPRESSIONS
}

//--------------------------------------------------------------------------------------------
//
//   checkUnicodeTestCase()   Run one test case from one of the Unicode Consortium
//                            test data files.  Do only a simple, forward-only check -
//                            this test is mostly to check that ICU and the Unicode
//                            data agree with each other.
//
//--------------------------------------------------------------------------------------------
void RBBITest::checkUnicodeTestCase(const char *testFileName, int lineNumber,
                         const UnicodeString &testString,   // Text data to be broken
                         UVector32 *breakPositions,         // Positions where breaks should be found.
                         RuleBasedBreakIterator *bi) {
    int32_t pos;                 // Break Position in the test string
    int32_t expectedI = 0;       // Index of expected break position in the vector of expected results.
    int32_t expectedPos;         // Expected break position (index into test string)

    bi->setText(testString);
    pos = bi->first();
    pos = bi->next();

    bool error = false;
    std::set<int32_t> actualBreaks;
    std::set<int32_t> expectedBreaks;
    while (pos != BreakIterator::DONE) {
        actualBreaks.insert(pos);
        if (expectedI >= breakPositions->size()) {
            errln("Test file \"%s\", line %d, unexpected break found at position %d",
                testFileName, lineNumber, pos);
            error = true;
            break;
        }
        expectedPos = breakPositions->elementAti(expectedI);
        expectedBreaks.insert(expectedPos);
        if (pos < expectedPos) {
            errln("Test file \"%s\", line %d, unexpected break found at position %d", testFileName,
                  lineNumber, pos);
            error = true;
            break;
        }
        if (pos > expectedPos) {
            errln("Test file \"%s\", line %d, failed to find expected break at position %d",
                  testFileName, lineNumber, expectedPos);
            error = true;
            break;
        }
        pos = bi->next();
        expectedI++;
    }

    if (pos==BreakIterator::DONE && expectedI<breakPositions->size()) {
        errln("Test file \"%s\", line %d, failed to find expected break at position %d", testFileName,
              lineNumber, breakPositions->elementAti(expectedI));
        error = true;
    }

    if (error) {
        for (; pos != BreakIterator::DONE; pos = bi->next()) {
            actualBreaks.insert(pos);
        }
        for (; expectedI < breakPositions->size(); ++expectedI) {
            expectedBreaks.insert(breakPositions->elementAti(expectedI));
        }
        UnicodeString expected;
        UnicodeString actual;
        for (int32_t i = 0; i < testString.length();) {
            const UChar32 c = testString.char32At(i);
            i += U16_LENGTH(c);
            expected += expectedBreaks.count(i) == 1 ? u"÷" : u"×";
            actual += actualBreaks.count(i) == 1 ? u"÷" : u"×";
            expected += c;
            actual += c;
        }
        expected += expectedBreaks.count(testString.length()) == 1 ? u"÷" : u"×";
        actual += actualBreaks.count(testString.length()) == 1 ? u"÷" : u"×";
        errln("Expected : " + expected);
        errln("Actual   : " + actual);
    }
}



#if !UCONFIG_NO_REGULAR_EXPRESSIONS

namespace {

class SegmentationRule {
  public:
    enum Resolution {
        BREAK = u'÷',
        NO_BREAK = u'×',
    };
    struct BreakContext {
        BreakContext(int32_t index) : indexInRemapped(index) {}
        std::optional<int32_t> indexInRemapped;
        const SegmentationRule *appliedRule = nullptr;
    };

    SegmentationRule(std::u16string_view name) { UnicodeString(name).toUTF8String(name_); }
    virtual ~SegmentationRule() = default;

    virtual void apply(UnicodeString &remapped, std::vector<BreakContext> &resolved) const = 0;
    virtual Resolution resolution() const = 0;
    const std::string &name() const { return name_; }

    std::chrono::steady_clock::duration timeSpent() const { return timeSpent_; }

  private:
    std::string name_;

  protected:
    mutable std::chrono::steady_clock::duration timeSpent_{};
};

class RemapRule : public SegmentationRule {
  public:
    RemapRule(const std::u16string_view name, const std::u16string_view pattern,
              const std::u16string_view replacement)
        : SegmentationRule(name), replacement_(replacement) {
        UParseError parseError;
        UErrorCode status = U_ZERO_ERROR;
        pattern_.reset(
            RegexPattern::compile(pattern, UREGEX_COMMENTS | UREGEX_DOTALL, parseError, status));
        U_ASSERT(U_SUCCESS(status));
    }

    virtual void apply(UnicodeString &remapped, std::vector<BreakContext> &resolved) const override {
        auto const start = std::chrono::steady_clock::now();
        UErrorCode status = U_ZERO_ERROR;
        UnicodeString result;
        int32_t i = 0;
        int32_t offset = 0;
        // We find all matches of the `pattern_` and replace them according to
        // the `replacement_`, producing the new remapped string `result`.
        // For every position i in the original string,
        // `resolved[i].indexInRemapped` is nullopt if i lies within a replaced
        // match, and is set to the new index in `result` otherwise, by adding
        // the accumulated difference `offset` between match lengths and
        // replacement lengths.
        // Consider a 4-codepoint, 6 code unit string s = ⟨ 𒀀, ◌́, ␠, ◌𝅲 ⟩, where
        // ␠ stands for U+0020 and U+12000 𒀀 and U+1D172 ◌𝅲 each require two code
        // units, and apply the following two rules:
        //   1. (?<X>\P{lb=SP}) \p{lb=CM}* → ${X}
        //   2. \p{lb=CM} → A
        // The string remapped and the indexInRemapped values change as follows:
        //   indexInRemapped                   remapped string  rule      final
        //   (aligned on the initial string)                    applied   offset
        //      𒀀     ◌́     ␠     ◌𝅲
        //   0  1  2     3     4  5  6         ⟨ 𒀀, ◌́, ␠, ◌𝅲 ⟩   (none)
        //   0  -  -     2     3  4  5         ⟨ 𒀀,    ␠, ◌𝅲 ⟩   1              -1
        //   0  -  -     2     3  -  4         ⟨ 𒀀,    ␠, A ⟩   2              -1
        //
        // Note that the last indexInRemapped is always equal to the length of
        // the remapped string.
        std::unique_ptr<RegexMatcher> matcher(pattern_->matcher(remapped, status));
        while (matcher->find()) {
            for (;; ++i) {
                if (!resolved[i].indexInRemapped.has_value()) {
                    continue;
                }
                if (*resolved[i].indexInRemapped > matcher->start(status)) {
                    break;
                }
                *resolved[i].indexInRemapped += offset;
            }
            for (;; ++i) {
                if (!resolved[i].indexInRemapped.has_value()) {
                    continue;
                }
                // Note that
                // `*resolved[i].indexInRemapped > matcher->end(status)` should
                // never happen with ordinary rules, but could in principle
                // happen with rules that remap to code point sequences, e.g.,
                //   1. BC → TYZ
                //   2. AT → X
                // applied to ⟨ A, B, C ⟩:
                //   indexInRemapped  remapped       rule
                //    A B C
                //   0 1 2 3          ⟨ A, B, C ⟩     (none)
                //   0 1 - 4          ⟨ A, T, Y, Z ⟩  1
                //   0 - - 3          ⟨ X, Y, Z ⟩     2
                // Where for the application of rule 2, the match ends at
                // position 2 in remapped, which does not correspond to a
                // position in the original string.
                if (*resolved[i].indexInRemapped >= matcher->end(status)) {
                    break;
                }
                if (resolved[i].appliedRule != nullptr &&
                        resolved[i].appliedRule->resolution() == BREAK) {
                    printf("Replacement rule at remapped indices %d sqq. spans a break",
                           matcher->start(status));
                    std::terminate();
                }
                resolved[i].appliedRule = this;
                resolved[i].indexInRemapped.reset();
            }
            // While replacing, we need to check that we are not creating
            // surrogate pairs.  Since appendReplacement performs two
            // concatenations (the unreplaced segment and the replacement), we
            // need to check in two places: whether the unreplaced segment
            // starts with a trailing surrogate that ends up after a leading
            // surrogate, and whether the replaced segment starts with a leading
            // surrogate that ends up after a trailing surrogate.
            // We break the pair by replacing one of the surrogates with U+FFFF,
            // which has the same properties for all but line breaking, and the
            // same behaviour in line breaking (lb=SG and lb=XX are both treated
            // as lb=AL).
            std::optional<int32_t> trailingLead;
            if (result.length() > 0 && U16_IS_LEAD(result[result.length() - 1])) {
                trailingLead = result.length() - 1;
            }

            matcher->appendReplacement(result, replacement_, status);

            if (trailingLead && *trailingLead + 1 < result.length() &&
                    U16_IS_TRAIL(result[*trailingLead + 1])) {
                result.setCharAt(*trailingLead, u'\uFFFF');
            }

            if (matcher->start(status) + offset > 0 &&
                    U16_IS_LEAD(result[matcher->start(status) + offset - 1]) &&
                    U16_IS_TRAIL(result[matcher->start(status) + offset])) {
                result.setCharAt(matcher->start(status) + offset, u'\uFFFF');
            }
            offset = result.length() - *resolved[i].indexInRemapped;
        }
        for (; i < static_cast<int32_t>(resolved.size()); ++i) {
            if (!resolved[i].indexInRemapped.has_value()) {
                continue;
            }
            *resolved[i].indexInRemapped += offset;
        }

        std::optional<int32_t> trailingLead;
        if (result.length() > 0 && U16_IS_LEAD(result[result.length() - 1])) {
            trailingLead = result.length() - 1;
        }
        matcher->appendTail(result);
        if (trailingLead && *trailingLead + 1 < result.length() &&
                U16_IS_TRAIL(result[*trailingLead + 1])) {
            result.setCharAt(*trailingLead, u'\uFFFF');
        }

        if (resolved.back().indexInRemapped != result.length()) {
            std::string indices;
            for (const auto r : resolved) {
                indices += r.indexInRemapped.has_value() ? std::to_string(*r.indexInRemapped) : "null";
                indices += ",";
            }
            std::string s;
            puts(("Inconsistent indexInRemapped " + indices + " for new remapped string " +
                  result.toUTF8String(s))
                     .c_str());
            std::terminate();
        }
        remapped = result;
        if (U_FAILURE(status)) {
            puts(("Failed to apply rule " + name()).c_str());
            std::terminate();
        }
        timeSpent_ += std::chrono::steady_clock::now() - start;
    }

    virtual Resolution resolution() const override { return NO_BREAK; }

  private:
    std::unique_ptr<RegexPattern> pattern_;
    UnicodeString replacement_;
};

class RegexRule : public SegmentationRule {
  public:
    template <typename T, typename = std::enable_if_t<std::is_constructible_v<T, Resolution>>>
    RegexRule(const std::u16string_view name, const std::u16string_view before, T resolution,
              const std::u16string_view after)
        : SegmentationRule(name), resolution_(static_cast<Resolution>(resolution)) {
        UParseError parseError;
        UErrorCode status = U_ZERO_ERROR;
        before_.reset(
            RegexPattern::compile(before, UREGEX_COMMENTS | UREGEX_DOTALL, parseError, status));
        endsWithBefore_.reset(RegexPattern::compile(
            ".*(" + before + ")", UREGEX_COMMENTS | UREGEX_DOTALL, parseError, status));
        after_.reset(RegexPattern::compile(after, UREGEX_COMMENTS | UREGEX_DOTALL, parseError, status));
        if (U_FAILURE(status)) {
            puts(("Failed to compile regular expressions for rule " + this->name()).c_str());
            std::terminate();
        }
    }

    virtual void apply(UnicodeString &remapped, std::vector<BreakContext> &resolved) const override {
        auto const start = std::chrono::steady_clock::now();
        UErrorCode status = U_ZERO_ERROR;
        // The unicodetools implementation simply tries, for each index, to
        // match the string up to the index against /.*(before)/ (with
        // `matches`) and the beginning of the string after the index against
        // /after/ (with `lookingAt`), but that is very slow, especially for
        // nonempty /before/.  While the old monkeys are not a production
        // implementation, we still do not want them to be too slow, since we
        // need to test millions of sample strings.  Instead we search for
        // /before/ and /after/, and check resulting candidates.  This speeds
        // things up by a factor of ~40.
        // We need to be careful about greedy matching: The first position where
        // the rule matches may be before the end of the first /before/ match.
        // However, it is both:
        //   1. within a /before/ match or at its bounds,
        //   2. at the beginning of an /after/ match.
        // Further, the /before/ context of the rule matches within the
        // aforementioned /before/ match.  Note that we need to look for
        // overlapping matches, thus calls to `find` are always preceded by a
        // reset via `region`.
        std::unique_ptr<RegexMatcher> beforeSearch(before_->matcher(remapped, status));
        std::unique_ptr<RegexMatcher> afterSearch(after_->matcher(remapped, status));
        beforeSearch->useAnchoringBounds(false);
        afterSearch->useAnchoringBounds(false);
        U_ASSERT(U_SUCCESS(status));
        if (beforeSearch->find() && afterSearch->find()) {
            for (;;) {
                if (afterSearch->start(status) < beforeSearch->start(status)) {
                    afterSearch->region(beforeSearch->start(status), remapped.length(), status);
                    if (!afterSearch->find()) {
                        break;
                    }
                } else if (afterSearch->start(status) > beforeSearch->end(status)) {
                    if (beforeSearch->start(status) == remapped.length()) {
                        break;
                    }
                    beforeSearch->region(remapped.moveIndex32(beforeSearch->start(status), 1),
                                         remapped.length(), status);
                    if (!beforeSearch->find()) {
                        break;
                    }
                } else {
                    auto const it = std::find_if(resolved.begin(), resolved.end(), [&](auto r) {
                        return r.indexInRemapped == afterSearch->start(status);
                    });
                    if (it == resolved.end()) {
                        puts(("Rule " + name() +
                              " found a break at a position which does not correspond to an index in "
                              "the original string")
                                 .c_str());
                        std::terminate();
                    }
                    U_ASSERT(U_SUCCESS(status));
                    if (it->appliedRule == nullptr &&
                        std::unique_ptr<RegexMatcher>(endsWithBefore_->matcher(remapped, status))
                            ->useAnchoringBounds(false)
                            .region(beforeSearch->start(status), afterSearch->start(status), status)
                            .matches(status)) {
                        it->appliedRule = this;
                    }
                    if (afterSearch->start(status) == remapped.length()) {
                        break;
                    }
                    afterSearch->region(remapped.moveIndex32(afterSearch->start(status), 1),
                                        remapped.length(), status);
                    if (!afterSearch->find()) {
                        break;
                    }
                }
                U_ASSERT(U_SUCCESS(status));
            }
        }
        if (U_FAILURE(status)) {
            puts(("Failed to apply rule " + name()).c_str());
            std::terminate();
        }
        timeSpent_ += std::chrono::steady_clock::now() - start;
    }

    virtual Resolution resolution() const override { return resolution_; }

  private:
    std::unique_ptr<RegexPattern> before_;
    std::unique_ptr<RegexPattern> endsWithBefore_;
    std::unique_ptr<RegexPattern> after_;
    const Resolution resolution_;
};

}  // namespace

//---------------------------------------------------------------------------------------
//
//   class RBBIMonkeyKind
//
//      Monkey Test for Break Iteration
//      Abstract interface class.   Concrete derived classes independently
//      implement the break rules for different iterator types.
//
//      The Monkey Test itself uses doesn't know which type of break iterator it is
//      testing, but works purely in terms of the interface defined here.
//
//---------------------------------------------------------------------------------------
class RBBIMonkeyKind {
public:
    // Return a vector of UnicodeSets, representing the character classes used
    //   for this type of iterator.
    const std::vector<UnicodeSet> &charClasses();

    const UnicodeSet &dictionarySet() const;

    // Set the test text on which subsequent calls to next() will operate
    void setText(const UnicodeString &s);

    // Find the next break position, starting from the prev break position, or from zero.
    // Return -1 after reaching end of string.
    int32_t next(int32_t i);

    // Name of each character class, parallel with charClasses. Used for debugging output
    // of characters.
    std::vector<std::string> &characterClassNames();

    void setAppliedRule(int32_t position, const char *value);

    std::string getAppliedRule(int32_t position);

    virtual ~RBBIMonkeyKind();
    UErrorCode deferredStatus;

    std::string classNameFromCodepoint(const UChar32 c);
    unsigned int maxClassNameSize();

protected:
    RBBIMonkeyKind();
    std::vector<std::string> classNames;
    std::vector<UnicodeSet> sets;
    std::vector<std::unique_ptr<SegmentationRule>> rules;
    UnicodeSet dictionarySet_;

    // Clear `appliedRules` and fill it with empty strings in the size of test text.
    void prepareAppliedRules(int32_t size);

private:
    std::vector<std::string> appliedRules;
    UnicodeString text;
    std::vector<SegmentationRule::BreakContext> resolved;
};

RBBIMonkeyKind::RBBIMonkeyKind() {
    deferredStatus = U_ZERO_ERROR;
}

RBBIMonkeyKind::~RBBIMonkeyKind() {
}

const std::vector<UnicodeSet> &RBBIMonkeyKind::charClasses() {
    return sets;
}

const UnicodeSet &RBBIMonkeyKind::dictionarySet() const {
    return dictionarySet_;
}

void RBBIMonkeyKind::setText(const UnicodeString &s) {
   text = s;
   prepareAppliedRules(s.length());
   UnicodeString remapped = s;
   resolved.clear();
   resolved.reserve(s.length() + 1);
   for (int i = 0; i <= s.length(); ++i) {
        resolved.emplace_back(i);
   }
   for (const auto &rule : rules) {
        rule->apply(remapped, resolved);
   }
   for (std::size_t i = 0; i < resolved.size(); ++i) {
        if (resolved[i].appliedRule == nullptr) {
            if (i > 0 && U16_IS_LEAD(s[i-1]) && U16_IS_TRAIL(s[i])) {
                continue;
            }
            printf("Failed to resolve at %zu between U+%04X and U+%04X ", i, s.char32At(i - 1),
                   s.char32At(i));
            if (resolved[i].indexInRemapped.has_value()) {
                printf("which is remapped %d between U+%04X and U+%04X", *resolved[i].indexInRemapped,
                       remapped.char32At(*resolved[i].indexInRemapped - 1),
                       remapped.char32At(*resolved[i].indexInRemapped));
            }
            std::terminate();
        } else {
            setAppliedRule(i, resolved[i].appliedRule->name().c_str());
        }
   }
}

int32_t RBBIMonkeyKind::next(int32_t startPos) {
   for (std::size_t i = startPos + 1; i < resolved.size(); ++i) {
        if (resolved[i].appliedRule != nullptr &&
            resolved[i].appliedRule->resolution() == SegmentationRule::BREAK) {
            return i;
        }
   }
   return -1;
}

std::vector<std::string> &RBBIMonkeyKind::characterClassNames() {
    return classNames;
}

void RBBIMonkeyKind::prepareAppliedRules(int32_t size) {
    // Remove all the information in the `appliedRules`.
    appliedRules.clear();
    appliedRules.resize(size + 1);
}

void RBBIMonkeyKind::setAppliedRule(int32_t position, const char* value) {
    appliedRules[position] = value;
}

std::string RBBIMonkeyKind::getAppliedRule(int32_t position){
    return appliedRules[position];
}

std::string RBBIMonkeyKind::classNameFromCodepoint(const UChar32 c) {
    // Simply iterate through charClasses to find character's class
    for (std::size_t aClassNum = 0; aClassNum < sets.size(); aClassNum++) {
        const UnicodeSet &classSet = sets[aClassNum];
        if (classSet.contains(c)) {
            return classNames[aClassNum];
        }
    }
    U_ASSERT(false);  // This should not happen.
    return "bad class name";
}

unsigned int RBBIMonkeyKind::maxClassNameSize() {
    unsigned int maxSize = 0;
    for (std::size_t aClassNum = 0; aClassNum < classNames.size(); aClassNum++) {
        auto aClassNumSize = static_cast<unsigned int>(classNames[aClassNum].size());
        if (aClassNumSize > maxSize) {
            maxSize = aClassNumSize;
        }
    }
    return maxSize;
}

//----------------------------------------------------------------------------------------
//
//   Random Numbers.  We need a long cycle length since we run overnight tests over
//                    millions of strings involving 1000 random generations per string
//                    (a 32-bit LCG will not do!), and we want a reasonably small state
//                    so that we can output it to reproduce failures.
//
//---------------------------------------------------------------------------------------
namespace {

using RandomNumberGenerator = std::ranlux48;
constexpr RandomNumberGenerator::result_type defaultSeed = std::ranlux48_base::default_seed;
static RandomNumberGenerator randomNumberGenerator;

RandomNumberGenerator deserialize(const std::string& state) {
    RandomNumberGenerator result;
    std::stringstream(state) >> result;
    return result;
}

std::string serialize(const RandomNumberGenerator& generator) {
    std::stringstream result;
    result << generator;
    return result.str();
}

}  // namespace

//------------------------------------------------------------------------------------------
//
//   class RBBICharMonkey      Character (Grapheme Cluster) specific implementation
//                             of RBBIMonkeyKind.
//
//------------------------------------------------------------------------------------------
class RBBICharMonkey: public RBBIMonkeyKind {
public:
    RBBICharMonkey();
};


RBBICharMonkey::RBBICharMonkey() {
    UErrorCode status = U_ZERO_ERROR;

    std::list<std::pair<std::string, UnicodeSet>> partition;

    // These two could be part of the rules.
    rules.push_back(std::make_unique<RegexRule>(uR"(GB1 sot ÷ Any)", uR"(^)", u'÷', uR"()"));
    // Note that /$/ matches ( BK | CR | LF | NL ) eot, so we use (?!.) instead.
    // The generated rules use the same (?!.).
    rules.push_back(std::make_unique<RegexRule>(uR"(GB2 Any ÷ eot)", uR"()", u'÷', uR"((?!.))"));

    // --- NOLI ME TANGERE ---
    // Generated by GenerateBreakTest.java in the Unicode tools.
    partition.emplace_back("CR", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=CR}])", status));
    partition.emplace_back("LF", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=LF}])", status));
    partition.emplace_back("Control", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=Control}])", status));
    partition.emplace_back("Extend_ConjunctLinker", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=Extend}&\p{Indic_Conjunct_Break=Linker}])", status));
    partition.emplace_back("Extend_ConjunctExtendermConjunctLinker", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=Extend}&[\p{Indic_Conjunct_Break=Linker}\p{Indic_Conjunct_Break=Extend}]-\p{Indic_Conjunct_Break=Linker}])", status));
    partition.emplace_back("ExtendmConjunctLinkermConjunctExtender", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=Extend}-\p{Indic_Conjunct_Break=Linker}-[\p{Indic_Conjunct_Break=Linker}\p{Indic_Conjunct_Break=Extend}]])", status));
    partition.emplace_back("ZWJ", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=ZWJ}])", status));
    partition.emplace_back("RI", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=Regional_Indicator}])", status));
    partition.emplace_back("Prepend", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=Prepend}])", status));
    partition.emplace_back("SpacingMark", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=SpacingMark}])", status));
    partition.emplace_back("L", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=L}])", status));
    partition.emplace_back("V", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=V}])", status));
    partition.emplace_back("T", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=T}])", status));
    partition.emplace_back("LV", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=LV}])", status));
    partition.emplace_back("LVT", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=LVT}])", status));
    partition.emplace_back("LinkingConsonant", UnicodeSet(uR"([\p{Indic_Conjunct_Break=Consonant}])", status));
    partition.emplace_back("ExtPict", UnicodeSet(uR"([\p{Extended_Pictographic}])", status));
    partition.emplace_back("XXmLinkingConsonantmExtPict", UnicodeSet(uR"([\p{Grapheme_Cluster_Break=Other}-\p{Indic_Conjunct_Break=Consonant}-\p{Extended_Pictographic}])", status));

    rules.push_back(std::make_unique<RegexRule>(uR"($CR × $LF)", uR"(\p{Grapheme_Cluster_Break=CR})", u'×', uR"(\p{Grapheme_Cluster_Break=LF})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(( $Control | $CR | $LF ) ÷)", uR"(( \p{Grapheme_Cluster_Break=Control} | \p{Grapheme_Cluster_Break=CR} | \p{Grapheme_Cluster_Break=LF} ))", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(÷ ( $Control | $CR | $LF ))", uR"()", u'÷', uR"(( \p{Grapheme_Cluster_Break=Control} | \p{Grapheme_Cluster_Break=CR} | \p{Grapheme_Cluster_Break=LF} ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($L × ( $L | $V | $LV | $LVT ))", uR"(\p{Grapheme_Cluster_Break=L})", u'×', uR"(( \p{Grapheme_Cluster_Break=L} | \p{Grapheme_Cluster_Break=V} | \p{Grapheme_Cluster_Break=LV} | \p{Grapheme_Cluster_Break=LVT} ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(( $LV | $V ) × ( $V | $T ))", uR"(( \p{Grapheme_Cluster_Break=LV} | \p{Grapheme_Cluster_Break=V} ))", u'×', uR"(( \p{Grapheme_Cluster_Break=V} | \p{Grapheme_Cluster_Break=T} ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(( $LVT | $T) × $T)", uR"(( \p{Grapheme_Cluster_Break=LVT} | \p{Grapheme_Cluster_Break=T}))", u'×', uR"(\p{Grapheme_Cluster_Break=T})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× ($Extend | $ZWJ))", uR"()", u'×', uR"((\p{Grapheme_Cluster_Break=Extend} | \p{Grapheme_Cluster_Break=ZWJ}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $SpacingMark)", uR"()", u'×', uR"(\p{Grapheme_Cluster_Break=SpacingMark})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Prepend ×)", uR"(\p{Grapheme_Cluster_Break=Prepend})", u'×', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"($LinkingConsonant $ConjunctExtender* $ConjunctLinker $ConjunctExtender* × $LinkingConsonant)", uR"(\p{Indic_Conjunct_Break=Consonant} [\p{Indic_Conjunct_Break=Linker}\p{Indic_Conjunct_Break=Extend}]* \p{Indic_Conjunct_Break=Linker} [\p{Indic_Conjunct_Break=Linker}\p{Indic_Conjunct_Break=Extend}]*)", u'×', uR"(\p{Indic_Conjunct_Break=Consonant})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ExtPict $Extend* $ZWJ × $ExtPict)", uR"(\p{Extended_Pictographic} \p{Grapheme_Cluster_Break=Extend}* \p{Grapheme_Cluster_Break=ZWJ})", u'×', uR"(\p{Extended_Pictographic})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(^ ($RI $RI)* $RI × $RI)", uR"(^ (\p{Grapheme_Cluster_Break=Regional_Indicator} \p{Grapheme_Cluster_Break=Regional_Indicator})* \p{Grapheme_Cluster_Break=Regional_Indicator})", u'×', uR"(\p{Grapheme_Cluster_Break=Regional_Indicator})"));
    rules.push_back(std::make_unique<RegexRule>(uR"([^$RI] ($RI $RI)* $RI × $RI)", uR"([^\p{Grapheme_Cluster_Break=Regional_Indicator}] (\p{Grapheme_Cluster_Break=Regional_Indicator} \p{Grapheme_Cluster_Break=Regional_Indicator})* \p{Grapheme_Cluster_Break=Regional_Indicator})", u'×', uR"(\p{Grapheme_Cluster_Break=Regional_Indicator})"));
    // --- End of generated code. ---

    // TODO(egg): This could just as well be part of the rules…
    rules.push_back(std::make_unique<RegexRule>(uR"(ALL ÷ / ÷ ALL)", uR"()", u'÷', uR"()"));

    for (const auto &[name, set] : partition) {
        sets.push_back(set);
        classNames.push_back(name);
    }

    if (U_FAILURE(status)) {
        deferredStatus = status;
    }
}

//------------------------------------------------------------------------------------------
//
//   class RBBIWordMonkey      Word Break specific implementation
//                             of RBBIMonkeyKind.
//
//------------------------------------------------------------------------------------------
class RBBIWordMonkey: public RBBIMonkeyKind {
public:
    RBBIWordMonkey();
};


RBBIWordMonkey::RBBIWordMonkey()
{
    UErrorCode  status = U_ZERO_ERROR;

    std::list<std::pair<std::string, UnicodeSet>> partition;

    dictionarySet_ = UnicodeSet(uR"([[\uac00-\ud7a3][:Han:][:Hiragana:]])", status);
    dictionarySet_.addAll(UnicodeSet(uR"([\p{Word_Break = Katakana}])", status));
    dictionarySet_.addAll(UnicodeSet(uR"([\p{LineBreak = Complex_Context}])", status));

    // These two could be part of the rules.
    rules.push_back(std::make_unique<RegexRule>(uR"(WB1 sot ÷ Any)", uR"(^)", u'÷', uR"()"));
    // Note that /$/ matches ( BK | CR | LF | NL ) eot, so we use (?!.) instead.
    // The generated rules use the same (?!.).
    rules.push_back(std::make_unique<RegexRule>(uR"(WB2 Any ÷ eot)", uR"()", u'÷', uR"((?!.))"));

    // --- NOLI ME TANGERE ---
    // Generated by GenerateBreakTest.java in the Unicode tools.
    partition.emplace_back("CR", UnicodeSet(uR"([\p{Word_Break=CR}])", status));
    partition.emplace_back("LF", UnicodeSet(uR"([\p{Word_Break=LF}])", status));
    partition.emplace_back("Newline", UnicodeSet(uR"([\p{Word_Break=Newline}])", status));
    partition.emplace_back("Extend", UnicodeSet(uR"([\p{Word_Break=Extend}])", status));
    partition.emplace_back("Format", UnicodeSet(uR"([[\p{Word_Break=Format}]])", status));
    partition.emplace_back("Katakana", UnicodeSet(uR"([\p{Word_Break=Katakana}])", status));
    partition.emplace_back("ALetter_ExtPict", UnicodeSet(uR"([\p{Word_Break=ALetter}&\p{Extended_Pictographic}])", status));
    partition.emplace_back("ALettermExtPict", UnicodeSet(uR"([\p{Word_Break=ALetter}-\p{Extended_Pictographic}])", status));
    partition.emplace_back("MidLetter", UnicodeSet(uR"([\p{Word_Break=MidLetter}])", status));
    partition.emplace_back("MidNum", UnicodeSet(uR"([\p{Word_Break=MidNum}])", status));
    partition.emplace_back("MidNumLet", UnicodeSet(uR"([\p{Word_Break=MidNumLet}])", status));
    partition.emplace_back("Numeric", UnicodeSet(uR"([\p{Word_Break=Numeric}])", status));
    partition.emplace_back("ExtendNumLet", UnicodeSet(uR"([\p{Word_Break=ExtendNumLet}])", status));
    partition.emplace_back("RI", UnicodeSet(uR"([\p{Word_Break=Regional_Indicator}])", status));
    partition.emplace_back("Hebrew_Letter", UnicodeSet(uR"([\p{Word_Break=Hebrew_Letter}])", status));
    partition.emplace_back("Double_Quote", UnicodeSet(uR"([\p{Word_Break=Double_Quote}])", status));
    partition.emplace_back("Single_Quote", UnicodeSet(uR"([\p{Word_Break=Single_Quote}])", status));
    partition.emplace_back("ZWJ", UnicodeSet(uR"([\p{Word_Break=ZWJ}])", status));
    partition.emplace_back("ExtPictmALetter", UnicodeSet(uR"([\p{Extended_Pictographic}-\p{Word_Break=ALetter}])", status));
    partition.emplace_back("WSegSpace", UnicodeSet(uR"([\p{Word_Break=WSegSpace}])", status));
    partition.emplace_back("XXmExtPict", UnicodeSet(uR"([\p{Word_Break=Other}-\p{Extended_Pictographic}])", status));

    rules.push_back(std::make_unique<RegexRule>(uR"($CR × $LF)", uR"(\p{Word_Break=CR})", u'×', uR"(\p{Word_Break=LF})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($Newline | $CR | $LF) ÷)", uR"((\p{Word_Break=Newline} | \p{Word_Break=CR} | \p{Word_Break=LF}))", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(÷ ($Newline | $CR | $LF))", uR"()", u'÷', uR"((\p{Word_Break=Newline} | \p{Word_Break=CR} | \p{Word_Break=LF}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ZWJ × $ExtPict)", uR"(\p{Word_Break=ZWJ})", u'×', uR"(\p{Extended_Pictographic})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($WSegSpace × $WSegSpace)", uR"(\p{Word_Break=WSegSpace})", u'×', uR"(\p{Word_Break=WSegSpace})"));
    rules.push_back(std::make_unique<RemapRule>(uR"((?<X>[^$CR $LF $Newline]) ($Extend | $Format | $ZWJ)* → ${X})", uR"((?<X>[^\p{Word_Break=CR} \p{Word_Break=LF} \p{Word_Break=Newline}]) (\p{Word_Break=Extend} | [\p{Word_Break=Format}] | \p{Word_Break=ZWJ})*)", uR"(${X})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($AHLetter × $AHLetter)", uR"([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}])", u'×', uR"([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($AHLetter × ($MidLetter | $MidNumLetQ) $AHLetter)", uR"([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}])", u'×', uR"((\p{Word_Break=MidLetter} | [\p{Word_Break=MidNumLet} \p{Word_Break=Single_Quote}]) [\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($AHLetter ($MidLetter | $MidNumLetQ) × $AHLetter)", uR"([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}] (\p{Word_Break=MidLetter} | [\p{Word_Break=MidNumLet} \p{Word_Break=Single_Quote}]))", u'×', uR"([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Hebrew_Letter × $Single_Quote)", uR"(\p{Word_Break=Hebrew_Letter})", u'×', uR"(\p{Word_Break=Single_Quote})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Hebrew_Letter × $Double_Quote $Hebrew_Letter)", uR"(\p{Word_Break=Hebrew_Letter})", u'×', uR"(\p{Word_Break=Double_Quote} \p{Word_Break=Hebrew_Letter})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Hebrew_Letter $Double_Quote × $Hebrew_Letter)", uR"(\p{Word_Break=Hebrew_Letter} \p{Word_Break=Double_Quote})", u'×', uR"(\p{Word_Break=Hebrew_Letter})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Numeric × $Numeric)", uR"(\p{Word_Break=Numeric})", u'×', uR"(\p{Word_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($AHLetter × $Numeric)", uR"([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}])", u'×', uR"(\p{Word_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Numeric × $AHLetter)", uR"(\p{Word_Break=Numeric})", u'×', uR"([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Numeric ($MidNum | $MidNumLetQ) × $Numeric)", uR"(\p{Word_Break=Numeric} (\p{Word_Break=MidNum} | [\p{Word_Break=MidNumLet} \p{Word_Break=Single_Quote}]))", u'×', uR"(\p{Word_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Numeric × ($MidNum | $MidNumLetQ) $Numeric)", uR"(\p{Word_Break=Numeric})", u'×', uR"((\p{Word_Break=MidNum} | [\p{Word_Break=MidNumLet} \p{Word_Break=Single_Quote}]) \p{Word_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($Katakana × $Katakana)", uR"(\p{Word_Break=Katakana})", u'×', uR"(\p{Word_Break=Katakana})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($AHLetter | $Numeric | $Katakana | $ExtendNumLet) × $ExtendNumLet)", uR"(([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}] | \p{Word_Break=Numeric} | \p{Word_Break=Katakana} | \p{Word_Break=ExtendNumLet}))", u'×', uR"(\p{Word_Break=ExtendNumLet})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ExtendNumLet × ($AHLetter | $Numeric | $Katakana))", uR"(\p{Word_Break=ExtendNumLet})", u'×', uR"(([\p{Word_Break=ALetter} \p{Word_Break=Hebrew_Letter}] | \p{Word_Break=Numeric} | \p{Word_Break=Katakana}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(^ ($RI $RI)* $RI × $RI)", uR"(^ (\p{Word_Break=Regional_Indicator} \p{Word_Break=Regional_Indicator})* \p{Word_Break=Regional_Indicator})", u'×', uR"(\p{Word_Break=Regional_Indicator})"));
    rules.push_back(std::make_unique<RegexRule>(uR"([^$RI] ($RI $RI)* $RI × $RI)", uR"([^\p{Word_Break=Regional_Indicator}] (\p{Word_Break=Regional_Indicator} \p{Word_Break=Regional_Indicator})* \p{Word_Break=Regional_Indicator})", u'×', uR"(\p{Word_Break=Regional_Indicator})"));
    // --- End of generated code. ---

    // TODO(egg): This could just as well be part of the rules…
    rules.push_back(std::make_unique<RegexRule>(uR"(ALL ÷ / ÷ ALL)", uR"()", u'÷', uR"()"));

    for (const auto &[name, set] : partition) {
        sets.push_back(set);
        classNames.push_back(name);
    }

    if (U_FAILURE(status)) {
        deferredStatus = status;
    }
}

//------------------------------------------------------------------------------------------
//
//   class RBBISentMonkey      Sentence Break specific implementation
//                             of RBBIMonkeyKind.
//
//------------------------------------------------------------------------------------------
class RBBISentMonkey: public RBBIMonkeyKind {
public:
    RBBISentMonkey();
};

RBBISentMonkey::RBBISentMonkey()
{
    UErrorCode status = U_ZERO_ERROR;

    std::list<std::pair<std::string, UnicodeSet>> partition;

    // These two could be part of the rules.
    rules.push_back(std::make_unique<RegexRule>(uR"(SB1 sot ÷ Any)", uR"(^)", u'÷', uR"()"));
    // Note that /$/ matches ( BK | CR | LF | NL ) eot, so we use (?!.) instead.
    // The generated rules use the same (?!.).
    rules.push_back(std::make_unique<RegexRule>(uR"(SB2 Any ÷ eot)", uR"()", u'÷', uR"((?!.))"));

    // --- NOLI ME TANGERE ---
    // Generated by GenerateBreakTest.java in the Unicode tools.
    partition.emplace_back("CR", UnicodeSet(uR"([\p{Sentence_Break=CR}])", status));
    partition.emplace_back("LF", UnicodeSet(uR"([\p{Sentence_Break=LF}])", status));
    partition.emplace_back("Extend", UnicodeSet(uR"([\p{Sentence_Break=Extend}])", status));
    partition.emplace_back("Format", UnicodeSet(uR"([\p{Sentence_Break=Format}])", status));
    partition.emplace_back("Sep", UnicodeSet(uR"([\p{Sentence_Break=Sep}])", status));
    partition.emplace_back("Sp", UnicodeSet(uR"([\p{Sentence_Break=Sp}])", status));
    partition.emplace_back("Lower", UnicodeSet(uR"([\p{Sentence_Break=Lower}])", status));
    partition.emplace_back("Upper", UnicodeSet(uR"([\p{Sentence_Break=Upper}])", status));
    partition.emplace_back("OLetter", UnicodeSet(uR"([\p{Sentence_Break=OLetter}])", status));
    partition.emplace_back("Numeric", UnicodeSet(uR"([\p{Sentence_Break=Numeric}])", status));
    partition.emplace_back("ATerm", UnicodeSet(uR"([\p{Sentence_Break=ATerm}])", status));
    partition.emplace_back("STerm", UnicodeSet(uR"([\p{Sentence_Break=STerm}])", status));
    partition.emplace_back("Close", UnicodeSet(uR"([\p{Sentence_Break=Close}])", status));
    partition.emplace_back("SContinue", UnicodeSet(uR"([\p{Sentence_Break=SContinue}])", status));
    partition.emplace_back("XX", UnicodeSet(uR"([\p{Sentence_Break=Other}])", status));

    rules.push_back(std::make_unique<RegexRule>(uR"($CR × $LF)", uR"(\p{Sentence_Break=CR})", u'×', uR"(\p{Sentence_Break=LF})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ParaSep ÷)", uR"([\p{Sentence_Break=Sep} \p{Sentence_Break=CR} \p{Sentence_Break=LF}])", u'÷', uR"()"));
    rules.push_back(std::make_unique<RemapRule>(uR"((?<X>[^$ParaSep]) ( $Extend | $Format )* → ${X})", uR"((?<X>[^[\p{Sentence_Break=Sep} \p{Sentence_Break=CR} \p{Sentence_Break=LF}]]) ( \p{Sentence_Break=Extend} | \p{Sentence_Break=Format} )*)", uR"(${X})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ATerm × $Numeric)", uR"(\p{Sentence_Break=ATerm})", u'×', uR"(\p{Sentence_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($Upper | $Lower) $ATerm × $Upper)", uR"((\p{Sentence_Break=Upper} | \p{Sentence_Break=Lower}) \p{Sentence_Break=ATerm})", u'×', uR"(\p{Sentence_Break=Upper})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ATerm $Close* $Sp* × [^ $OLetter $Upper $Lower $ParaSep $SATerm]* $Lower)", uR"(\p{Sentence_Break=ATerm} \p{Sentence_Break=Close}* \p{Sentence_Break=Sp}*)", u'×', uR"([^ \p{Sentence_Break=OLetter} \p{Sentence_Break=Upper} \p{Sentence_Break=Lower} [\p{Sentence_Break=Sep} \p{Sentence_Break=CR} \p{Sentence_Break=LF}] [\p{Sentence_Break=STerm} \p{Sentence_Break=ATerm}]]* \p{Sentence_Break=Lower})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($SATerm $Close* $Sp* × ($SContinue | $SATerm))", uR"([\p{Sentence_Break=STerm} \p{Sentence_Break=ATerm}] \p{Sentence_Break=Close}* \p{Sentence_Break=Sp}*)", u'×', uR"((\p{Sentence_Break=SContinue} | [\p{Sentence_Break=STerm} \p{Sentence_Break=ATerm}]))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($SATerm $Close* × ( $Close | $Sp | $ParaSep ))", uR"([\p{Sentence_Break=STerm} \p{Sentence_Break=ATerm}] \p{Sentence_Break=Close}*)", u'×', uR"(( \p{Sentence_Break=Close} | \p{Sentence_Break=Sp} | [\p{Sentence_Break=Sep} \p{Sentence_Break=CR} \p{Sentence_Break=LF}] ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($SATerm $Close* $Sp* × ( $Sp | $ParaSep ))", uR"([\p{Sentence_Break=STerm} \p{Sentence_Break=ATerm}] \p{Sentence_Break=Close}* \p{Sentence_Break=Sp}*)", u'×', uR"(( \p{Sentence_Break=Sp} | [\p{Sentence_Break=Sep} \p{Sentence_Break=CR} \p{Sentence_Break=LF}] ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($SATerm $Close* $Sp* $ParaSep? ÷)", uR"([\p{Sentence_Break=STerm} \p{Sentence_Break=ATerm}] \p{Sentence_Break=Close}* \p{Sentence_Break=Sp}* [\p{Sentence_Break=Sep} \p{Sentence_Break=CR} \p{Sentence_Break=LF}]?)", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $Any)", uR"()", u'×', uR"(.)"));
    // --- End of generated code. ---

    for (const auto &[name, set] : partition) {
        sets.push_back(set);
        classNames.push_back(name);
    }

    if (U_FAILURE(status)) {
        deferredStatus = status;
    }
}

//-------------------------------------------------------------------------------------------
//
//  RBBILineMonkey
//
//-------------------------------------------------------------------------------------------

class RBBILineMonkey: public RBBIMonkeyKind {
public:
    RBBILineMonkey();
private:
};

RBBILineMonkey::RBBILineMonkey() :
    RBBIMonkeyKind()
{
    if (U_FAILURE(deferredStatus)) {
        return;
    }

    UErrorCode  status = U_ZERO_ERROR;

    if (U_FAILURE(status)) {
        deferredStatus = status;
        return;
    }

    std::list<std::pair<std::string, UnicodeSet>> partition;

    rules.push_back(std::make_unique<RegexRule>(uR"(sot ÷ contra LB2)", uR"(^)", u'÷', uR"()"));
    // This one could be part of the rules.
    // Note that /$/ matches ( BK | CR | LF | NL ) eot, so we use (?!.) instead.
    // The generated rules use the same (?!.).
    rules.push_back(std::make_unique<RegexRule>(uR"(LB3 ÷ eot)", uR"()", u'÷', uR"((?!.))"));

    // --- NOLI ME TANGERE ---
    // Generated by GenerateBreakTest.java in the Unicode tools.
    partition.emplace_back("AI_EastAsian", UnicodeSet(uR"([\p{Line_Break=Ambiguous}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("AImEastAsian", UnicodeSet(uR"([\p{Line_Break=Ambiguous}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("AK", UnicodeSet(uR"([\p{Line_Break=Aksara}])", status));
    partition.emplace_back("ALorig_EastAsian", UnicodeSet(uR"([\p{Line_Break=Alphabetic}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("ALorig_DottedCircle", UnicodeSet(uR"([\p{Line_Break=Alphabetic}&[◌]])", status));
    partition.emplace_back("ALorigmEastAsianmDottedCircle", UnicodeSet(uR"([\p{Line_Break=Alphabetic}-[\p{ea=F}\p{ea=W}\p{ea=H}]-[◌]])", status));
    partition.emplace_back("AP", UnicodeSet(uR"([\p{Line_Break=Aksara_Prebase}])", status));
    partition.emplace_back("AS", UnicodeSet(uR"([\p{Line_Break=Aksara_Start}])", status));
    partition.emplace_back("B2", UnicodeSet(uR"([\p{Line_Break=Break_Both}])", status));
    partition.emplace_back("BA_EastAsian", UnicodeSet(uR"([\p{Line_Break=Break_After}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("BAmEastAsian", UnicodeSet(uR"([\p{Line_Break=Break_After}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("BB", UnicodeSet(uR"([\p{Line_Break=Break_Before}])", status));
    partition.emplace_back("BK", UnicodeSet(uR"([\p{Line_Break=Mandatory_Break}])", status));
    partition.emplace_back("CB", UnicodeSet(uR"([\p{Line_Break=Contingent_Break}])", status));
    partition.emplace_back("CL_EastAsian", UnicodeSet(uR"([\p{Line_Break=Close_Punctuation}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("CLmEastAsian", UnicodeSet(uR"([\p{Line_Break=Close_Punctuation}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("CP", UnicodeSet(uR"([\p{Line_Break=CP}])", status));
    partition.emplace_back("CMorig_EastAsian", UnicodeSet(uR"([\p{Line_Break=Combining_Mark}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("CMorigmEastAsian", UnicodeSet(uR"([\p{Line_Break=Combining_Mark}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("CR", UnicodeSet(uR"([\p{Line_Break=Carriage_Return}])", status));
    partition.emplace_back("EX_EastAsian", UnicodeSet(uR"([\p{Line_Break=Exclamation}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("EXmEastAsian", UnicodeSet(uR"([\p{Line_Break=Exclamation}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("GL_EastAsian", UnicodeSet(uR"([\p{Line_Break=Glue}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("GLmEastAsian", UnicodeSet(uR"([\p{Line_Break=Glue}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("H2", UnicodeSet(uR"([\p{Line_Break=H2}])", status));
    partition.emplace_back("H3", UnicodeSet(uR"([\p{Line_Break=H3}])", status));
    partition.emplace_back("HH", UnicodeSet(uR"([\p{Line_Break=Unambiguous_Hyphen}])", status));
    partition.emplace_back("HL", UnicodeSet(uR"([\p{Line_Break=HL}])", status));
    partition.emplace_back("HY", UnicodeSet(uR"([\p{Line_Break=Hyphen}])", status));
    partition.emplace_back("ID_EastAsian", UnicodeSet(uR"([\p{Line_Break=Ideographic}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("ID_ExtPictUnassigned", UnicodeSet(uR"([\p{Line_Break=Ideographic}&[\p{Extended_Pictographic=True}&\p{gc=Cn}]])", status));
    partition.emplace_back("IDmEastAsianmExtPictUnassigned", UnicodeSet(uR"([\p{Line_Break=Ideographic}-[\p{ea=F}\p{ea=W}\p{ea=H}]-[\p{Extended_Pictographic=True}&\p{gc=Cn}]])", status));
    partition.emplace_back("IN_EastAsian", UnicodeSet(uR"([\p{Line_Break=Inseparable}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("INmEastAsian", UnicodeSet(uR"([\p{Line_Break=Inseparable}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("IS", UnicodeSet(uR"([\p{Line_Break=Infix_Numeric}])", status));
    partition.emplace_back("JL", UnicodeSet(uR"([\p{Line_Break=JL}])", status));
    partition.emplace_back("JT", UnicodeSet(uR"([\p{Line_Break=JT}])", status));
    partition.emplace_back("JV", UnicodeSet(uR"([\p{Line_Break=JV}])", status));
    partition.emplace_back("LF", UnicodeSet(uR"([\p{Line_Break=Line_Feed}])", status));
    partition.emplace_back("NL", UnicodeSet(uR"([\p{Line_Break=Next_Line}])", status));
    partition.emplace_back("NSorig_EastAsian", UnicodeSet(uR"([\p{Line_Break=Nonstarter}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("NSorigmEastAsian", UnicodeSet(uR"([\p{Line_Break=Nonstarter}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("NU", UnicodeSet(uR"([\p{Line_Break=Numeric}])", status));
    partition.emplace_back("OP_EastAsian", UnicodeSet(uR"([\p{Line_Break=Open_Punctuation}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("OPmEastAsian", UnicodeSet(uR"([\p{Line_Break=Open_Punctuation}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("PO_EastAsian", UnicodeSet(uR"([\p{Line_Break=Postfix_Numeric}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("POmEastAsian", UnicodeSet(uR"([\p{Line_Break=Postfix_Numeric}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("PR_EastAsian", UnicodeSet(uR"([\p{Line_Break=Prefix_Numeric}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("PRmEastAsian", UnicodeSet(uR"([\p{Line_Break=Prefix_Numeric}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("QU_Pi", UnicodeSet(uR"([\p{Line_Break=Quotation}&\p{gc=Pi}])", status));
    partition.emplace_back("QU_Pf", UnicodeSet(uR"([\p{Line_Break=Quotation}&\p{gc=Pf}])", status));
    partition.emplace_back("QUmPimPf", UnicodeSet(uR"([\p{Line_Break=Quotation}-\p{gc=Pi}-\p{gc=Pf}])", status));
    partition.emplace_back("SA_Mn", UnicodeSet(uR"([[\p{Line_Break=Complex_Context}&\p{gc=Mn}]])", status));
    partition.emplace_back("SA_Mc", UnicodeSet(uR"([[\p{Line_Break=Complex_Context}&\p{gc=Mc}]])", status));
    partition.emplace_back("SAmMnmMc", UnicodeSet(uR"([[\p{Line_Break=Complex_Context}-\p{gc=Mn}-\p{gc=Mc}]])", status));
    partition.emplace_back("SG", UnicodeSet(uR"([\p{Line_Break=Surrogate}])", status));
    partition.emplace_back("SP", UnicodeSet(uR"([\p{Line_Break=Space}])", status));
    partition.emplace_back("SY", UnicodeSet(uR"([\p{Line_Break=Break_Symbols}])", status));
    partition.emplace_back("VF", UnicodeSet(uR"([\p{Line_Break=Virama_Final}])", status));
    partition.emplace_back("VI", UnicodeSet(uR"([\p{Line_Break=Virama}])", status));
    partition.emplace_back("WJ", UnicodeSet(uR"([\p{Line_Break=Word_Joiner}])", status));
    partition.emplace_back("XX_ExtPictUnassigned", UnicodeSet(uR"([\p{Line_Break=Unknown}&[\p{Extended_Pictographic=True}&\p{gc=Cn}]])", status));
    partition.emplace_back("XXmExtPictUnassigned", UnicodeSet(uR"([\p{Line_Break=Unknown}-[\p{Extended_Pictographic=True}&\p{gc=Cn}]])", status));
    partition.emplace_back("ZW", UnicodeSet(uR"([\p{Line_Break=ZWSpace}])", status));
    partition.emplace_back("CJ", UnicodeSet(uR"([\p{Line_Break=Conditional_Japanese_Starter}])", status));
    partition.emplace_back("RI", UnicodeSet(uR"([\p{Line_Break=Regional_Indicator}])", status));
    partition.emplace_back("EB_EastAsian", UnicodeSet(uR"([\p{Line_Break=E_Base}&[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("EBmEastAsian", UnicodeSet(uR"([\p{Line_Break=E_Base}-[\p{ea=F}\p{ea=W}\p{ea=H}]])", status));
    partition.emplace_back("EM", UnicodeSet(uR"([\p{Line_Break=E_Modifier}])", status));
    partition.emplace_back("ZWJ", UnicodeSet(uR"([\p{Line_Break=ZWJ}])", status));

    rules.push_back(std::make_unique<RegexRule>(uR"($BK ÷)", uR"(\p{Line_Break=Mandatory_Break})", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"($CR × $LF)", uR"(\p{Line_Break=Carriage_Return})", u'×', uR"(\p{Line_Break=Line_Feed})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($CR ÷)", uR"(\p{Line_Break=Carriage_Return})", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"($LF ÷)", uR"(\p{Line_Break=Line_Feed})", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NL ÷)", uR"(\p{Line_Break=Next_Line})", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× ( $BK | $CR | $LF | $NL ))", uR"()", u'×', uR"(( \p{Line_Break=Mandatory_Break} | \p{Line_Break=Carriage_Return} | \p{Line_Break=Line_Feed} | \p{Line_Break=Next_Line} ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $SP)", uR"()", u'×', uR"(\p{Line_Break=Space})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $ZW)", uR"()", u'×', uR"(\p{Line_Break=ZWSpace})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ZW $SP* ÷)", uR"(\p{Line_Break=ZWSpace} \p{Line_Break=Space}*)", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ZWJ ×)", uR"(\p{Line_Break=ZWJ})", u'×', uR"()"));
    rules.push_back(std::make_unique<RemapRule>(uR"((?<X>[^$BK $CR $LF $NL $SP $ZW]) ( $CM | $ZWJ )* → ${X})", uR"((?<X>[^\p{Line_Break=Mandatory_Break} \p{Line_Break=Carriage_Return} \p{Line_Break=Line_Feed} \p{Line_Break=Next_Line} \p{Line_Break=Space} \p{Line_Break=ZWSpace}]) ( [\p{Line_Break=Combining_Mark} [\p{Line_Break=Complex_Context}&&\p{gc=Mn}] [\p{Line_Break=Complex_Context}&&\p{gc=Mc}]] | \p{Line_Break=ZWJ} )*)", uR"(${X})"));
    rules.push_back(std::make_unique<RemapRule>(uR"(( $CM | $ZWJ ) → A)", uR"(( [\p{Line_Break=Combining_Mark} [\p{Line_Break=Complex_Context}&&\p{gc=Mn}] [\p{Line_Break=Complex_Context}&&\p{gc=Mc}]] | \p{Line_Break=ZWJ} ))", uR"(A)"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $WJ)", uR"()", u'×', uR"(\p{Line_Break=Word_Joiner})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($WJ ×)", uR"(\p{Line_Break=Word_Joiner})", u'×', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"($GL ×)", uR"(\p{Line_Break=Glue})", u'×', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"([^ $SP $BA $HY $HH] × $GL)", uR"([^ \p{Line_Break=Space} \p{Line_Break=Break_After} \p{Line_Break=Hyphen} \p{Line_Break=Unambiguous_Hyphen}])", u'×', uR"(\p{Line_Break=Glue})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $EX)", uR"()", u'×', uR"(\p{Line_Break=Exclamation})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $CL)", uR"()", u'×', uR"(\p{Line_Break=Close_Punctuation})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $CP)", uR"()", u'×', uR"(\p{Line_Break=CP})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $SY)", uR"()", u'×', uR"(\p{Line_Break=Break_Symbols})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($OP $SP* ×)", uR"(\p{Line_Break=Open_Punctuation} \p{Line_Break=Space}*)", u'×', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(( $BK | $CR | $LF | $NL | $OP | $QU | $GL | $SP | $ZW | $sot ) $QU_Pi $SP* ×)", uR"(( \p{Line_Break=Mandatory_Break} | \p{Line_Break=Carriage_Return} | \p{Line_Break=Line_Feed} | \p{Line_Break=Next_Line} | \p{Line_Break=Open_Punctuation} | \p{Line_Break=Quotation} | \p{Line_Break=Glue} | \p{Line_Break=Space} | \p{Line_Break=ZWSpace} | ^ ) [\p{Line_Break=Quotation} && \p{gc=Pi}] \p{Line_Break=Space}*)", u'×', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $QU_Pf ( $SP | $GL | $WJ | $CL | $QU | $CP | $EX | $IS | $SY | $BK | $CR | $LF | $NL | $ZW | $eot ))", uR"()", u'×', uR"([\p{Line_Break=Quotation} && \p{gc=Pf}] ( \p{Line_Break=Space} | \p{Line_Break=Glue} | \p{Line_Break=Word_Joiner} | \p{Line_Break=Close_Punctuation} | \p{Line_Break=Quotation} | \p{Line_Break=CP} | \p{Line_Break=Exclamation} | \p{Line_Break=Infix_Numeric} | \p{Line_Break=Break_Symbols} | \p{Line_Break=Mandatory_Break} | \p{Line_Break=Carriage_Return} | \p{Line_Break=Line_Feed} | \p{Line_Break=Next_Line} | \p{Line_Break=ZWSpace} | (?!.) ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($SP ÷ $IS $NU)", uR"(\p{Line_Break=Space})", u'÷', uR"(\p{Line_Break=Infix_Numeric} \p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $IS)", uR"()", u'×', uR"(\p{Line_Break=Infix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($CL | $CP) $SP* × $NS)", uR"((\p{Line_Break=Close_Punctuation} | \p{Line_Break=CP}) \p{Line_Break=Space}*)", u'×', uR"([\p{Line_Break=Nonstarter} \p{Line_Break=Conditional_Japanese_Starter}])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($B2 $SP* × $B2)", uR"(\p{Line_Break=Break_Both} \p{Line_Break=Space}*)", u'×', uR"(\p{Line_Break=Break_Both})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($SP ÷)", uR"(\p{Line_Break=Space})", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $QUmPi)", uR"()", u'×', uR"([\p{Line_Break=Quotation} -- \p{gc=Pi}])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($QUmPf ×)", uR"([\p{Line_Break=Quotation} -- \p{gc=Pf}])", u'×', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"([^$EastAsian] × $QU)", uR"([^[\p{ea=F}\p{ea=W}\p{ea=H}]])", u'×', uR"(\p{Line_Break=Quotation})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $QU ( [^$EastAsian] | $eot ))", uR"()", u'×', uR"(\p{Line_Break=Quotation} ( [^[\p{ea=F}\p{ea=W}\p{ea=H}]] | (?!.) ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($QU × [^$EastAsian])", uR"(\p{Line_Break=Quotation})", u'×', uR"([^[\p{ea=F}\p{ea=W}\p{ea=H}]])"));
    rules.push_back(std::make_unique<RegexRule>(uR"(( [^$EastAsian] | $sot ) $QU ×)", uR"(( [^[\p{ea=F}\p{ea=W}\p{ea=H}]] | ^ ) \p{Line_Break=Quotation})", u'×', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(÷ $CB)", uR"()", u'÷', uR"(\p{Line_Break=Contingent_Break})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($CB ÷)", uR"(\p{Line_Break=Contingent_Break})", u'÷', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"(( $BK | $CR | $LF | $NL | $SP | $ZW | $CB | $GL | $sot ) ( $HY | $HH ) × ( $AL | $HL ))", uR"(( \p{Line_Break=Mandatory_Break} | \p{Line_Break=Carriage_Return} | \p{Line_Break=Line_Feed} | \p{Line_Break=Next_Line} | \p{Line_Break=Space} | \p{Line_Break=ZWSpace} | \p{Line_Break=Contingent_Break} | \p{Line_Break=Glue} | ^ ) ( \p{Line_Break=Hyphen} | \p{Line_Break=Unambiguous_Hyphen} ))", u'×', uR"(( [\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL} ))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $BA)", uR"()", u'×', uR"(\p{Line_Break=Break_After})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $HH)", uR"()", u'×', uR"(\p{Line_Break=Unambiguous_Hyphen})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $HY)", uR"()", u'×', uR"(\p{Line_Break=Hyphen})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $NS)", uR"()", u'×', uR"([\p{Line_Break=Nonstarter} \p{Line_Break=Conditional_Japanese_Starter}])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($BB ×)", uR"(\p{Line_Break=Break_Before})", u'×', uR"()"));
    rules.push_back(std::make_unique<RegexRule>(uR"($HL ($HY | $HH) × [^$HL])", uR"(\p{Line_Break=HL} (\p{Line_Break=Hyphen} | \p{Line_Break=Unambiguous_Hyphen}))", u'×', uR"([^\p{Line_Break=HL}])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($SY × $HL)", uR"(\p{Line_Break=Break_Symbols})", u'×', uR"(\p{Line_Break=HL})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(× $IN)", uR"()", u'×', uR"(\p{Line_Break=Inseparable})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($AL | $HL) × $NU)", uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL}))", u'×', uR"(\p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NU × ($AL | $HL))", uR"(\p{Line_Break=Numeric})", u'×', uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($PR × ($ID | $EB | $EM))", uR"(\p{Line_Break=Prefix_Numeric})", u'×', uR"((\p{Line_Break=Ideographic} | \p{Line_Break=E_Base} | \p{Line_Break=E_Modifier}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($ID | $EB | $EM) × $PO)", uR"((\p{Line_Break=Ideographic} | \p{Line_Break=E_Base} | \p{Line_Break=E_Modifier}))", u'×', uR"(\p{Line_Break=Postfix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($PR | $PO) × ($AL | $HL))", uR"((\p{Line_Break=Prefix_Numeric} | \p{Line_Break=Postfix_Numeric}))", u'×', uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($AL | $HL) × ($PR | $PO))", uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL}))", u'×', uR"((\p{Line_Break=Prefix_Numeric} | \p{Line_Break=Postfix_Numeric}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NU ( $SY | $IS )* $CL × $PO)", uR"(\p{Line_Break=Numeric} ( \p{Line_Break=Break_Symbols} | \p{Line_Break=Infix_Numeric} )* \p{Line_Break=Close_Punctuation})", u'×', uR"(\p{Line_Break=Postfix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NU ( $SY | $IS )* $CP × $PO)", uR"(\p{Line_Break=Numeric} ( \p{Line_Break=Break_Symbols} | \p{Line_Break=Infix_Numeric} )* \p{Line_Break=CP})", u'×', uR"(\p{Line_Break=Postfix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NU ( $SY | $IS )* $CL × $PR)", uR"(\p{Line_Break=Numeric} ( \p{Line_Break=Break_Symbols} | \p{Line_Break=Infix_Numeric} )* \p{Line_Break=Close_Punctuation})", u'×', uR"(\p{Line_Break=Prefix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NU ( $SY | $IS )* $CP × $PR)", uR"(\p{Line_Break=Numeric} ( \p{Line_Break=Break_Symbols} | \p{Line_Break=Infix_Numeric} )* \p{Line_Break=CP})", u'×', uR"(\p{Line_Break=Prefix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NU ( $SY | $IS )* × $PO)", uR"(\p{Line_Break=Numeric} ( \p{Line_Break=Break_Symbols} | \p{Line_Break=Infix_Numeric} )*)", u'×', uR"(\p{Line_Break=Postfix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NU ( $SY | $IS )* × $PR)", uR"(\p{Line_Break=Numeric} ( \p{Line_Break=Break_Symbols} | \p{Line_Break=Infix_Numeric} )*)", u'×', uR"(\p{Line_Break=Prefix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($PO × $OP $NU)", uR"(\p{Line_Break=Postfix_Numeric})", u'×', uR"(\p{Line_Break=Open_Punctuation} \p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($PO × $OP $IS $NU)", uR"(\p{Line_Break=Postfix_Numeric})", u'×', uR"(\p{Line_Break=Open_Punctuation} \p{Line_Break=Infix_Numeric} \p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($PO × $NU)", uR"(\p{Line_Break=Postfix_Numeric})", u'×', uR"(\p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($PR × $OP $NU)", uR"(\p{Line_Break=Prefix_Numeric})", u'×', uR"(\p{Line_Break=Open_Punctuation} \p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($PR × $OP $IS $NU)", uR"(\p{Line_Break=Prefix_Numeric})", u'×', uR"(\p{Line_Break=Open_Punctuation} \p{Line_Break=Infix_Numeric} \p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($PR × $NU)", uR"(\p{Line_Break=Prefix_Numeric})", u'×', uR"(\p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($HY × $NU)", uR"(\p{Line_Break=Hyphen})", u'×', uR"(\p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($IS × $NU)", uR"(\p{Line_Break=Infix_Numeric})", u'×', uR"(\p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($NU ( $SY | $IS )* × $NU)", uR"(\p{Line_Break=Numeric} ( \p{Line_Break=Break_Symbols} | \p{Line_Break=Infix_Numeric} )*)", u'×', uR"(\p{Line_Break=Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($JL × $JL | $JV | $H2 | $H3)", uR"(\p{Line_Break=JL})", u'×', uR"(\p{Line_Break=JL} | \p{Line_Break=JV} | \p{Line_Break=H2} | \p{Line_Break=H3})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($JV | $H2 × $JV | $JT)", uR"(\p{Line_Break=JV} | \p{Line_Break=H2})", u'×', uR"(\p{Line_Break=JV} | \p{Line_Break=JT})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($JT | $H3 × $JT)", uR"(\p{Line_Break=JT} | \p{Line_Break=H3})", u'×', uR"(\p{Line_Break=JT})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($JL | $JV | $JT | $H2 | $H3 × $PO)", uR"(\p{Line_Break=JL} | \p{Line_Break=JV} | \p{Line_Break=JT} | \p{Line_Break=H2} | \p{Line_Break=H3})", u'×', uR"(\p{Line_Break=Postfix_Numeric})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($PR × $JL | $JV | $JT | $H2 | $H3)", uR"(\p{Line_Break=Prefix_Numeric})", u'×', uR"(\p{Line_Break=JL} | \p{Line_Break=JV} | \p{Line_Break=JT} | \p{Line_Break=H2} | \p{Line_Break=H3})"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($AL | $HL) × ($AL | $HL))", uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL}))", u'×', uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($AP × ($AK | $DottedCircle | $AS))", uR"(\p{Line_Break=Aksara_Prebase})", u'×', uR"((\p{Line_Break=Aksara} | [◌] | \p{Line_Break=Aksara_Start}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($AK | $DottedCircle | $AS) × ($VF | $VI))", uR"((\p{Line_Break=Aksara} | [◌] | \p{Line_Break=Aksara_Start}))", u'×', uR"((\p{Line_Break=Virama_Final} | \p{Line_Break=Virama}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($AK | $DottedCircle | $AS) $VI × ($AK | $DottedCircle))", uR"((\p{Line_Break=Aksara} | [◌] | \p{Line_Break=Aksara_Start}) \p{Line_Break=Virama})", u'×', uR"((\p{Line_Break=Aksara} | [◌]))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($AK | $DottedCircle | $AS) × ($AK | $DottedCircle | $AS) $VF)", uR"((\p{Line_Break=Aksara} | [◌] | \p{Line_Break=Aksara_Start}))", u'×', uR"((\p{Line_Break=Aksara} | [◌] | \p{Line_Break=Aksara_Start}) \p{Line_Break=Virama_Final})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($IS × ($AL | $HL))", uR"(\p{Line_Break=Infix_Numeric})", u'×', uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"(($AL | $HL | $NU) × $OPmEastAsian)", uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL} | \p{Line_Break=Numeric}))", u'×', uR"([\p{Line_Break=Open_Punctuation}--[\p{ea=F}\p{ea=W}\p{ea=H}]])"));
    rules.push_back(std::make_unique<RegexRule>(uR"($CPmEastAsian × ($AL | $HL | $NU))", uR"([\p{Line_Break=CP}--[\p{ea=F}\p{ea=W}\p{ea=H}]])", u'×', uR"(([\p{Line_Break=Ambiguous} \p{Line_Break=Alphabetic} \p{Line_Break=Surrogate} \p{Line_Break=Unknown} [\p{Line_Break=Complex_Context}--\p{gc=Mn}--\p{gc=Mc}]] | \p{Line_Break=HL} | \p{Line_Break=Numeric}))"));
    rules.push_back(std::make_unique<RegexRule>(uR"($sot ($RI $RI)* $RI × $RI)", uR"(^ (\p{Line_Break=Regional_Indicator} \p{Line_Break=Regional_Indicator})* \p{Line_Break=Regional_Indicator})", u'×', uR"(\p{Line_Break=Regional_Indicator})"));
    rules.push_back(std::make_unique<RegexRule>(uR"([^$RI] ($RI $RI)* $RI × $RI)", uR"([^\p{Line_Break=Regional_Indicator}] (\p{Line_Break=Regional_Indicator} \p{Line_Break=Regional_Indicator})* \p{Line_Break=Regional_Indicator})", u'×', uR"(\p{Line_Break=Regional_Indicator})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($RI ÷ $RI)", uR"(\p{Line_Break=Regional_Indicator})", u'÷', uR"(\p{Line_Break=Regional_Indicator})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($EB × $EM)", uR"(\p{Line_Break=E_Base})", u'×', uR"(\p{Line_Break=E_Modifier})"));
    rules.push_back(std::make_unique<RegexRule>(uR"($ExtPictUnassigned × $EM)", uR"([\p{Extended_Pictographic=True}&&\p{gc=Cn}])", u'×', uR"(\p{Line_Break=E_Modifier})"));
    // --- End of generated code. ---



    // TODO(egg): This could just as well be part of the rules…
    rules.push_back(std::make_unique<RegexRule>(uR"(ALL ÷ / ÷ ALL)",
                                                uR"()", u'÷',
                                                uR"()"));

    dictionarySet_ = UnicodeSet(uR"(\p{lb=SA})", status);

    for (const auto &[name, set] : partition) {
        sets.push_back(set);
        classNames.push_back(name);
    }

    if (U_FAILURE(status)) {
        deferredStatus = status;
    }
}


//-------------------------------------------------------------------------------------------
//
//   TestMonkey
//
//     params
//       seed=nnnnn        Random number starting seed.
//                         Setting the seed allows errors to be reproduced.
//       loop=nnn          Looping count.  Controls running time.
//                         -1:  run forever.
//                          0 or greater:  run length.
//
//       type = char | word | line | sent | title
//
//       export = (path)   Export test cases to (path)_(type).txt in the UCD
//                         test case format.
//
//  Example:
//     intltest  rbbi/RBBITest/TestMonkey@"type=line loop=-1"
//
//-------------------------------------------------------------------------------------------

static int64_t  getIntParam(UnicodeString name, UnicodeString &params, int64_t defaultVal) {
    int64_t val = defaultVal;
    name.append(" *= *(-?\\d+)");
    UErrorCode status = U_ZERO_ERROR;
    RegexMatcher m(name, params, 0, status);
    if (m.find()) {
        // The param exists.  Convert the string to an int.
        char valString[100];
        int32_t paramLength = m.end(1, status) - m.start(1, status);
        if (paramLength >= static_cast<int32_t>(sizeof(valString) - 1)) {
            paramLength = static_cast<int32_t>(sizeof(valString) - 2);
        }
        params.extract(m.start(1, status), paramLength, valString, sizeof(valString));
        val = strtoll(valString, nullptr, 10);

        // Delete this parameter from the params string.
        m.reset();
        params = m.replaceFirst("", status);
    }
    U_ASSERT(U_SUCCESS(status));
    return val;
}
#endif

#if !UCONFIG_NO_REGULAR_EXPRESSIONS
static void testBreakBoundPreceding(RBBITest *test, UnicodeString ustr,
                                    BreakIterator *bi,
                                    int expected[],
                                    int expectedcount)
{
    int count = 0;
    int i = 0;
    int forward[50];
    bi->setText(ustr);
    for (i = bi->first(); i != BreakIterator::DONE; i = bi->next()) {
        forward[count] = i;
        if (count < expectedcount && expected[count] != i) {
            test->errln("%s:%d break forward test failed: expected %d but got %d",
                        __FILE__, __LINE__, expected[count], i);
            break;
        }
        count ++;
    }
    if (count != expectedcount) {
        printStringBreaks(ustr, expected, expectedcount);
        test->errln("%s:%d break forward test failed: missed %d match",
                    __FILE__, __LINE__, expectedcount - count);
        return;
    }
    // testing boundaries
    for (i = 1; i < expectedcount; i ++) {
        int j = expected[i - 1];
        if (!bi->isBoundary(j)) {
            printStringBreaks(ustr, expected, expectedcount);
            test->errln("%s:%d isBoundary() failed.  Expected boundary at position %d",
                    __FILE__, __LINE__, j);
            return;
        }
        for (j = expected[i - 1] + 1; j < expected[i]; j ++) {
            if (bi->isBoundary(j)) {
                printStringBreaks(ustr, expected, expectedcount);
                test->errln("%s:%d isBoundary() failed.  Not expecting boundary at position %d",
                    __FILE__, __LINE__, j);
                return;
            }
        }
    }

    for (i = bi->last(); i != BreakIterator::DONE; i = bi->previous()) {
        count --;
        if (forward[count] != i) {
            printStringBreaks(ustr, expected, expectedcount);
            test->errln("%s:%d happy break test previous() failed: expected %d but got %d",
                        __FILE__, __LINE__, forward[count], i);
            break;
        }
    }
    if (count != 0) {
        printStringBreaks(ustr, expected, expectedcount);
        test->errln("break test previous() failed: missed a match");
        return;
    }

    // testing preceding
    for (i = 0; i < expectedcount - 1; i ++) {
        // int j = expected[i] + 1;
        int j = ustr.moveIndex32(expected[i], 1);
        for (; j <= expected[i + 1]; j ++) {
            int32_t expectedPreceding = expected[i];
            int32_t actualPreceding = bi->preceding(j);
            if (actualPreceding != expectedPreceding) {
                printStringBreaks(ustr, expected, expectedcount);
                test->errln("%s:%d preceding(%d): expected %d, got %d",
                        __FILE__, __LINE__, j, expectedPreceding, actualPreceding);
                return;
            }
        }
    }
}
#endif

// The following tests test random strings chosen in 2003, see
// https://github.com/unicode-org/icu/commit/469c2d5b76138869c3c78eb49affe04970600e0e.
// Presumably they were interesting failures found by the monkey tests at the time.
// They are made redundant by any reasonably long run of the monkey tests, but are mostly harmless.
// However, if the expansion of dictionary-based segmentation happens to encompass the randomly
// chosen characters, the offending tests should be removed: only rule-based segmentation is in
// scope for these tests.

void RBBITest::TestWordBreaks()
{
#if !UCONFIG_NO_REGULAR_EXPRESSIONS

    Locale        locale("en");
    UErrorCode    status = U_ZERO_ERROR;
    // BreakIterator  *bi = BreakIterator::createCharacterInstance(locale, status);
    BreakIterator *bi = BreakIterator::createWordInstance(locale, status);
    static const char *strlist[] =
    {
    "\\U000e0037\\u2666\\u1202\\u003a\\U000e0031\\u064d\\u0bea\\u091c\\U000e0040\\u003b",
    "\\u200e\\U000e0072\\u0a4b\\U000e003f\\ufd2b\\u2027\\u002e\\u002e",
    "\\U000e0042\\u002e\\u0fb8\\u09ef\\u0ed1\\u2044",
    "\\u2027\\U000e0067\\u0a47\\u00b7",
    "\\u1fcd\\u002c\\u07aa\\u0027\\u11b0",
    "\\u002c\\U000e003c\\U0001d7f4\\u003a\\u0c6f\\u0027",
    "\\u0589\\U000e006e\\u0a42\\U000104a5",
    "\\u0f66\\u2523\\u003a\\u0cae\\U000e0047\\u003a",
    "\\u003a\\u0f21\\u0668\\u0dab\\u003a\\u0655\\u00b7",
    "\\u0027\\u11af\\U000e0057\\u0602",
    "\\U0001d7f2\\U000e007\\u0004\\u0589",
    "\\U000e0022\\u003a\\u10b3\\u003a\\ua21b\\u002e\\U000e0058\\u1732\\U000e002b",
    "\\U0001d7f2\\U000e007d\\u0004\\u0589",
    "\\u0233\\U000e0020\\u0a69\\u0d6a",
    "\\u18f4\\U000e0049\\u20e7\\u2027",
    "\\u17e8\\u06e7\\u002e\\u096d\\u003b",
    "\\u003a\\u0e57\\u0fad\\u002e",
    "\\U000e005d\\u2044\\u0731\\u0650\\u0061",
    "\\u003a\\u0664\\u00b7\\u1fba",
    };
    int loop;
    if (U_FAILURE(status)) {
        errcheckln(status, "Creation of break iterator failed %s", u_errorName(status));
        return;
    }
    for (loop = 0; loop < UPRV_LENGTHOF(strlist); loop ++) {
        // printf("looping %d\n", loop);
        UnicodeString ustr = CharsToUnicodeString(strlist[loop]);
        // RBBICharMonkey monkey;
        RBBIWordMonkey monkey;

        int expected[50];
        int expectedcount = 0;

        monkey.setText(ustr);
        int i;
        for (i = 0; i != BreakIterator::DONE; i = monkey.next(i)) {
            expected[expectedcount ++] = i;
        }

        testBreakBoundPreceding(this, ustr, bi, expected, expectedcount);
    }
    delete bi;
#endif
}

void RBBITest::TestWordBoundary()
{
    // <data><>\u1d4a\u206e<?>\u0603\U0001d7ff<>\u2019<></data>
    Locale        locale("en");
    UErrorCode    status = U_ZERO_ERROR;
    // BreakIterator  *bi = BreakIterator::createCharacterInstance(locale, status);
    LocalPointer<BreakIterator> bi(BreakIterator::createWordInstance(locale, status), status);
    if (U_FAILURE(status)) {
        errcheckln(status, "%s:%d Creation of break iterator failed %s",
                __FILE__, __LINE__, u_errorName(status));
        return;
    }
    char16_t      str[50];
    static const char *strlist[] =
    {
    "\\u200e\\U000e0072\\u0a4b\\U000e003f\\ufd2b\\u2027\\u002e\\u002e",
    "\\U000e0042\\u002e\\u0fb8\\u09ef\\u0ed1\\u2044",
    "\\u003b\\u024a\\u102e\\U000e0071\\u0600",
    "\\u2027\\U000e0067\\u0a47\\u00b7",
    "\\u1fcd\\u002c\\u07aa\\u0027\\u11b0",
    "\\u002c\\U000e003c\\U0001d7f4\\u003a\\u0c6f\\u0027",
    "\\u0589\\U000e006e\\u0a42\\U000104a5",
    "\\u4f66\\ub523\\u003a\\uacae\\U000e0047\\u003a",
    "\\u003a\\u0f21\\u0668\\u0dab\\u003a\\u0655\\u00b7",
    "\\u0027\\u11af\\U000e0057\\u0602",
    "\\U0001d7f2\\U000e007\\u0004\\u0589",
    "\\U000e0022\\u003a\\u10b3\\u003a\\ua21b\\u002e\\U000e0058\\u1732\\U000e002b",
    "\\U0001d7f2\\U000e007d\\u0004\\u0589",
    "\\u82ab\\u17e8\\u0736\\u2019\\U0001d64d",
    "\\u0e01\\ub55c\\u0a68\\U000e0037\\u0cd6\\u002c\\ub959",
    "\\U000e0065\\u302c\\u09ee\\U000e0068",
    "\\u0be8\\u002e\\u0c68\\u066e\\u136d\\ufc99\\u59e7",
    "\\u0233\\U000e0020\\u0a69\\u0d6a",
    "\\u206f\\u0741\\ub3ab\\u2019\\ubcac\\u2019",
    "\\u58f4\\U000e0049\\u20e7\\u2027",
    "\\U0001d7e5\\U000e0073\\u0c47\\u06f2\\u0c6a\\u0037\\u10fe",
    "\\ua183\\u102d\\u0bec\\u003a",
    "\\u17e8\\u06e7\\u002e\\u096d\\u003b",
    "\\u003a\\u0e57\\u0fad\\u002e",
    "\\u002e\\U000e004c\\U0001d7ea\\u05bb\\ud0fd\\u02de",
    "\\u32e6\\U0001d7f6\\u0fa1\\u206a\\U000e003c\\u0cec\\u003a",
    "\\ua2a5\\u0038\\u2044\\u002e\\u0c67\\U000e003c\\u05f4\\u2027\\u05f4\\u2019",
    "\\u003a\\u0664\\u00b7\\u1fba",
    "\\u003b\\u0027\\u00b7\\u47a3",
    };
    int loop;
    for (loop = 0; loop < UPRV_LENGTHOF(strlist); loop ++) {
        u_unescape(strlist[loop], str, UPRV_LENGTHOF(str));
        UnicodeString ustr(str);
        int forward[50];
        int count = 0;

        bi->setText(ustr);
        int prev = -1;
        for (int32_t boundary = bi->first(); boundary != BreakIterator::DONE; boundary = bi->next()) {
            ++count;
            if (count >= UPRV_LENGTHOF(forward)) {
                errln("%s:%d too many breaks found. (loop, count, boundary) = (%d, %d, %d)",
                        __FILE__, __LINE__, loop, count, boundary);
                return;
            }
            forward[count] = boundary;
            if (boundary <= prev) {
                errln("%s:%d bi::next() did not advance. (loop, prev, boundary) = (%d, %d, %d)\n",
                        __FILE__, __LINE__, loop, prev, boundary);
                break;
            }
            for (int32_t nonBoundary = prev + 1; nonBoundary < boundary; nonBoundary ++) {
                if (bi->isBoundary(nonBoundary)) {
                    printStringBreaks(ustr, forward, count);
                    errln("%s:%d isBoundary(nonBoundary) failed. (loop, prev, nonBoundary, boundary) = (%d, %d, %d, %d)",
                           __FILE__, __LINE__, loop, prev, nonBoundary, boundary);
                    return;
                }
            }
            if (!bi->isBoundary(boundary)) {
                printStringBreaks(ustr, forward, count);
                errln("%s:%d happy boundary test failed: expected %d a boundary",
                       __FILE__, __LINE__, boundary);
                return;
            }
            prev = boundary;
        }
    }
}

void RBBITest::TestLineBreaks()
{
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
    Locale        locale("en");
    UErrorCode    status = U_ZERO_ERROR;
    BreakIterator *bi = BreakIterator::createLineInstance(locale, status);
    const int32_t  STRSIZE = 50;
    char16_t      str[STRSIZE];
    static const char *strlist[] =
    {
     "\\u300f\\ufdfc\\ub798\\u2011\\u2011\\u0020\\u0b43\\u002d\\ubeec\\ufffc",
     "\\u24ba\\u2060\\u3405\\ub290\\u000d\\U000e0032\\ufe35\\u00a0\\u0361\\"
             "U000112ed\\u0f0c\\u000a\\u308e\\ua875\\u0085\\u114d",
     "\\ufffc\\u3063\\u2e08\\u30e3\\u000d\\u002d\\u0ed8\\u002f\\U00011a57\\"
             "u2014\\U000e0105\\u118c\\u000a\\u07f8",
     "\\u0668\\u192b\\u002f\\u2034\\ufe39\\u00b4\\u0cc8\\u2571\\u200b\\u003f",
     "\\ufeff\\ufffc\\u3289\\u0085\\u2772\\u0020\\U000e010a\\u0020\\u2025\\u000a\\U000e0123",
     "\\ufe3c\\u201c\\u000d\\u2025\\u2007\\u201c\\u002d\\u20a0\\u002d\\u30a7\\u17a4",
     "\\u2772\\u0020\\U000e010a\\u0020\\u2025\\u000a\\U000e0123",
     "\\u002d\\uff1b\\u02c8\\u2029\\ufeff\\u0f22\\u2044\\ufe09\\u003a\\u096d\\u2009\\u000a\\u06f7\\u02cc\\u1019\\u2060",
     "\\u2770\\u0020\\U000e010f\\u0020\\u2060\\u000a\\u02cc\\u0bcc\\u060d\\u30e7\\u0f3b\\u002f",
     "\\ufeff\\u0028\\u003b\\U00012fec\\u2010\\u0020\\u0004\\u200b\\u0020\\u275c\\u002f\\u17b1",
     "\\u20a9\\u2014\\u00a2\\u31f1\\u002f\\u0020\\u05b8\\u200b\\u0cc2\\u003b\\u060d\\u02c8\\ua4e8\\u002f\\u17d5",
     "\\u002d\\u136f\\uff63\\u0084\\ua933\\u2028\\u002d\\u431b\\u200b\\u20b0",
     "\\uade3\\u11d6\\u000a\\U0001107d\\u203a\\u201d\\ub070\\u000d\\u2024\\ufffc",
     "\\uff5b\\u101c\\u1806\\u002f\\u2213\\uff5f",
     "\\u2014\\u0a83\\ufdfc\\u003f\\u00a0\\u0020\\u000a\\u2991\\U0001d179\\u0020\\u201d\\U000125f6\\u0a67\\u20a7\\ufeff\\u043f",
     "\\u169b\\U000e0130\\u002d\\u1041\\u0f3d\\u0abf\\u00b0\\u31fb\\u00a0\\u002d\\u02c8\\u003b",
     "\\u2762\\u1680\\u002d\\u2028\\u0027\\u01dc\\ufe56\\u003a\\u000a\\uffe6\\u29fd\\u0020\\u30ee\\u007c\\U0001d178\\u0af1\\u0085",
     "\\u3010\\u200b\\u2029\\ufeff\\ufe6a\\u275b\\U000e013b\\ufe37\\u24d4\\u002d\\u1806\\u256a\\u1806\\u247c\\u0085\\u17ac",
     "\\u99ab\\u0027\\u003b\\u2026\\ueaf0\\u0020\\u0020\\u0313\\u0020\\u3099\\uff09\\u208e\\u2011\\u2007\\u2060\\u000a\\u0020\\u0020\\u300b\\u0bf9",
     "\\u1806\\u060d\\u30f5\\u00b4\\u17e9\\u2544\\u2028\\u2024\\u2011\\u20a3\\u002d\\u09cc\\u1782\\u000d\\uff6f\\u0025",
     "\\u002f\\uf22e\\u1944\\ufe3d\\u0020\\u206f\\u31b3\\u2014\\u002d\\u2025\\u0f0c\\u0085\\u2763",
     "\\u002f\\u2563\\u202f\\u0085\\u17d5\\u200b\\u0020\\U000e0043\\u2014\\u058a\\u3d0a\\ufe57\\u2035\\u2028\\u2029",
     "\\u20ae\\U0001d169\\u9293\\uff1f\\uff1f\\u0021\\u2012\\u2039\\u0085\\u02cc\\u00a2\\u0020\\U000e01ab\\u3085\\u0f3a\\u1806\\u0f0c\\u1945\\u000a\\U0001d7e7",
     "\\u02cc\\ufe6a\\u00a0\\u0021\\u002d\\u7490\\uec2e\\u200b\\u000a",
     "\\uec2e\\u200b\\u000a\\u0020\\u2028\\u2014\\u8945",
     "\\u7490\\uec2e\\u200b\\u000a\\u0020\\u2028\\u2014",
     "\\u0020\\u2028\\u2014\\u8945\\u002c\\u005b",
     "\\u000a\\ufe3c\\u201c\\u000d\\u2025\\u2007\\u201c\\u002d\\u20a0",
     "\\U0001d16e\\ufffc\\u2025\\u0021\\u002d",
     "\\ufffc\\u301b\\u0fa5\\U000e0103\\u2060\\u208e\\u17d5\\u034f\\u1009\\u003a\\u180e\\u2009\\u3111",
     "\\ufffc\\u0020\\u2116\\uff6c\\u200b\\u0ac3\\U0001028f",
     "\\uaeb0\\u0344\\u0085\\ufffc\\u073b\\u2010",
     "\\ufeff\\u0589\\u0085\\u0eb8\\u30fd\\u002f\\u003a\\u2014\\ufe43",
     "\\u09cc\\u256a\\u276d\\u002d\\u3085\\u000d\\u0e05\\u2028\\u0fbb",
     "\\u2034\\u00bb\\u0ae6\\u300c\\u0020\\u31f8\\ufffc",
     "\\u2116\\u0ed2\\uff64\\u02cd\\u2001\\u2060",
     "\\ufe10\\u2060\\u1a5a\\u2060\\u17e4\\ufffc\\ubbe1\\ufe15\\u0020\\u00a0",
         "\\u2060\\u2213\\u200b\\u2019\\uc2dc\\uff6a\\u1736\\u0085\\udb07",
    };
    int loop;
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }
    for (loop = 0; loop < UPRV_LENGTHOF(strlist); loop ++) {
        // printf("looping %d\n", loop);
        int32_t t = u_unescape(strlist[loop], str, STRSIZE);
        if (t >= STRSIZE) {
            TEST_ASSERT(false);
            continue;
        }


        UnicodeString ustr(str);
        RBBILineMonkey monkey;
        if (U_FAILURE(monkey.deferredStatus)) {
            continue;
        }

        const int EXPECTEDSIZE = 50;
        int expected[EXPECTEDSIZE];
        int expectedcount = 0;

        monkey.setText(ustr);

        int i;
        for (i = 0; i != BreakIterator::DONE; i = monkey.next(i)) {
            if (expectedcount >= EXPECTEDSIZE) {
                TEST_ASSERT(expectedcount < EXPECTEDSIZE);
                return;
            }
            expected[expectedcount ++] = i;
        }

        testBreakBoundPreceding(this, ustr, bi, expected, expectedcount);
    }
    delete bi;
#endif
}

void RBBITest::TestSentBreaks()
{
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
    Locale        locale("en");
    UErrorCode    status = U_ZERO_ERROR;
    BreakIterator *bi = BreakIterator::createSentenceInstance(locale, status);
    char16_t      str[200];
    static const char *strlist[] =
    {
     "Now\ris\nthe\r\ntime\n\rfor\r\r",
     "This\n",
     "Hello! how are you? I'am fine. Thankyou. How are you doing? This\n costs $20,00,000.",
     "\"Sentence ending with a quote.\" Bye.",
     "  (This is it).  Testing the sentence iterator. \"This isn't it.\"",
     "Hi! This is a simple sample sentence. (This is it.) This is a simple sample sentence. \"This isn't it.\"",
     "Hi! This is a simple sample sentence. It does not have to make any sense as you can see. ",
     "Nel mezzo del cammin di nostra vita, mi ritrovai in una selva oscura. ",
     "Che la dritta via aveo smarrita. He said, that I said, that you said!! ",
     "Don't rock the boat.\\u2029Because I am the daddy, that is why. Not on my time (el timo.)!",
     "\\U0001040a\\u203a\\u1217\\u2b23\\u000d\\uff3b\\u03dd\\uff57\\u0a69\\u104a\\ufe56\\ufe52"
             "\\u3016\\U000e002f\\U000e0077\\u0662\\u1680\\u2984\\U000e006a\\u002e\\ua6ab\\u104a"
             "\\u002e\\u019b\\u2005\\u002e\\u0477\\u0438\\u0085\\u0441\\u002e\\u5f61\\u202f"
             "\\U0001019f\\uff08\\u27e8\\u055c\\u0352",
     "\\u1f3e\\u004d\\u000a\\ua3e4\\U000e0023\\uff63\\u0c52\\u276d\\U0001d5de\\U0001d171"
             "\\u0e38\\u17e5\\U00012fe6\\u0fa9\\u267f\\u1da3\\u0046\\u03ed\\udc72\\u0030"
             "\\U0001d688\\u0b6d\\u0085\\u0c67\\u1f94\\u0c6c\\u9cb2\\u202a\\u180e\\u000b"
             "\\u002e\\U000e005e\\u035b\\u061f\\u02c1\\U000e0025\\u0357\\u0969\\u202b"
             "\\U000130c5\\u0486\\U000e0123\\u2019\\u01bc\\u2006\\u11ad\\u180e\\u2e05"
             "\\u10b7\\u013e\\u000a\\u002e\\U00013ea4"
    };
    int loop;
    if (U_FAILURE(status)) {
        errcheckln(status, "Creation of break iterator failed %s", u_errorName(status));
        return;
    }
    for (loop = 0; loop < UPRV_LENGTHOF(strlist); loop ++) {
        u_unescape(strlist[loop], str, UPRV_LENGTHOF(str));
        UnicodeString ustr(str);

        RBBISentMonkey monkey;
        if (U_FAILURE(monkey.deferredStatus)) {
            continue;
        }

        const int EXPECTEDSIZE = 50;
        int expected[EXPECTEDSIZE];
        int expectedcount = 0;

        monkey.setText(ustr);

        int i;
        for (i = 0; i != BreakIterator::DONE; i = monkey.next(i)) {
            if (expectedcount >= EXPECTEDSIZE) {
                TEST_ASSERT(expectedcount < EXPECTEDSIZE);
                return;
            }
            expected[expectedcount ++] = i;
        }

        testBreakBoundPreceding(this, ustr, bi, expected, expectedcount);
    }
    delete bi;
#endif
}

void RBBITest::TestMonkey() {
#if !UCONFIG_NO_REGULAR_EXPRESSIONS

    UErrorCode     status    = U_ZERO_ERROR;
    int64_t        loopCount = 500;
    uint64_t       seed = defaultSeed;
    UnicodeString  breakType = "all";
    Locale         locale("en");
    UBool          useUText  = false;
    UBool          scalarsOnly = false;
    std::string    exportPath;
    std::string    engineState;

    if (quick == false) {
        loopCount = 10000;
    }

    if (fTestParams) {
        UnicodeString p(fTestParams);
        loopCount = getIntParam("loop", p, loopCount);
        seed = getIntParam("seed", p, defaultSeed);

        RegexMatcher engineStateMatcher(R"( *engineState *=\[*([0-9 ]+)\] *)", p, 0, status);
        if (engineStateMatcher.find()) {
            engineStateMatcher.group(1, status).toUTF8String(engineState);
            engineStateMatcher.reset();
            p = engineStateMatcher.replaceFirst("", status);
        }

        RegexMatcher m(" *type *= *(char|word|line|sent|title) *", p, 0, status);
        if (m.find()) {
            breakType = m.group(1, status);
            m.reset();
            p = m.replaceFirst("", status);
        }

        RegexMatcher u(" *utext", p, 0, status);
        if (u.find()) {
            useUText = true;
            u.reset();
            p = u.replaceFirst("", status);
        }

        RegexMatcher pathMatcher(" *export *= *([^ ]+) *", p, 0, status);
        if (pathMatcher.find()) {
            pathMatcher.group(1, status).toUTF8String(exportPath);
            pathMatcher.reset();
            p = pathMatcher.replaceFirst("", status);
        }

        RegexMatcher s(" *scalars_only", p, 0, status);
        if (s.find()) {
            scalarsOnly = true;
            s.reset();
            p = s.replaceFirst("", status);
        }

        // m.reset(p);
        if (RegexMatcher(UNICODE_STRING_SIMPLE("\\S"), p, 0, status).find()) {
            // Each option is stripped out of the option string as it is processed.
            // All options have been checked.  The option string should have been completely emptied..
            char buf[100];
            p.extract(buf, sizeof(buf), nullptr, status);
            buf[sizeof(buf)-1] = 0;
            errln("Unrecognized or extra parameter:  %s\n", buf);
            return;
        }

    }
    if (seed != defaultSeed && !engineState.empty()) {
        errln("seed and engineState parameters are mutually exclusive\n");
        return;
    }
    if (engineState.empty()) {
        engineState = serialize(RandomNumberGenerator(seed));
    }

    if (breakType == "char" || breakType == "all") {
        FILE *file = exportPath.empty() ? nullptr : fopen((exportPath + "_char.txt").c_str(), "w");
        RBBICharMonkey  m;
        BreakIterator  *bi = BreakIterator::createCharacterInstance(locale, status);
        if (U_SUCCESS(status)) {
            RunMonkey(bi, m, "char", engineState, loopCount, useUText, file, scalarsOnly);
            if (breakType == "all" && useUText==false) {
                // Also run a quick test with UText when "all" is specified
                RunMonkey(bi, m, "char", engineState, loopCount, true, nullptr, scalarsOnly);
            }
        }
        else {
            errcheckln(status, "Creation of character break iterator failed %s", u_errorName(status));
        }
        delete bi;
        if (file != nullptr) {
            fclose(file);
        }
    }

    if (breakType == "word" || breakType == "all") {
        logln("Word Break Monkey Test");
        FILE *file = exportPath.empty() ? nullptr : fopen((exportPath + "_word.txt").c_str(), "w");
        RBBIWordMonkey  m;
        BreakIterator  *bi = BreakIterator::createWordInstance(locale, status);
        if (U_SUCCESS(status)) {
            RunMonkey(bi, m, "word", engineState, loopCount, useUText, file, scalarsOnly);
        }
        else {
            errcheckln(status, "Creation of word break iterator failed %s", u_errorName(status));
        }
        delete bi;
        if (file != nullptr) {
            fclose(file);
        }
    }

    if (breakType == "line" || breakType == "all") {
        logln("Line Break Monkey Test");
        FILE *file = exportPath.empty() ? nullptr : fopen((exportPath + "_line.txt").c_str(), "w");
        RBBILineMonkey  m;
        BreakIterator  *bi = BreakIterator::createLineInstance(locale, status);
        if (loopCount >= 10) {
            loopCount = loopCount / 5;   // Line break runs slower than the others.
        }
        if (U_SUCCESS(status)) {
            RunMonkey(bi, m, "line", engineState, loopCount, useUText, file, scalarsOnly);
        }
        else {
            errcheckln(status, "Creation of line break iterator failed %s", u_errorName(status));
        }
        delete bi;
        if (file != nullptr) {
            fclose(file);
        }
    }

    if (breakType == "sent" || breakType == "all"  ) {
        logln("Sentence Break Monkey Test");
        FILE *file = exportPath.empty() ? nullptr : fopen((exportPath + "_sent.txt").c_str(), "w");
        RBBISentMonkey  m;
        BreakIterator  *bi = BreakIterator::createSentenceInstance(locale, status);
        if (loopCount >= 10) {
            loopCount = loopCount / 10;   // Sentence runs slower than the other break types
        }
        if (U_SUCCESS(status)) {
            RunMonkey(bi, m, "sent", engineState, loopCount, useUText, file, scalarsOnly);
        }
        else {
            errcheckln(status, "Creation of line break iterator failed %s", u_errorName(status));
        }
        delete bi;
        if (file != nullptr) {
            fclose(file);
        }
    }

#endif
}

//
//  Run a RBBI monkey test.  Common routine, for all break iterator types.
//    Parameters:
//       bi          - the break iterator to use
//       mk          - MonkeyKind, abstraction for obtaining expected results
//       name        - Name of test (char, word, etc.) for use in error messages
//       engineState - State for starting random number generator (parameter from user)
//       numIterations
//       exportFile  - Pointer to a file to which the test cases will be written in
//                     UCD format.  May be null.
//       scalarsOnly - Only test sequences of Unicode scalar values; if this is false,
//                     arbitrary sequences of code points (including unpaired surrogates)
//                     are tested.
//
void RBBITest::RunMonkey(BreakIterator *bi, RBBIMonkeyKind &mk, const char *name, std::string engineState,
                         int64_t numIterations, UBool useUText, FILE *exportFile, UBool scalarsOnly) {

#if !UCONFIG_NO_REGULAR_EXPRESSIONS

    const int32_t    TESTSTRINGLEN = 500;
    UnicodeString    testText;
    int32_t          numCharClasses;
    int              expectedCount = 0;
    char             expectedBreaks[TESTSTRINGLEN*2 + 1];
    char             forwardBreaks[TESTSTRINGLEN*2 + 1];
    char             reverseBreaks[TESTSTRINGLEN*2+1];
    char             isBoundaryBreaks[TESTSTRINGLEN*2+1];
    char             followingBreaks[TESTSTRINGLEN*2+1];
    char             precedingBreaks[TESTSTRINGLEN*2+1];
    int              i;
    int64_t          loopCount = 0;

    if (engineState.empty()) {
        randomNumberGenerator = {};
    } else {
      randomNumberGenerator = deserialize(engineState);
    }

    numCharClasses = mk.charClasses().size();
    const std::vector<UnicodeSet>& chClasses = mk.charClasses();

    // Check for errors that occurred during the construction of the MonkeyKind object.
    //  Can't report them where they occurred because errln() is a method coming from intlTest,
    //  and is not visible outside of RBBITest :-(
    if (U_FAILURE(mk.deferredStatus)) {
        errln("status of \"%s\" in creation of RBBIMonkeyKind.", u_errorName(mk.deferredStatus));
        return;
    }

    // Verify that the character classes all have at least one member.
    for (i=0; i<numCharClasses; i++) {
        const UnicodeSet& s = chClasses[i];
        if (s.size() == 0) {
            errln("Character Class #%d is null or of zero size. (Have you updated the generated "
                  "monkey test rules and partition?)",
                  i);
            return;
        }
    }

    // For minimizing width of class name output.
    int classNameSize = mk.maxClassNameSize();

    while (loopCount < numIterations || numIterations == -1) {
        if (numIterations == -1 && loopCount % 10 == 0) {
            // If test is running in an infinite loop, display a periodic tic so
            //   we can tell that it is making progress.
            constexpr std::array<std::string_view, 5> monkeys{"🙈", "🙉", "🙊", "🐵", "🐒"};
            fprintf(stderr, "%s",
                    monkeys[RandomNumberGenerator(randomNumberGenerator)() % monkeys.size()].data());
            if (loopCount % 1'000'000 == 0) {
                fprintf(stderr, "\nTested %" PRId64 " million random strings with %d errors…\n",
                        loopCount / 1'000'000, getErrors());
            }
        }
        // Save current RNG state, so that we can recreate the random numbers
        //   for this loop iteration in event of an error.
        engineState = serialize(randomNumberGenerator);

        // Populate a test string with data.
        testText.truncate(0);
        for (i=0; i<TESTSTRINGLEN; i++) {
            const int32_t aClassNum = std::uniform_int_distribution<>(0, numCharClasses - 1)(randomNumberGenerator);
            const UnicodeSet& classSet = chClasses[aClassNum];
            const int32_t charIdx = std::uniform_int_distribution<>(0, classSet.size() - 1)(randomNumberGenerator);
            UChar32   c = classSet.charAt(charIdx);
            if (c < 0) {   // TODO:  deal with sets containing strings.
                errln("%s:%d c < 0", __FILE__, __LINE__);
                break;
            }
            if (mk.dictionarySet().contains(c)) {
              continue;
            }
            if (scalarsOnly && U16_IS_SURROGATE(c)) {
              continue;
            }
            // Do not assemble a supplementary character from randomly generated separate surrogates.
            //   (It could be a dictionary character)
            if (U16_IS_TRAIL(c) && testText.length() > 0 && U16_IS_LEAD(testText.charAt(testText.length()-1))) {
                continue;
            }

            testText.append(c);
        }

        // Calculate the expected results for this test string and reset applied rules.
        mk.setText(testText);

        memset(expectedBreaks, 0, sizeof(expectedBreaks));
        expectedBreaks[0] = 1;
        int32_t breakPos = 0;
        expectedCount = 0;
        for (;;) {
            breakPos = mk.next(breakPos);
            if (breakPos == -1) {
                break;
            }
            if (breakPos > testText.length()) {
                errln("breakPos > testText.length()");
            }
            expectedBreaks[breakPos] = 1;
            expectedCount++;
            U_ASSERT(expectedCount<testText.length());
	    (void)expectedCount;  // Used by U_ASSERT().
        }

        // Find the break positions using forward iteration
        memset(forwardBreaks, 0, sizeof(forwardBreaks));
        if (useUText) {
            UErrorCode status = U_ZERO_ERROR;
            UText *testUText = utext_openReplaceable(nullptr, &testText, &status);
            // testUText = utext_openUnicodeString(testUText, &testText, &status);
            bi->setText(testUText, status);
            TEST_ASSERT_SUCCESS(status);
            utext_close(testUText);   // The break iterator does a shallow clone of the UText
                                      //  This UText can be closed immediately, so long as the
                                      //  testText string continues to exist.
        } else {
            bi->setText(testText);
        }

        for (i=bi->first(); i != BreakIterator::DONE; i=bi->next()) {
            if (i < 0 || i > testText.length()) {
                errln("%s break monkey test: Out of range value returned by breakIterator::next()", name);
                break;
            }
            forwardBreaks[i] = 1;
        }

        // Find the break positions using reverse iteration
        memset(reverseBreaks, 0, sizeof(reverseBreaks));
        for (i=bi->last(); i != BreakIterator::DONE; i=bi->previous()) {
            if (i < 0 || i > testText.length()) {
                errln("%s break monkey test: Out of range value returned by breakIterator::next()", name);
                break;
            }
            reverseBreaks[i] = 1;
        }

        // Find the break positions using isBoundary() tests.
        memset(isBoundaryBreaks, 0, sizeof(isBoundaryBreaks));
        U_ASSERT((int32_t)sizeof(isBoundaryBreaks) > testText.length());
        for (i=0; i<=testText.length(); i++) {
            isBoundaryBreaks[i] = bi->isBoundary(i);
        }


        // Find the break positions using the following() function.
        // printf(".");
        memset(followingBreaks, 0, sizeof(followingBreaks));
        int32_t   lastBreakPos = 0;
        followingBreaks[0] = 1;
        for (i=0; i<testText.length(); i++) {
            breakPos = bi->following(i);
            if (breakPos <= i ||
                breakPos < lastBreakPos ||
                breakPos > testText.length() ||
                (breakPos > lastBreakPos && lastBreakPos > i)) {
                errln("%s break monkey test: "
                    "Out of range value returned by BreakIterator::following().\n"
                        "Random engineState=[%s]  index=%d; following returned %d;  lastbreak=%d",
                        name, engineState.c_str(), i, breakPos, lastBreakPos);
                break;
            }
            followingBreaks[breakPos] = 1;
            lastBreakPos = breakPos;
        }

        // Find the break positions using the preceding() function.
        memset(precedingBreaks, 0, sizeof(precedingBreaks));
        lastBreakPos = testText.length();
        precedingBreaks[testText.length()] = 1;
        for (i=testText.length(); i>0; i--) {
            breakPos = bi->preceding(i);
            if (breakPos >= i ||
                breakPos > lastBreakPos ||
                (breakPos < 0 && testText.getChar32Start(i)>0) ||
                (breakPos < lastBreakPos && lastBreakPos < testText.getChar32Start(i)) ) {
                errln("%s break monkey test: "
                    "Out of range value returned by BreakIterator::preceding().\n"
                    "index=%d;  prev returned %d; lastBreak=%d" ,
                    name,  i, breakPos, lastBreakPos);
                if (breakPos >= 0 && breakPos < static_cast<int32_t>(sizeof(precedingBreaks))) {
                    precedingBreaks[i] = 2;   // Forces an error.
                }
            } else {
                if (breakPos >= 0) {
                    precedingBreaks[breakPos] = 1;
                }
                lastBreakPos = breakPos;
            }
        }

        if (exportFile != nullptr) {
            for (i = 0; i < testText.length();) {
                fprintf(exportFile, expectedBreaks[i] ? "÷ " : "× ");
                char32_t const c = testText.char32At(i);
                fprintf(exportFile, "%04X ", static_cast<uint32_t>(c));
                i += U16_LENGTH(c);
            }
            fprintf(exportFile, expectedBreaks[testText.length()] ? "÷  # 🐒\n" : "×  # 🐒\n");
        }

        // Compare the expected and actual results.
        for (i=0; i<=testText.length(); i++) {
            const char *errorType = nullptr;
            const char* currentBreakData = nullptr;
            if  (forwardBreaks[i] != expectedBreaks[i]) {
                errorType = "next()";
                currentBreakData = forwardBreaks;
            } else if (reverseBreaks[i] != forwardBreaks[i]) {
                errorType = "previous()";
                currentBreakData = reverseBreaks;
           } else if (isBoundaryBreaks[i] != expectedBreaks[i]) {
                errorType = "isBoundary()";
                currentBreakData = isBoundaryBreaks;
            } else if (followingBreaks[i] != expectedBreaks[i]) {
                errorType = "following()";
                currentBreakData = followingBreaks;
            } else if (precedingBreaks[i] != expectedBreaks[i]) {
                errorType = "preceding()";
                currentBreakData = precedingBreaks;
            }

            if (errorType != nullptr) {
                // Format a range of the test text that includes the failure as
                //  a data item that can be included in the rbbi test data file.

                // Start of the range is the last point where expected and actual results
                //  both agreed that there was a break position.

                int startContext = i;
                int32_t count = 0;
                for (;;) {
                    if (startContext==0) { break; }
                    startContext --;
                    if (expectedBreaks[startContext] != 0) {
                        if (count == 2) break;
                        count ++;
                    }
                }

                // End of range is two expected breaks past the start position.
                int endContext = i + 1;
                int ci;
                for (ci=0; ci<2; ci++) {  // Number of items to include in error text.
                    for (;;) {
                        if (endContext >= testText.length()) {break;}
                        if (expectedBreaks[endContext-1] != 0) {
                            if (count == 0) break;
                            count --;
                        }
                        endContext ++;
                    }
                }

                // Formatting of each line includes:
                //   character code
                //   reference break: '|' -> a break, '.' -> no break
                //   actual break:    '|' -> a break, '.' -> no break
                //   (name of character clase)
                //   Unicode name of character
                //   '-->' indicates location of the difference.

                MONKEY_ERROR(
                    (expectedBreaks[i] ? "Break expected but not found" :
                       "Break found but not expected"),
                    name, i, engineState);

                for (ci = startContext;; (ci = testText.moveIndex32(ci, 1))) {
                    UChar32  c;
                    c = testText.char32At(ci);

                    std::string currentLineFlag = "   ";
                    if (ci == i) {
                        currentLineFlag = "-->";  // Error position
                    }

                    // BMP or SMP character in hex
                    char hexCodePoint[12];
                    std::string format = "    \\u%04x";
                    if (c >= 0x10000) {
                        format = "\\U%08x";
                    }
                    snprintf(hexCodePoint, sizeof(hexCodePoint), format.c_str(), c);

                    // Get the class name and character name for the character.
                    char cName[200];
                    UErrorCode status = U_ZERO_ERROR;
                    u_charName(c, U_EXTENDED_CHAR_NAME, cName, sizeof(cName), &status);

                    char buffer[280];
                    auto ret = snprintf(buffer, sizeof(buffer),
                             "%4s %3i :  %1s  %1s  %10s  %-*s  %-40s  %-40s",
                             currentLineFlag.c_str(),
                             ci,
                             expectedBreaks[ci] == 0 ? "." : "|",  // Reference break
                             currentBreakData[ci] == 0 ? "." : "|",  // Actual break
                             hexCodePoint,
                             classNameSize,
                             mk.classNameFromCodepoint(c).c_str(),
                             mk.getAppliedRule(ci).c_str(), cName);
                    (void)ret;
                    U_ASSERT(0 <= ret && ret < UPRV_LENGTHOF(buffer));

                    // Output the error
                    if (ci == i) {
                        errln(buffer);
                    } else {
                        infoln(buffer);
                    }

                    if (ci >= endContext) { break; }
                }
                break;
            }
        }

        loopCount++;
    }
#endif
}


//  Bug 5532.  UTF-8 based UText fails in dictionary code.
//             This test checks the initial patch,
//             which is to just keep it from crashing.  Correct word boundaries
//             await a proper fix to the dictionary code.
//
void RBBITest::TestBug5532()  {
   // Text includes a mixture of Thai and Latin.
   const unsigned char utf8Data[] = {
           0xE0u, 0xB8u, 0x82u, 0xE0u, 0xB8u, 0xB2u, 0xE0u, 0xB8u, 0xA2u, 0xE0u,
           0xB9u, 0x80u, 0xE0u, 0xB8u, 0x84u, 0xE0u, 0xB8u, 0xA3u, 0xE0u, 0xB8u,
           0xB7u, 0xE0u, 0xB9u, 0x88u, 0xE0u, 0xB8u, 0xADu, 0xE0u, 0xB8u, 0x87u,
           0xE0u, 0xB9u, 0x80u, 0xE0u, 0xB8u, 0xA5u, 0xE0u, 0xB9u, 0x88u, 0xE0u,
           0xB8u, 0x99u, 0xE0u, 0xB8u, 0x8Bu, 0xE0u, 0xB8u, 0xB5u, 0xE0u, 0xB8u,
           0x94u, 0xE0u, 0xB8u, 0xB5u, 0x20u, 0x73u, 0x69u, 0x6Du, 0x20u, 0x61u,
           0x75u, 0x64u, 0x69u, 0x6Fu, 0x2Fu, 0x20u, 0x4Du, 0x4Fu, 0x4Fu, 0x4Eu,
           0x20u, 0x65u, 0x63u, 0x6Cu, 0x69u, 0x70u, 0x73u, 0x65u, 0x20u, 0xE0u,
           0xB8u, 0xA3u, 0xE0u, 0xB8u, 0xB2u, 0xE0u, 0xB8u, 0x84u, 0xE0u, 0xB8u,
           0xB2u, 0x20u, 0x34u, 0x37u, 0x30u, 0x30u, 0x20u, 0xE0u, 0xB8u, 0xA2u,
           0xE0u, 0xB8u, 0xB9u, 0xE0u, 0xB9u, 0x82u, 0xE0u, 0xB8u, 0xA3u, 0x00};

    UErrorCode status = U_ZERO_ERROR;
    UText utext=UTEXT_INITIALIZER;
    utext_openUTF8(&utext, reinterpret_cast<const char*>(utf8Data), -1, &status);
    TEST_ASSERT_SUCCESS(status);

    BreakIterator *bi = BreakIterator::createWordInstance(Locale("th"), status);
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        bi->setText(&utext, status);
        TEST_ASSERT_SUCCESS(status);

        int32_t breakCount = 0;
        int32_t previousBreak = -1;
        for (bi->first(); bi->next() != BreakIterator::DONE; breakCount++) {
            // For now, just make sure that the break iterator doesn't hang.
            TEST_ASSERT(previousBreak < bi->current());
            previousBreak = bi->current();
        }
        TEST_ASSERT(breakCount > 0);
    }
    delete bi;
    utext_close(&utext);
}


void RBBITest::TestBug9983()  {
    UnicodeString text = UnicodeString("\\u002A"  // * Other
                                       "\\uFF65"  //   Other
                                       "\\u309C"  //   Katakana
                                       "\\uFF9F"  //   Extend
                                       "\\uFF65"  //   Other
                                       "\\u0020"  //   Other
                                       "\\u0000").unescape();

    UErrorCode status = U_ZERO_ERROR;
    LocalPointer<RuleBasedBreakIterator> brkiter(dynamic_cast<RuleBasedBreakIterator *>(
        BreakIterator::createWordInstance(Locale::getRoot(), status)));
    TEST_ASSERT_SUCCESS(status);
    LocalPointer<RuleBasedBreakIterator> brkiterPOSIX(dynamic_cast<RuleBasedBreakIterator *>(
        BreakIterator::createWordInstance(Locale::createFromName("en_US_POSIX"), status)));
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }
    int32_t offset, rstatus, iterationCount;

    brkiter->setText(text);
    brkiter->last();
    iterationCount = 0;
    while ( (offset = brkiter->previous()) != UBRK_DONE ) {
        iterationCount++;
        rstatus = brkiter->getRuleStatus();
        (void)rstatus;     // Suppress set but not used warning.
        if (iterationCount >= 10) {
           break;
        }
    }
    TEST_ASSERT(iterationCount == 6);

    brkiterPOSIX->setText(text);
    brkiterPOSIX->last();
    iterationCount = 0;
    while ( (offset = brkiterPOSIX->previous()) != UBRK_DONE ) {
        iterationCount++;
        rstatus = brkiterPOSIX->getRuleStatus();
        (void)rstatus;     // Suppress set but not used warning.
        if (iterationCount >= 10) {
           break;
        }
    }
    TEST_ASSERT(iterationCount == 6);
}

// Bug 7547 - verify that building a break itereator from empty rules produces an error.
//
void RBBITest::TestBug7547() {
    UnicodeString rules;
    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;
    RuleBasedBreakIterator breakIterator(rules, parseError, status);
    if (status != U_BRK_RULE_SYNTAX) {
        errln("%s:%d Expected U_BRK_RULE_SYNTAX, got %s", __FILE__, __LINE__, u_errorName(status));
    }
    if (parseError.line != 1 || parseError.offset != 0) {
        errln("parseError (line, offset) expected (1, 0), got (%d, %d)", parseError.line, parseError.offset);
    }
}


void RBBITest::TestBug12797() {
    UnicodeString rules = "!!chain; !!forward; $v=b c; a b; $v; !!reverse; .*;";
    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;
    RuleBasedBreakIterator bi(rules, parseError, status);
    if (U_FAILURE(status)) {
        errln("%s:%s status = %s", __FILE__, __LINE__, u_errorName(status));
        return;
    }
    UnicodeString text = "abc";
    bi.setText(text);
    bi.first();
    int32_t boundary = bi.next();
    if (boundary != 3) {
        errln("%s:%d expected boundary==3, got %d", __FILE__, __LINE__, boundary);
    }
}

void RBBITest::TestBug12918() {
    // This test triggers an assertion failure in dictbe.cpp
    const char16_t *crasherString = u"\u3325\u4a16";
    UErrorCode status = U_ZERO_ERROR;
    UBreakIterator* iter = ubrk_open(UBRK_WORD, nullptr, crasherString, -1, &status);
    if (U_FAILURE(status)) {
        dataerrln("%s:%d status = %s", __FILE__, __LINE__, u_errorName(status));
        return;
    }
    ubrk_first(iter);
    int32_t pos = 0;
    int32_t lastPos = -1;
    while((pos = ubrk_next(iter)) != UBRK_DONE) {
        if (pos <= lastPos) {
            errln("%s:%d (pos, lastPos) = (%d, %d)", __FILE__, __LINE__, pos, lastPos);
            break;
        }
    }
    ubrk_close(iter);
}

void RBBITest::TestBug12932() {
    // Node Stack overflow in the RBBI rule parser caused a seg fault.
    UnicodeString ruleStr(
            "((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
            "((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
            "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((()))"
            ")))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
            ")))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
            ")))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))");

    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;
    RuleBasedBreakIterator rbbi(ruleStr, parseError, status);
    if (status != U_BRK_RULE_SYNTAX) {
        errln("%s:%d expected U_BRK_RULE_SYNTAX, got %s",
                __FILE__, __LINE__, u_errorName(status));
    }
}


// Emoji Test. Verify that the sequences defined in the Unicode data file emoji-test.txt
//             remain undevided by ICU char, word and line break.
void RBBITest::TestEmoji() {
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
    UErrorCode  status = U_ZERO_ERROR;

    CharString testFileName;
    testFileName.append(IntlTest::getSourceTestData(status), status);
    testFileName.appendPathPart("emoji-test.txt", status);
    if (U_FAILURE(status)) {
        errln("%s:%s %s while opening emoji-test.txt", __FILE__, __LINE__, u_errorName(status));
        return;
    }
    logln("Opening data file %s\n", testFileName.data());

    int    len;
    char16_t *testFile = ReadAndConvertFile(testFileName.data(), len, "UTF-8", status);
    if (U_FAILURE(status) || testFile == nullptr) {
        errln("%s:%s %s while opening emoji-test.txt", __FILE__, __LINE__, u_errorName(status));
        return;
    }
    UnicodeString testFileAsString(testFile, len);
    delete [] testFile;

    RegexMatcher lineMatcher(u"^.*?$", testFileAsString, UREGEX_MULTILINE, status);
    RegexMatcher hexMatcher(u"\\s*([a-f0-9]*)", UREGEX_CASE_INSENSITIVE, status);
    //           hexMatcher group(1) is a hex number, or empty string if no hex number present.
    int32_t lineNumber = 0;

    LocalPointer<BreakIterator> charBreaks(BreakIterator::createCharacterInstance(Locale::getEnglish(), status), status);
    LocalPointer<BreakIterator> wordBreaks(BreakIterator::createWordInstance(Locale::getEnglish(), status), status);
    LocalPointer<BreakIterator> lineBreaks(BreakIterator::createLineInstance(Locale::getEnglish(), status), status);
    if (U_FAILURE(status)) {
        dataerrln("%s:%d %s while opening break iterators", __FILE__, __LINE__, u_errorName(status));
        return;
    }

    while (lineMatcher.find()) {
        ++lineNumber;
        UnicodeString line = lineMatcher.group(status);
        hexMatcher.reset(line);
        UnicodeString testString;   // accumulates the emoji sequence.
        while (hexMatcher.find() && hexMatcher.group(1, status).length() > 0) {
            UnicodeString hex = hexMatcher.group(1, status);
            if (hex.length() > 8) {
                errln("%s:%d emoji-test.txt:%d invalid code point %s", __FILE__, __LINE__, lineNumber, CStr(hex)());
                break;
            }
            CharString hex8;
            hex8.appendInvariantChars(hex, status);
            UChar32 c = static_cast<UChar32>(strtol(hex8.data(), nullptr, 16));
            if (c<=0x10ffff) {
                testString.append(c);
            } else {
                errln("%s:%d emoji-test.txt:%d Error: Unicode Character %s value out of range.",
                        __FILE__, __LINE__, lineNumber, hex8.data());
                break;
            }
        }

        if (testString.length() > 1) {
            charBreaks->setText(testString);
            charBreaks->first();
            int32_t firstBreak = charBreaks->next();
            if (testString.length() != firstBreak) {
                errln("%s:%d  emoji-test.txt:%d Error, uexpected break at offset %d",
                        __FILE__, __LINE__, lineNumber, firstBreak);
            }
            wordBreaks->setText(testString);
            wordBreaks->first();
            firstBreak = wordBreaks->next();
            if (testString.length() != firstBreak) {
                errln("%s:%d  emoji-test.txt:%d Error, uexpected break at offset %d",
                        __FILE__, __LINE__, lineNumber, firstBreak);
            }
            lineBreaks->setText(testString);
            lineBreaks->first();
            firstBreak = lineBreaks->next();
            if (testString.length() != firstBreak) {
                errln("%s:%d  emoji-test.txt:%d Error, uexpected break at offset %d",
                        __FILE__, __LINE__, lineNumber, firstBreak);
            }
        }
    }
#endif
}


// TestBug12519  -  Correct handling of Locales by assignment / copy / clone

void RBBITest::TestBug12519() {
    UErrorCode status = U_ZERO_ERROR;
    LocalPointer<RuleBasedBreakIterator> biEn(dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createWordInstance(Locale::getEnglish(), status)));
    LocalPointer<RuleBasedBreakIterator> biFr(dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createWordInstance(Locale::getFrance(), status)));
    if (!assertSuccess(WHERE, status)) {
        dataerrln("%s %d status = %s", __FILE__, __LINE__, u_errorName(status));
        return;
    }
    assertTrue(WHERE, Locale::getEnglish() == biEn->getLocale(ULOC_VALID_LOCALE, status));

    assertTrue(WHERE, Locale::getFrench() == biFr->getLocale(ULOC_VALID_LOCALE, status));
    assertTrue(WHERE "Locales do not participate in BreakIterator equality.", *biEn == *biFr);

    LocalPointer<RuleBasedBreakIterator>cloneEn(biEn->clone());
    assertTrue(WHERE, *biEn == *cloneEn);
    assertTrue(WHERE, Locale::getEnglish() == cloneEn->getLocale(ULOC_VALID_LOCALE, status));

    LocalPointer<RuleBasedBreakIterator>cloneFr(biFr->clone());
    assertTrue(WHERE, *biFr == *cloneFr);
    assertTrue(WHERE, Locale::getFrench() == cloneFr->getLocale(ULOC_VALID_LOCALE, status));

    LocalPointer<RuleBasedBreakIterator>biDe(dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createLineInstance(Locale::getGerman(), status)));
    UnicodeString text("Hallo Welt");
    biDe->setText(text);
    assertTrue(WHERE "before assignment of \"biDe = biFr\", they should be different, but are equal.", *biFr != *biDe);
    *biDe = *biFr;
    assertTrue(WHERE "after assignment of \"biDe = biFr\", they should be equal, but are not.", *biFr == *biDe);
}

void RBBITest::TestBug12677() {
    // Check that stripping of comments from rules for getRules() is not confused by
    // the presence of '#' characters in the rules that do not introduce comments.
    UnicodeString rules(u"!!forward; \n"
                         "$x = [ab#];  # a set with a # literal. \n"
                         " # .;        # a comment that looks sort of like a rule.   \n"
                         " '#' '?';    # a rule with a quoted #   \n"
                       );

    UErrorCode status = U_ZERO_ERROR;
    UParseError pe;
    RuleBasedBreakIterator bi(rules, pe, status);
    assertSuccess(WHERE, status);
    UnicodeString rtRules = bi.getRules();
    assertEquals(WHERE, UnicodeString(u"!!forward;$x=[ab#];'#''?';"),  rtRules);
}


void RBBITest::TestTableRedundancies() {
    UErrorCode status = U_ZERO_ERROR;

    LocalPointer<RuleBasedBreakIterator> bi (
        dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createLineInstance(Locale::getEnglish(), status)));
    assertSuccess(WHERE, status);
    if (U_FAILURE(status)) return;

    RBBIDataWrapper *dw = bi->fData;
    const RBBIStateTable *fwtbl = dw->fForwardTable;
    UBool in8Bits = fwtbl->fFlags & RBBI_8BITS_ROWS;
    int32_t numCharClasses = dw->fHeader->fCatCount;
    // printf("Char Classes: %d     states: %d\n", numCharClasses, fwtbl->fNumStates);

    // Check for duplicate columns (character categories)

    std::vector<UnicodeString> columns;
    for (int32_t column = 0; column < numCharClasses; column++) {
        UnicodeString s;
        for (int32_t r = 1; r < static_cast<int32_t>(fwtbl->fNumStates); r++) {
            char *rowBytes = const_cast<char*>(fwtbl->fTableData + (fwtbl->fRowLen * r));
            if (in8Bits) {
                RBBIStateTableRow8 *row = reinterpret_cast<RBBIStateTableRow8 *>(rowBytes);
                s.append(row->fNextState[column]);
            } else {
                RBBIStateTableRow16 *row = reinterpret_cast<RBBIStateTableRow16 *>(rowBytes);
                s.append(row->fNextState[column]);
            }
        }
        columns.push_back(s);
    }
    // Ignore column (char class) 0 while checking; it's special, and may have duplicates.
    for (int c1=1; c1<numCharClasses; c1++) {
        int limit = c1 < static_cast<int>(fwtbl->fDictCategoriesStart) ? fwtbl->fDictCategoriesStart : numCharClasses;
        for (int c2 = c1+1; c2 < limit; c2++) {
            if (columns.at(c1) == columns.at(c2)) {
                errln("%s:%d Duplicate columns (%d, %d)\n", __FILE__, __LINE__, c1, c2);
                goto out;
            }
        }
    }
  out:

    // Check for duplicate states
    std::vector<UnicodeString> rows;
    for (int32_t r = 0; r < static_cast<int32_t>(fwtbl->fNumStates); r++) {
        UnicodeString s;
        char *rowBytes = const_cast<char*>(fwtbl->fTableData + (fwtbl->fRowLen * r));
        if (in8Bits) {
            RBBIStateTableRow8 *row = reinterpret_cast<RBBIStateTableRow8 *>(rowBytes);
            s.append(row->fAccepting);
            s.append(row->fLookAhead);
            s.append(row->fTagsIdx);
            for (int32_t column = 0; column < numCharClasses; column++) {
                s.append(row->fNextState[column]);
            }
        } else {
            RBBIStateTableRow16 *row = reinterpret_cast<RBBIStateTableRow16 *>(rowBytes);
            s.append(row->fAccepting);
            s.append(row->fLookAhead);
            s.append(row->fTagsIdx);
            for (int32_t column = 0; column < numCharClasses; column++) {
                s.append(row->fNextState[column]);
            }
        }
        rows.push_back(s);
    }
    for (int r1 = 0; r1 < static_cast<int32_t>(fwtbl->fNumStates); r1++) {
        for (int r2 = r1 + 1; r2 < static_cast<int32_t>(fwtbl->fNumStates); r2++) {
            if (rows.at(r1) == rows.at(r2)) {
                errln("%s:%d Duplicate rows (%d, %d)\n", __FILE__, __LINE__, r1, r2);
                return;
            }
        }
    }
}

// Bug 13447: verify that getRuleStatus() returns the value corresponding to current(),
//            even after next() has returned DONE.

void RBBITest::TestBug13447() {
    UErrorCode status = U_ZERO_ERROR;
    LocalPointer<RuleBasedBreakIterator> bi(
        dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createWordInstance(Locale::getEnglish(), status)));
    assertSuccess(WHERE, status);
    if (U_FAILURE(status)) return;
    UnicodeString data(u"1234");
    bi->setText(data);
    assertEquals(WHERE, UBRK_WORD_NONE, bi->getRuleStatus());
    assertEquals(WHERE, 4, bi->next());
    assertEquals(WHERE, UBRK_WORD_NUMBER, bi->getRuleStatus());
    assertEquals(WHERE, UBRK_DONE, bi->next());
    assertEquals(WHERE, 4, bi->current());
    assertEquals(WHERE, UBRK_WORD_NUMBER, bi->getRuleStatus());
}

//  TestReverse exercises both the synthesized safe reverse rules and the logic
//  for filling the break iterator cache when starting from random positions
//  in the text.
//
//  It's a monkey test, working on random data, with the expected data obtained
//  from forward iteration (no safe rules involved), comparing with results
//  when indexing into the interior of the string (safe rules needed).

void RBBITest::TestReverse() {
    UErrorCode status = U_ZERO_ERROR;

    TestReverse(std::unique_ptr<RuleBasedBreakIterator>(dynamic_cast<RuleBasedBreakIterator*>(
            BreakIterator::createCharacterInstance(Locale::getEnglish(), status))));
    assertSuccess(WHERE, status, true);
    status = U_ZERO_ERROR;
    TestReverse(std::unique_ptr<RuleBasedBreakIterator>(dynamic_cast<RuleBasedBreakIterator*>(
            BreakIterator::createWordInstance(Locale::getEnglish(), status))));
    assertSuccess(WHERE, status, true);
    status = U_ZERO_ERROR;
    TestReverse(std::unique_ptr<RuleBasedBreakIterator>(dynamic_cast<RuleBasedBreakIterator*>(
            BreakIterator::createLineInstance(Locale::getEnglish(), status))));
    assertSuccess(WHERE, status, true);
    status = U_ZERO_ERROR;
    TestReverse(std::unique_ptr<RuleBasedBreakIterator>(dynamic_cast<RuleBasedBreakIterator*>(
            BreakIterator::createSentenceInstance(Locale::getEnglish(), status))));
    assertSuccess(WHERE, status, true);
}

void RBBITest::TestReverse(std::unique_ptr<RuleBasedBreakIterator>bi) {
    if (!bi) {
        return;
    }

    // From the mapping trie in the break iterator's internal data, create a
    // vector of UnicodeStrings, one for each character category, containing
    // all of the code points that map to that category. Unicode planes 0 and 1 only,
    // to avoid an execess of unassigned code points.

    RBBIDataWrapper *data = bi->fData;
    int32_t categoryCount = data->fHeader->fCatCount;
    UCPTrie *trie = data->fTrie;
    bool use8BitsTrie = ucptrie_getValueWidth(trie) == UCPTRIE_VALUE_BITS_8;
    uint32_t dictBit = use8BitsTrie ? 0x0080 : 0x4000;

    std::vector<UnicodeString> strings(categoryCount, UnicodeString());
    for (int cp=0; cp<0x1fff0; ++cp) {
        int cat = ucptrie_get(trie, cp);
        cat &= ~dictBit;    // And off the dictionary bit from the category.
        assertTrue(WHERE, cat < categoryCount && cat >= 0);
        if (cat < 0 || cat >= categoryCount) return;
        strings[cat].append(cp);
    }

    icu_rand randomGen;
    const int testStringLength = 10000;
    UnicodeString testString;

    for (int i=0; i<testStringLength; ++i) {
        int charClass = randomGen() % categoryCount;
        if (strings[charClass].length() > 0) {
            int cp = strings[charClass].char32At(randomGen() % strings[charClass].length());
            testString.append(cp);
        }
    }

    typedef std::pair<UBool, int32_t> Result;
    std::vector<Result> expectedResults;
    bi->setText(testString);
    for (int i=0; i<testString.length(); ++i) {
        bool isboundary = bi->isBoundary(i);
        int  ruleStatus = bi->getRuleStatus();
        expectedResults.emplace_back(isboundary, ruleStatus);
    }

    for (int i=testString.length()-1; i>=0; --i) {
        bi->setText(testString);   // clears the internal break cache
        Result expected = expectedResults[i];
        assertEquals(WHERE, expected.first, bi->isBoundary(i));
        assertEquals(WHERE, expected.second, bi->getRuleStatus());
    }
}


// Ticket 13692 - finding word boundaries in very large numbers or words could
//                be very time consuming. When the problem was present, this void test
//                would run more than fifteen minutes, which is to say, the failure was noticeale.

void RBBITest::TestBug13692() {
    UErrorCode status = U_ZERO_ERROR;
    LocalPointer<RuleBasedBreakIterator> bi (dynamic_cast<RuleBasedBreakIterator*>(
            BreakIterator::createWordInstance(Locale::getEnglish(), status)), status);
    if (!assertSuccess(WHERE, status, true)) {
        return;
    }
    constexpr int32_t LENGTH = 1000000;
    UnicodeString longNumber(LENGTH, static_cast<UChar32>(u'3'), LENGTH);
    for (int i=0; i<20; i+=2) {
        longNumber.setCharAt(i, u' ');
    }
    bi->setText(longNumber);
    assertFalse(WHERE, bi->isBoundary(LENGTH-5));
    assertSuccess(WHERE, status);
}


void RBBITest::TestProperties() {
    UErrorCode errorCode = U_ZERO_ERROR;
    UnicodeSet prependSet(UNICODE_STRING_SIMPLE("[:GCB=Prepend:]"), errorCode);
    if (!prependSet.isEmpty()) {
        errln(
            "[:GCB=Prepend:] is not empty any more. "
            "Uncomment relevant lines in source/data/brkitr/char.txt and "
            "change this test to the opposite condition.");
    }
}


//
//  TestDebug    -  A place-holder test for debugging purposes.
//                  For putting in fragments of other tests that can be invoked
//                  for tracing  without a lot of unwanted extra stuff happening.
//
void RBBITest::TestDebug() {
    UErrorCode status = U_ZERO_ERROR;
    LocalPointer<RuleBasedBreakIterator> bi (dynamic_cast<RuleBasedBreakIterator*>(
            BreakIterator::createCharacterInstance(Locale::getEnglish(), status)), status);
    if (!assertSuccess(WHERE, status, true)) {
        return;
    }
    const UnicodeString &rules = bi->getRules();
    UParseError pe;
    LocalPointer<RuleBasedBreakIterator> newbi(new RuleBasedBreakIterator(rules, pe, status));
    assertSuccess(WHERE, status);
}


//
//  TestDebugRules   A stub test for use in debugging rule compilation problems.
//                   Can be freely altered as needed or convenient.
//                   Leave disabled - #ifdef'ed out - when not activley debugging. The rule source
//                   data files may not be available in all environments.
//                   Any permanent test cases should be moved to rbbitst.txt
//                   (see Bug 20303 in that file, for example), or to another test function in this file.
//
void RBBITest::TestDebugRules() {
#if 0
    const char16_t *rules = u""
        "!!quoted_literals_only; \n"
        "!!chain; \n"
        "!!lookAheadHardBreak; \n"
        " \n"
        // "[a] / ; \n"
        "[a] [b] / [c] [d]; \n"
        "[a] [b] / [c] [d] {100}; \n"
        "[x] [a] [b] / [c] [d] {100}; \n"
        "[a] [b] [c] / [d] {100}; \n"
        //" [c] [d] / [e] [f]; \n"
        //"[a] [b] / [c]; \n"
        ;

    UErrorCode status = U_ZERO_ERROR;
    CharString path(pathToDataDirectory(), status);
    path.appendPathPart("brkitr", status);
    path.appendPathPart("rules", status);
    path.appendPathPart("line.txt", status);
    int    len;
    std::unique_ptr<char16_t []> testFile(ReadAndConvertFile(path.data(), len, "UTF-8", status));
    if (!assertSuccess(WHERE, status)) {
        return;
    }

    UParseError pe;
    // rules = testFile.get();
    RuleBasedBreakIterator *bi = new RuleBasedBreakIterator(rules, pe, status);

    if (!assertSuccess(WHERE, status)) {
        delete bi;
        return;
    }
    // bi->dumpTables();

    delete bi;
#endif
}

void RBBITest::testTrieStateTable(int32_t numChar, bool expectedTrieWidthIn8Bits, bool expectedStateRowIn8Bits) {
    UCPTrieValueWidth expectedTrieWidth = expectedTrieWidthIn8Bits ? UCPTRIE_VALUE_BITS_8 : UCPTRIE_VALUE_BITS_16;
    int32_t expectedStateRowBits = expectedStateRowIn8Bits ? RBBI_8BITS_ROWS : 0;
    // Text are duplicate characters from U+4E00 to U+4FFF
    UnicodeString text;
    for (char16_t c = 0x4e00; c < 0x5000; c++) {
        text.append(c).append(c);
    }
    // Generate rule which will caused length+4 character classes and
    // length+3 states
    UnicodeString rules(u"!!quoted_literals_only;");
    for (char16_t c = 0x4e00; c < 0x4e00 + numChar; c++) {
        rules.append(u'\'').append(c).append(c).append(u"';");
    }
    rules.append(u".;");
    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;
    RuleBasedBreakIterator bi(rules, parseError, status);

    assertEquals(WHERE, numChar + 4, bi.fData->fHeader->fCatCount);
    assertEquals(WHERE, numChar + 3, bi.fData->fForwardTable->fNumStates);
    assertEquals(WHERE, expectedTrieWidth, ucptrie_getValueWidth(bi.fData->fTrie));
    assertEquals(WHERE, expectedStateRowBits, bi.fData->fForwardTable->fFlags & RBBI_8BITS_ROWS);
    assertEquals(WHERE, expectedStateRowBits, bi.fData->fReverseTable->fFlags & RBBI_8BITS_ROWS);

    bi.setText(text);

    int32_t pos;
    int32_t i = 0;
    while ((pos = bi.next()) > 0) {
        // The first numChar should not break between the pair
        if (i++ < numChar) {
            assertEquals(WHERE, i * 2, pos);
        } else {
            // After the first numChar next(), break on each character.
            assertEquals(WHERE, i + numChar, pos);
        }
    }
    while ((pos = bi.previous()) > 0) {
        // The first numChar should not break between the pair
        if (--i < numChar) {
            assertEquals(WHERE, i * 2, pos);
        } else {
            // After the first numChar next(), break on each character.
            assertEquals(WHERE, i + numChar, pos);
        }
    }
}

void RBBITest::Test8BitsTrieWith8BitStateTable() {
    testTrieStateTable(251, true /* expectedTrieWidthIn8Bits */, true /* expectedStateRowIn8Bits */);
}

void RBBITest::Test16BitsTrieWith8BitStateTable() {
    testTrieStateTable(252, false /* expectedTrieWidthIn8Bits */, true /* expectedStateRowIn8Bits */);
}

void RBBITest::Test16BitsTrieWith16BitStateTable() {
    testTrieStateTable(253, false /* expectedTrieWidthIn8Bits */, false /* expectedStateRowIn8Bits */);
}

void RBBITest::Test8BitsTrieWith16BitStateTable() {
    // Test UCPTRIE_VALUE_BITS_8 with 16 bits rows. Use a different approach to
    // create state table in 16 bits.

    // Generate 510 'a' as text
    UnicodeString text;
    for (int32_t i = 0; i < 510; i++) {
        text.append(u'a');
    }

    UnicodeString rules(u"!!quoted_literals_only;'");
    // 254 'a' in the rule will cause 256 states
    for (int32_t i = 0; i < 254; i++) {
        rules.append(u'a');
    }
    rules.append(u"';.;");

    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;
    LocalPointer<RuleBasedBreakIterator> bi(new RuleBasedBreakIterator(rules, parseError, status));

    assertEquals(WHERE, 256, bi->fData->fForwardTable->fNumStates);
    assertEquals(WHERE, UCPTRIE_VALUE_BITS_8, ucptrie_getValueWidth(bi->fData->fTrie));
    assertEquals(WHERE,
                 false, RBBI_8BITS_ROWS == (bi->fData->fForwardTable->fFlags & RBBI_8BITS_ROWS));
    bi->setText(text);

    // break positions:
    // 254, 508, 509, ... 510
    assertEquals("next()", 254, bi->next());
    int32_t i = 0;
    int32_t pos;
    while ((pos = bi->next()) > 0) {
        assertEquals(WHERE, 508 + i , pos);
        i++;
    }
    i = 0;
    while ((pos = bi->previous()) > 0) {
        i++;
        if (pos >= 508) {
            assertEquals(WHERE, 510 - i , pos);
        } else {
            assertEquals(WHERE, 254 , pos);
        }
    }
}

// Test that both compact (8 bit) and full sized (16 bit) rbbi tables work, and
// that there are no problems with rules at the size that transitions between the two.
//
// A rule that matches a literal string, like 'abcdefghij', will require one state and
// one character class per character in the string. So we can make a rule to tickle the
// boundaries by using literal strings of various lengths.
//
// For both the number of states and the number of character classes, the eight bit format
// only has 7 bits available, allowing for 128 values. For both, a few values are reserved,
// leaving 120 something available. This test runs the string over the range of 120 - 130,
// which allows some margin for changes to the number of values reserved by the rule builder
// without breaking the test.

void RBBITest::TestTable_8_16_Bits() {

    // testStr serves as both the source of the rule string (truncated to the desired length)
    // and as test data to check matching behavior. A break rule consisting of the first 120
    // characters of testStr will match the first 120 chars of the full-length testStr.
    UnicodeString testStr;
    for (char16_t c=0x3000; c<0x3200; ++c) {
        testStr.append(c);
    }

    const int32_t startLength = 120;   // The shortest rule string to test.
    const int32_t endLength = 260;     // The longest rule string to test
    const int32_t increment = this->quick ? endLength - startLength : 1;

    for (int32_t ruleLen=startLength; ruleLen <= endLength; ruleLen += increment) {
        UParseError parseError;
        UErrorCode status = U_ZERO_ERROR;

        UnicodeString ruleString{u"!!quoted_literals_only; '#';"};
        ruleString.findAndReplace(UnicodeString(u"#"), UnicodeString(testStr, 0, ruleLen));
        RuleBasedBreakIterator bi(ruleString, parseError, status);
        if (!assertSuccess(WHERE, status)) {
            errln(ruleString);
            break;
        }
        // bi.dumpTables();

        // Verify that the break iterator is functioning - that the first boundary found
        // in testStr is at the length of the rule string.
        bi.setText(testStr);
        assertEquals(WHERE, ruleLen, bi.next());

        // Reverse iteration. Do a setText() first, to flush the break iterator's internal cache
        // of previously detected boundaries, thus forcing the engine to run the safe reverse rules.
        bi.setText(testStr);
        int32_t result = bi.preceding(ruleLen);
        assertEquals(WHERE, 0, result);

        // Verify that the range of rule lengths being tested cover the translations
        // from 8 to 16 bit data.
        bool has8BitRowData = bi.fData->fForwardTable->fFlags & RBBI_8BITS_ROWS;
        bool has8BitsTrie = ucptrie_getValueWidth(bi.fData->fTrie) == UCPTRIE_VALUE_BITS_8;

        if (ruleLen == startLength) {
            assertEquals(WHERE, true, has8BitRowData);
            assertEquals(WHERE, true, has8BitsTrie);
        }
        if (ruleLen == endLength) {
            assertEquals(WHERE, false, has8BitRowData);
            assertEquals(WHERE, false, has8BitsTrie);
        }
    }
}

/* Test handling of a large number of look-ahead rules.
 * The number of rules in the test exceeds the implementation limits prior to the
 * improvements introduced with #13590.
 *
 * The test look-ahead rules have the form "AB / CE"; "CD / EG"; ...
 * The text being matched is sequential, "ABCDEFGHI..."
 *
 * The upshot is that the look-ahead rules all match on their preceding context,
 * and consequently must save a potential result, but then fail to match on their
 * trailing context, so that they don't actually cause a boundary.
 *
 * Additionally, add a ".*" rule, so there are no boundaries unless a
 * look-ahead hard-break rule forces one.
 */
void RBBITest::TestBug13590() {
    UnicodeString rules {u"!!quoted_literals_only; !!chain; .*;\n"};

    const int NUM_LOOKAHEAD_RULES = 50;
    const char16_t STARTING_CHAR = u'\u5000';
    char16_t firstChar;
    for (int ruleNum = 0; ruleNum < NUM_LOOKAHEAD_RULES; ++ruleNum) {
        firstChar = STARTING_CHAR + ruleNum*2;
        rules.append(u'\'') .append(firstChar) .append(firstChar+1) .append(u'\'')
             .append(u' ') .append(u'/') .append(u' ')
             .append(u'\'') .append(firstChar+2) .append(firstChar+4) .append(u'\'')
             .append(u';') .append(u'\n');
    }

    // Change the last rule added from the form "UV / WY" to "UV / WX".
    // Changes the rule so that it will match - all 4 chars are in ascending sequence.
    rules.findAndReplace(UnicodeString(firstChar+4), UnicodeString(firstChar+3));

    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;
    RuleBasedBreakIterator bi(rules, parseError, status);
    if (!assertSuccess(WHERE, status)) {
        errln(rules);
        return;
    }
    // bi.dumpTables();

    UnicodeString testString;
    for (char16_t c = STARTING_CHAR-200; c < STARTING_CHAR + NUM_LOOKAHEAD_RULES*4; ++c) {
        testString.append(c);
    }
    bi.setText(testString);

    int breaksFound = 0;
    while (bi.next() != UBRK_DONE) {
        ++breaksFound;
    }

    // Two matches are expected, one from the last rule that was explicitly modified,
    // and one at the end of the text.
    assertEquals(WHERE, 2, breaksFound);
}


#if U_ENABLE_TRACING
static std::vector<std::string> gData;
static std::vector<int32_t> gEntryFn;
static std::vector<int32_t> gExitFn;
static std::vector<int32_t> gDataFn;

static void U_CALLCONV traceData(
        const void*,
        int32_t fnNumber,
        int32_t,
        const char *,
        va_list args) {
    if (UTRACE_UBRK_START <= fnNumber && fnNumber <= UTRACE_UBRK_LIMIT) {
        const char* data = va_arg(args, const char*);
        gDataFn.push_back(fnNumber);
        gData.push_back(data);
    }
}

static void traceEntry(const void *, int32_t fnNumber) {
    if (UTRACE_UBRK_START <= fnNumber && fnNumber <= UTRACE_UBRK_LIMIT) {
        gEntryFn.push_back(fnNumber);
    }
}

static void traceExit(const void *, int32_t fnNumber, const char *, va_list) {
    if (UTRACE_UBRK_START <= fnNumber && fnNumber <= UTRACE_UBRK_LIMIT) {
        gExitFn.push_back(fnNumber);
    }
}


void RBBITest::assertTestTraceResult(int32_t fnNumber, const char* expectedData) {
    assertEquals("utrace_entry should be called ", 1, gEntryFn.size());
    assertEquals("utrace_entry should be called with ", fnNumber, gEntryFn[0]);
    assertEquals("utrace_exit should be called ", 1, gExitFn.size());
    assertEquals("utrace_exit should be called with ", fnNumber, gExitFn[0]);

    if (expectedData == nullptr) {
      assertEquals("utrace_data should not be called ", 0, gDataFn.size());
      assertEquals("utrace_data should not be called ", 0, gData.size());
    } else {
      assertEquals("utrace_data should be called ", 1, gDataFn.size());
      assertEquals("utrace_data should be called with ", fnNumber, gDataFn[0]);
      assertEquals("utrace_data should be called ", 1, gData.size());
      assertEquals("utrace_data should pass in ", expectedData, gData[0].c_str());
    }
}

void SetupTestTrace() {
    gEntryFn.clear();
    gExitFn.clear();
    gDataFn.clear();
    gData.clear();

    const void* context = nullptr;
    utrace_setFunctions(context, traceEntry, traceExit, traceData);
    utrace_setLevel(UTRACE_INFO);
}

void RBBITest::TestTraceCreateCharacter() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateCharacter");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createCharacterInstance("zh-CN", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_CHARACTER, nullptr);
}

void RBBITest::TestTraceCreateTitle() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateTitle");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createTitleInstance("zh-CN", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_TITLE, nullptr);
}

void RBBITest::TestTraceCreateSentence() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateSentence");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createSentenceInstance("zh-CN", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_SENTENCE, nullptr);
}

void RBBITest::TestTraceCreateWord() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateWord");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createWordInstance("zh-CN", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_WORD, nullptr);
}

void RBBITest::TestTraceCreateLine() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateLine");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createLineInstance("zh-CN", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_LINE, "line");
}

void RBBITest::TestTraceCreateLineStrict() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateLineStrict");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createLineInstance("zh-CN-u-lb-strict", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_LINE, "line_strict");
}

void RBBITest::TestTraceCreateLineNormal() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateLineNormal");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createLineInstance("zh-CN-u-lb-normal", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_LINE, "line_normal");
}

void RBBITest::TestTraceCreateLineLoose() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateLineLoose");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createLineInstance("zh-CN-u-lb-loose", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_LINE, "line_loose");
}

void RBBITest::TestTraceCreateLineLoosePhrase() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateLineLoosePhrase");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createLineInstance("ja-u-lb-loose-lw-phrase", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_LINE, "line_loose_phrase");
}

void RBBITest::TestTraceCreateLineNormalPhrase() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateLineNormalPhrase");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createLineInstance("ja-u-lb-normal-lw-phrase", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_LINE, "line_normal_phrase");
}

void RBBITest::TestTraceCreateLineStrictPhrase() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateLineStrictPhrase");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createLineInstance("ja-u-lb-strict-lw-phrase", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_LINE, "line_strict_phrase");
}

void RBBITest::TestTraceCreateLinePhrase() {
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateLinePhrase");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createLineInstance("ja-u-lw-phrase", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_LINE, "line_phrase");
}

void RBBITest::TestTraceCreateBreakEngine() {
    rbbi_cleanup();
    SetupTestTrace();
    IcuTestErrorCode status(*this, "TestTraceCreateBreakEngine");
    LocalPointer<BreakIterator> brkitr(
        BreakIterator::createWordInstance("zh-CN", status));
    status.errIfFailureAndReset();
    assertTestTraceResult(UTRACE_UBRK_CREATE_WORD, nullptr);

    // To word break the following text, BreakIterator will create 5 dictionary
    // break engine internally.
    UnicodeString text(
        u"test "
        u"測試 " // Hani
        u"សាកល្បង " // Khmr
        u"ທົດສອບ " // Laoo
        u"စမ်းသပ်မှု " // Mymr
        u"ทดสอบ " // Thai
        u"test "
    );
    brkitr->setText(text);

    // Loop through all the text.
    while (brkitr->next() > 0) ;

    assertEquals("utrace_entry should be called ", 6, gEntryFn.size());
    assertEquals("utrace_exit should be called ", 6, gExitFn.size());
    assertEquals("utrace_data should be called ", 5, gDataFn.size());

    for (std::vector<int>::size_type i = 0; i < gDataFn.size(); i++) {
        assertEquals("utrace_entry should be called ",
                     UTRACE_UBRK_CREATE_BREAK_ENGINE, gEntryFn[i+1]);
        assertEquals("utrace_exit should be called ",
                     UTRACE_UBRK_CREATE_BREAK_ENGINE, gExitFn[i+1]);
        assertEquals("utrace_data should be called ",
                     UTRACE_UBRK_CREATE_BREAK_ENGINE, gDataFn[i]);
    }

    assertEquals("utrace_data should pass ", "Hani", gData[0].c_str());
    assertEquals("utrace_data should pass ", "Khmr", gData[1].c_str());
    assertEquals("utrace_data should pass ", "Laoo", gData[2].c_str());
    assertEquals("utrace_data should pass ", "Mymr", gData[3].c_str());
    assertEquals("utrace_data should pass ", "Thai", gData[4].c_str());

}
#endif

void RBBITest::TestUnpairedSurrogate() {
    UnicodeString rules(u"ab;");

    UErrorCode status = U_ZERO_ERROR;
    UParseError pe;
    RuleBasedBreakIterator bi1(rules, pe, status);
    assertSuccess(WHERE, status);
    UnicodeString rtRules = bi1.getRules();
    // make sure the simple one work first.
    assertEquals(WHERE, rules,  rtRules);


    rules = UnicodeString(u"a\\ud800b;").unescape();
    pe.line = 0;
    pe.offset = 0;
    RuleBasedBreakIterator bi2(rules, pe, status);
    assertEquals(WHERE "unpaired lead surrogate", U_ILLEGAL_CHAR_FOUND , status);
    if (pe.line != 1 || pe.offset != 1) {
        errln("pe (line, offset) expected (1, 1), got (%d, %d)", pe.line, pe.offset);
    }

    status = U_ZERO_ERROR;
    rules = UnicodeString(u"a\\ude00b;").unescape();
    pe.line = 0;
    pe.offset = 0;
    RuleBasedBreakIterator bi3(rules, pe, status);
    assertEquals(WHERE "unpaired tail surrogate", U_ILLEGAL_CHAR_FOUND , status);
    if (pe.line != 1 || pe.offset != 1) {
        errln("pe (line, offset) expected (1, 1), got (%d, %d)", pe.line, pe.offset);
    }

    // make sure the surrogate one work too.
    status = U_ZERO_ERROR;
    rules = UnicodeString(u"a😀b;");
    RuleBasedBreakIterator bi4(rules, pe, status);
    rtRules = bi4.getRules();
    assertEquals(WHERE, rules, rtRules);
}

// Read file generated by
// https://github.com/unicode-org/lstm_word_segmentation/blob/master/segment_text.py
// as test cases and compare the Output.
// Format of the file
//   Model:\t[Model Name (such as 'Thai_graphclust_model4_heavy')]
//   Embedding:\t[Embedding type (such as 'grapheme_clusters_tf')]
//   Input:\t[source text]
//   Output:\t[expected output separated by | ]
//   Input: ...
//   Output: ...

void RBBITest::runLSTMTestFromFile(const char* filename, UScriptCode script) {
    // The expectation in this test depends on LSTM, skip the test if the
    // configuration is not build with LSTM data.
    if (skipLSTMTest()) {
        return;
    }
    UErrorCode   status = U_ZERO_ERROR;
    LocalPointer<BreakIterator> iterator(BreakIterator::createWordInstance(Locale(), status));
    if (U_FAILURE(status)) {
        errln("%s:%d Error %s Cannot create Word BreakIterator", __FILE__, __LINE__, u_errorName(status));
        return;
    }
    //  Open and read the test data file.
    const char *testDataDirectory = IntlTest::getSourceTestData(status);
    CharString testFileName(testDataDirectory, -1, status);
    testFileName.append(filename, -1, status);

    int len;
    char16_t *testFile = ReadAndConvertFile(testFileName.data(), len, "UTF-8", status);
    if (U_FAILURE(status)) {
        errln("%s:%d Error %s opening test file %s", __FILE__, __LINE__, u_errorName(status), filename);
        return;
    }

    //  Put the test data into a UnicodeString
    UnicodeString testString(false, testFile, len);

    int32_t start = 0;

    UnicodeString line;
    int32_t end;
    std::string actual_sep_str;
    int32_t caseNum = 0;
    // Iterate through all the lines in the test file.
    do {
        int32_t cr = testString.indexOf(u'\r', start);
        int32_t lf = testString.indexOf(u'\n', start);
        end = cr >= 0 ? (lf >= 0 ? std::min(cr, lf) : cr) : lf;
        line = testString.tempSubString(start, end < 0 ? INT32_MAX : end - start);
        if (line.length() > 0) {
            // Separate each line to key and value by TAB.
            int32_t tab = line.indexOf(u'\t');
            UnicodeString key = line.tempSubString(0, tab);
            const UnicodeString value = line.tempSubString(tab+1);

            if (key == "Model:") {
                // Verify the expectation in the test file match the LSTM model
                // we are using now.
                const LSTMData* data = CreateLSTMDataForScript(script, status);
                if (U_FAILURE(status)) {
                    dataerrln("%s:%d Error %s Cannot create LSTM data for script %s",
                              __FILE__, __LINE__, u_errorName(status), uscript_getName(script));
                    return;
                }
                UnicodeString name(LSTMDataName(data));
                DeleteLSTMData(data);
                if (value != name) {
                    std::string utf8Name, utf8Value;
                    dataerrln("%s:%d Error %s The LSTM data for script %s is %s instead of %s",
                              __FILE__, __LINE__, u_errorName(status), uscript_getName(script),
                              name.toUTF8String<std::string>(utf8Name).c_str(),
                              value.toUTF8String<std::string>(utf8Value).c_str());
                    return;
                }
            } else if (key == "Input:") {
                UnicodeString input("prefix ");
                input += value + " suffix";
                std::stringstream ss;

                // Construct the UText which is expected by the engine as
                // input from the UnicodeString.
                UText ut = UTEXT_INITIALIZER;
                utext_openConstUnicodeString(&ut, &input, &status);
                if (U_FAILURE(status)) {
                    dataerrln("Could not utext_openConstUnicodeString for " + value + UnicodeString(u_errorName(status)));
                    return;
                }

                iterator->setText(&ut, status);
                if (U_FAILURE(status)) {
                    errln("%s:%d Error %s Could not setText to BreakIterator", __FILE__, __LINE__, u_errorName(status));
                    return;
                }

                int32_t bp;
                for (bp = iterator->first(); bp != BreakIterator::DONE; bp = iterator->next()) {
                    ss << bp;
                    if (bp != input.length()) {
                        ss << ", ";
                    }
                }

                utext_close(&ut);
                // Turn the break points into a string for easy comparison
                // output.
                actual_sep_str = "{" + ss.str() + "}";
            } else if (key == "Output:" && !actual_sep_str.empty()) {
                UnicodeString input("prefix| |");
                input += value + "| |suffix";
                std::string d;
                int32_t sep;
                int32_t start = 0;
                int32_t curr = 0;
                std::stringstream ss;
                // Include 0 as the break point.
                ss << "0, ";
                while ((sep = input.indexOf(u'|', start)) >= 0) {
                    int32_t len = sep - start;
                    if (len > 0) {
                        if (curr > 0) {
                            ss << ", ";
                        }
                        curr += len;
                        ss << curr;
                    }
                    start = sep + 1;
                }
                // Include end of the string as break point.
                ss << ", " << curr + input.length() - start;
                // Turn the break points into a string for easy comparison
                // output.
                std::string expected = "{" + ss.str() + "}";
                std::string utf8;

                assertEquals((input + " Test Case#" + caseNum).toUTF8String<std::string>(utf8).c_str(),
                             expected.c_str(), actual_sep_str.c_str());
                actual_sep_str.clear();
            }
        }
        start = std::max(cr, lf) + 1;
    } while (end >= 0);

    delete [] testFile;
}

void RBBITest::TestLSTMThai() {
    runLSTMTestFromFile("Thai_graphclust_model4_heavy_Test.txt", USCRIPT_THAI);
}

void RBBITest::TestLSTMBurmese() {
    runLSTMTestFromFile("Burmese_graphclust_model5_heavy_Test.txt", USCRIPT_MYANMAR);
}


// Test preceding(index) and following(index), with semi-random indexes.
// The random indexes are produced in clusters that are relatively closely spaced,
// to increase the occurrences of hits to the internal break cache.

void RBBITest::TestRandomAccess() {
    static constexpr int32_t CACHE_SIZE = 128;

    UnicodeString testData;
    for (int i=0; i<CACHE_SIZE*2; ++i) {
        testData.append(u"aaaa\n");
    }

    UErrorCode status = U_ZERO_ERROR;
    LocalPointer<RuleBasedBreakIterator> bi(
          dynamic_cast<RuleBasedBreakIterator*>(BreakIterator::createLineInstance(Locale::getEnglish(), status)),
            status);
    if (!assertSuccess(WHERE, status)) { return; };

    bi->setText(testData);

    auto expectedPreceding = [](int from) {
        if (from == 0) {return UBRK_DONE;}
        if (from % 5 == 0) {return from - 5;}
        return from - (from % 5);
    };

    auto expectedFollow = [testData](int from) {
        if (from >= testData.length()) {return UBRK_DONE;}
        if (from % 5 == 0) {return from + 5;}
        return from + (5 - (from % 5));
    };

    auto randomStringIndex = [testData]() {
        static icu_rand randomGenerator;  // produces random uint32_t values.
        static int lastNum;
        static int clusterCount;
        static constexpr int CLUSTER_SIZE = 100;
        static constexpr int CLUSTER_LENGTH = 10;

        if (clusterCount < CLUSTER_LENGTH) {
            ++clusterCount;
            lastNum += (randomGenerator() % CLUSTER_SIZE);
            lastNum -= CLUSTER_SIZE / 2;
            lastNum = std::max(0, lastNum);
            // Deliberately test indexes > testData.length.
            lastNum = std::min(testData.length() + 5, lastNum);
        } else {
            clusterCount = 0;
            lastNum = randomGenerator() % testData.length();
        }
        return lastNum;
    };

    for (int i=0; i<5000; ++i) {
        int idx = randomStringIndex();
        assertEquals(WHERE, expectedFollow(idx), bi->following(idx));
        idx = randomStringIndex();
        assertEquals(WHERE, expectedPreceding(idx), bi->preceding(idx));
    }
}

// A Fake Tai Le break engine which handle Unicode Tai Le (Tale) block
// https://unicode.org/charts/PDF/U1950.pdf
// U+1950 - U+197F and always break after Tone letters (U+1970-U+1974)
class FakeTaiLeBreakEngine : public ExternalBreakEngine {
 public:
  FakeTaiLeBreakEngine() : block(0x1950, 0x197f), tones(0x1970, 0x1974) {
  }
  virtual ~FakeTaiLeBreakEngine() {
  }
  virtual bool isFor(UChar32 c, const char* /* locale */) const override {
      // We implmement this for any locale, not return false for some langauge
      // here.
      return handles(c);
  }
  virtual bool handles(UChar32 c) const override {
      return block.contains(c);
  }
  virtual int32_t fillBreaks(UText* text,  int32_t start, int32_t end,
                             int32_t* foundBreaks, int32_t foundBreaksCapacity,
                             UErrorCode& status) const override {
       if (U_FAILURE(status)) return 0;
       int32_t i = 0;
       // Save the state of the utext
       int64_t savedIndex = utext_getNativeIndex(text);
       if (savedIndex != start) {
           utext_setNativeIndex(text, start);
       }
       int32_t current;
       while ((current = static_cast<int32_t>(utext_getNativeIndex(text))) < end) {
         UChar32 c = utext_current32(text);
         // Break after tone marks as a fake break point.
         if (tones.contains(c)) {
             if (i >= foundBreaksCapacity) {
                 status = U_BUFFER_OVERFLOW_ERROR;
                 utext_setNativeIndex(text, savedIndex);
                 return i;
             }
             foundBreaks[i++] = current;
         }
         UTEXT_NEXT32(text);
       }
       // Restore the utext
       if (savedIndex != current) {
           utext_setNativeIndex(text, savedIndex);
       }
       return i;
  }

 private:
  UnicodeSet block;
  UnicodeSet tones;
};

// A Fake Yue Break Engine which handle CJK Unified Ideographs
// block (U+4E00-U+9FFF) when locale start with 'yue' and break
// after every character.
class FakeYueBreakEngine : public ExternalBreakEngine {
 public:
  FakeYueBreakEngine() : block(0x4e00, 0x9FFF) {
  }
  virtual ~FakeYueBreakEngine() {
  }
  virtual bool isFor(UChar32 c, const char* locale) const override {
      // We implmement this for any locale starts with "yue" such as
      // "yue", "yue-CN", "yue-Hant-CN", etc.
      return handles(c) && uprv_strncmp("yue", locale, 3) == 0;
  }
  virtual bool handles(UChar32 c) const override {
      return block.contains(c);
  }
  virtual int32_t fillBreaks(UText* text,  int32_t start, int32_t end,
                             int32_t* foundBreaks, int32_t foundBreaksCapacity,
                             UErrorCode& status) const override {
       (void)text;
       if (U_FAILURE(status)) return 0;
       int32_t i = 0;
       int32_t current = start;
       while (current++ < end) {
           // A fake word segmentation by breaking every two Unicode.
           if ((current - start) % 2 == 0) {
               if (i >= foundBreaksCapacity) {
                   status = U_BUFFER_OVERFLOW_ERROR;
                   return i;
               }
               foundBreaks[i++] = current;
           }
       }
       return i;
  }

 private:
  UnicodeSet block;
};

void RBBITest::TestExternalBreakEngineWithFakeYue() {
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString text(u"a bc def一兩年前佢真係唔鍾意畀我影相i jk lmn");

    std::vector<int32_t> actual1;
    {
        LocalPointer<BreakIterator> bi1(
            BreakIterator::createWordInstance(Locale::getRoot(), status),
            status);
        bi1->setText(text);
        assertTrue(WHERE "BreakIterator::createWordInstance( root )",
                   U_SUCCESS(status));

        do {
            actual1.push_back(bi1->current());
        } while(bi1->next() != BreakIterator::DONE);
    }

    std::vector<int32_t> expected1{
        0,  1,  2,  4,  5,  8,  10, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 26, 27, 30,
    };
    assertTrue("root break Yue as Chinese", expected1 == actual1);

    status = U_ZERO_ERROR;
    RuleBasedBreakIterator::registerExternalBreakEngine(
        new FakeYueBreakEngine(), status);
    assertTrue(WHERE "registerExternalBreakEngine w FakeYueBreakEngine",
               U_SUCCESS(status));

    std::vector<int32_t> actual2;
    {
        status = U_ZERO_ERROR;
        LocalPointer<BreakIterator> bi2(
            BreakIterator::createWordInstance(Locale("yue"), status), status);
        assertTrue(WHERE "BreakIterator::createWordInstance( yue )",
                   U_SUCCESS(status));
        bi2->setText(text);
        do {
            actual2.push_back(bi2->current());
        } while(bi2->next() != BreakIterator::DONE);
    }

    std::vector<int32_t> expected2{
        0, 1, 2, 4, 5, 8, 10, 12, 14, 16, 18, 20, 22, 23, 24, 26, 27, 30,
    };
    assertTrue(WHERE "break Yue by Fake external breaker",
               expected2 == actual2);
}

void RBBITest::TestExternalBreakEngineWithFakeTaiLe() {
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString text(
        u"a bc defᥛᥫᥒᥰᥖᥭᥰᥞᥝᥰᥙᥥᥢᥛᥫᥒᥰᥑᥩᥢᥲᥔᥣᥝᥴᥓᥬᥖᥩᥢᥲᥛᥣᥝᥱᥙᥝᥱᥙᥤᥱᥓᥣᥒᥛᥣᥰᥓᥧ"
        u"ᥰᥘᥩᥰᥗᥪᥒᥴᥛᥣᥰᥘᥬᥰᥝᥣᥱᥘᥒᥱᥔᥣᥛᥴᥘᥫᥢi jk lmn");

    std::vector<int32_t> actual1;
    {
        LocalPointer<BreakIterator> bi1(
            BreakIterator::createLineInstance(Locale::getRoot(), status),
            status);
        bi1->setText(text);
        assertTrue(WHERE "BreakIterator::createLineInstance( root )",
                   U_SUCCESS(status));

        do {
            actual1.push_back(bi1->current());
        } while(bi1->next() != BreakIterator::DONE);
    }

    std::vector<int32_t> expected1{
        0, 2, 5, 86, 89, 92,
    };
    assertTrue(WHERE "root break Tai Le", expected1 == actual1);

    RuleBasedBreakIterator::registerExternalBreakEngine(
        new FakeTaiLeBreakEngine(), status);
    assertTrue(WHERE "registerExternalBreakEngine w FakeTaiLeBreakEngine",
               U_SUCCESS(status));

    std::vector<int32_t> actual2;
    {
        status = U_ZERO_ERROR;
        LocalPointer<BreakIterator> bi2(
            BreakIterator::createLineInstance(Locale("tdd"), status), status);
        assertTrue(WHERE "BreakIterator::createLineInstance( tdd )",
                   U_SUCCESS(status));
        bi2->setText(text);
        do {
            actual2.push_back(bi2->current());
        } while(bi2->next() != BreakIterator::DONE);
    }

    std::vector<int32_t> expected2{
        0,  2,  5,  11, 14, 17, 24, 28, 32, 38, 42, 45, 48,
        54, 57, 60, 64, 67, 70, 73, 76, 80, 86, 89, 92,
    };
    assertTrue("break Tai Le by Fake external breaker",
               expected2 == actual2);
}

// Test a single unpaired unpaired char (either surrogate low or high) in
// an Unicode set will not cause infinity loop.
void RBBITest::TestBug22585() {
    UnicodeString rule = u"$a=[";
    rule.append(0xdecb) // an unpaired surrogate high
        .append("];");
    UParseError pe {};
    UErrorCode ec {U_ZERO_ERROR};
    RuleBasedBreakIterator bi(rule, pe, ec);

    rule = u"$a=[";
    rule.append(0xd94e) // an unpaired surrogate low
        .append("];");
    ec = U_ZERO_ERROR;
    RuleBasedBreakIterator bi2(rule, pe, ec);
}

// Test a long string with a ; in the end will not cause stack overflow.
void RBBITest::TestBug22602() {
    UnicodeString rule(25000, static_cast<UChar32>('A'), 25000 - 1);
    rule.append(u";");
    UParseError pe {};
    UErrorCode ec {U_ZERO_ERROR};
    RuleBasedBreakIterator bi(rule, pe, ec);
}

void RBBITest::TestBug22636() {
    UParseError pe {};
    UErrorCode ec {U_ZERO_ERROR};
    RuleBasedBreakIterator bi(u"A{77777777777777};", pe, ec);
    assertEquals(WHERE, ec, U_BRK_RULE_SYNTAX);
    ec = U_ZERO_ERROR;
    RuleBasedBreakIterator bi2(u"A{2147483648};", pe, ec);
    assertEquals(WHERE, ec, U_BRK_RULE_SYNTAX);
    ec = U_ZERO_ERROR;
    RuleBasedBreakIterator bi3(u"A{2147483647};", pe, ec);
    assertEquals(WHERE, ec, U_ZERO_ERROR);
}

void RBBITest::TestBug22584() {
    // Creating a break iterator from a rule consisting of a very long
    // literal input string caused a stack overflow when deleting the
    // parse tree for the input during the rule building process.

    // Failure of this test showed as a crash during the break iterator construction.

    UnicodeString ruleStr(100000, static_cast<UChar32>(0), 100000);
    UParseError pe {};
    UErrorCode ec {U_ZERO_ERROR};

    RuleBasedBreakIterator bi(ruleStr, pe, ec);
    ec = U_ZERO_ERROR;
    ruleStr = u"a/b;c";
    RuleBasedBreakIterator bi2(ruleStr, pe, ec);
}

void RBBITest::TestBug22579() {
    // Test not causing null deref in cloneTree
    UnicodeString ruleStr = u"[{ab}];";
    UParseError pe {};
    UErrorCode ec {U_ZERO_ERROR};

    RuleBasedBreakIterator bi(ruleStr, pe, ec);
}
void RBBITest::TestBug22581() {
    // Test duplicate variable setting will not leak the rule compilation
    UnicodeString ruleStr = u"$foo=[abc]; $foo=[xyz]; $foo;";
    UParseError pe {};
    UErrorCode ec {U_ZERO_ERROR};

    RuleBasedBreakIterator bi(ruleStr, pe, ec);
}
#endif // #if !UCONFIG_NO_BREAK_ITERATION
