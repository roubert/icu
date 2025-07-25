// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
********************************************************************************
*   Copyright (C) 1999-2016 International Business Machines Corporation and
*   others. All Rights Reserved.
********************************************************************************
*   Date        Name        Description
*   10/20/99    alan        Creation.
*   03/22/2000  Madhu       Added additional tests
********************************************************************************
*/

#include <stdio.h>
#include <string.h>

#include <string_view>
#include <unordered_map>

#include "unicode/utypes.h"
#include "usettest.h"
#include "unicode/ucnv.h"
#include "unicode/uniset.h"
#include "unicode/uchar.h"
#include "unicode/usetiter.h"
#include "unicode/ustring.h"
#include "unicode/parsepos.h"
#include "unicode/symtable.h"
#include "unicode/utf8.h"
#include "unicode/utf16.h"
#include "unicode/uversion.h"
#include "cmemory.h"
#include "hash.h"

#define TEST_ASSERT_SUCCESS(status) UPRV_BLOCK_MACRO_BEGIN { \
    if (U_FAILURE(status)) { \
        dataerrln("fail in file \"%s\", line %d: \"%s\"", __FILE__, __LINE__, \
                  u_errorName(status)); \
    } \
} UPRV_BLOCK_MACRO_END

#define TEST_ASSERT(expr) UPRV_BLOCK_MACRO_BEGIN { \
    if (!(expr)) { \
        dataerrln("fail in file \"%s\", line %d", __FILE__, __LINE__); \
    } \
} UPRV_BLOCK_MACRO_END

UnicodeString operator+(const UnicodeString& left, const UnicodeSet& set) {
    UnicodeString pat;
    set.toPattern(pat);
    return left + UnicodeSetTest::escape(pat);
}

UnicodeSetTest::UnicodeSetTest() : utf8Cnv(nullptr) {
}

UConverter *UnicodeSetTest::openUTF8Converter() {
    if(utf8Cnv==nullptr) {
        UErrorCode errorCode=U_ZERO_ERROR;
        utf8Cnv=ucnv_open("UTF-8", &errorCode);
    }
    return utf8Cnv;
}

UnicodeSetTest::~UnicodeSetTest() {
    ucnv_close(utf8Cnv);
}

void
UnicodeSetTest::runIndexedTest(int32_t index, UBool exec,
                               const char* &name, char* /*par*/) {
    if (exec) {
        logln(u"TestSuite UnicodeSetTest");
    }
    TESTCASE_AUTO_BEGIN;
    TESTCASE_AUTO(TestPatterns);
    TESTCASE_AUTO(TestAddRemove);
    TESTCASE_AUTO(TestCategories);
    TESTCASE_AUTO(TestCloneEqualHash);
    TESTCASE_AUTO(TestMinimalRep);
    TESTCASE_AUTO(TestAPI);
    TESTCASE_AUTO(TestScriptSet);
    TESTCASE_AUTO(TestPropertySet);
    TESTCASE_AUTO(TestClone);
    TESTCASE_AUTO(TestExhaustive);
    TESTCASE_AUTO(TestToPattern);
    TESTCASE_AUTO(TestIndexOf);
    TESTCASE_AUTO(TestStrings);
    TESTCASE_AUTO(Testj2268);
    TESTCASE_AUTO(TestCloseOver);
    TESTCASE_AUTO(TestCloseOverSimpleCaseFolding);
    TESTCASE_AUTO(TestCloseOverLargeSets);
    TESTCASE_AUTO(TestEscapePattern);
    TESTCASE_AUTO(TestInvalidCodePoint);
    TESTCASE_AUTO(TestSymbolTable);
    TESTCASE_AUTO(TestSurrogate);
    TESTCASE_AUTO(TestPosixClasses);
    TESTCASE_AUTO(TestIteration);
    TESTCASE_AUTO(TestFreezable);
    TESTCASE_AUTO(TestSpan);
    TESTCASE_AUTO(TestStringSpan);
    TESTCASE_AUTO(TestPatternWithSurrogates);
    TESTCASE_AUTO(TestIntOverflow);
    TESTCASE_AUTO(TestUnusedCcc);
    TESTCASE_AUTO(TestDeepPattern);
    TESTCASE_AUTO(TestEmptyString);
    TESTCASE_AUTO(TestSkipToStrings);
    TESTCASE_AUTO(TestPatternCodePointComplement);
    TESTCASE_AUTO(TestCodePointIterator);
    TESTCASE_AUTO(TestRangeIterator);
    TESTCASE_AUTO(TestStringIterator);
    TESTCASE_AUTO(TestElementIterator);
    TESTCASE_AUTO_END;
}

static const char NOT[] = "%%%%";

/** 
 * UVector was improperly copying contents
 * This code will crash this is still true
 */
void UnicodeSetTest::Testj2268() {
  UnicodeSet t;
  t.add(UnicodeString("abc"));
  UnicodeSet test(t);
  UnicodeString ustrPat;
  test.toPattern(ustrPat, true);
}

/**
 * Test toPattern().
 */
void UnicodeSetTest::TestToPattern() {
    UErrorCode ec = U_ZERO_ERROR;

    // Test that toPattern() round trips with syntax characters and
    // whitespace.
    {
        static const char* OTHER_TOPATTERN_TESTS[] = {
            "[[:latin:]&[:greek:]]", 
            "[[:latin:]-[:greek:]]",
            "[:nonspacing mark:]",
            nullptr
        };

        for (int32_t j=0; OTHER_TOPATTERN_TESTS[j]!=nullptr; ++j) {
            ec = U_ZERO_ERROR;
            UnicodeSet s(OTHER_TOPATTERN_TESTS[j], ec);
            if (U_FAILURE(ec)) {
                dataerrln(UnicodeString("FAIL: bad pattern ") + OTHER_TOPATTERN_TESTS[j] + " - " + UnicodeString(u_errorName(ec)));
                continue;
            }
            checkPat(OTHER_TOPATTERN_TESTS[j], s);
        }
    
        for (UChar32 i = 0; i <= 0x10FFFF; ++i) {
            if ((i <= 0xFF && !u_isalpha(i)) || u_isspace(i)) {

                // check various combinations to make sure they all work.
                if (i != 0 && !toPatternAux(i, i)){
                    continue;
                }
                if (!toPatternAux(0, i)){
                    continue;
                }
                if (!toPatternAux(i, 0xFFFF)){
                    continue;
                }
            }
        }
    }

    // Test pattern behavior of multicharacter strings.
    {
        ec = U_ZERO_ERROR;
        UnicodeSet* s = new UnicodeSet("[a-z {aa} {ab}]", ec);

        // This loop isn't a loop.  It's here to make the compiler happy.
        // If you're curious, try removing it and changing the 'break'
        // statements (except for the last) to goto's.
        for (;;) {
            if (U_FAILURE(ec)) break;
            const char* exp1[] = {"aa", "ab", NOT, "ac", nullptr};
            expectToPattern(*s, "[a-z{aa}{ab}]", exp1);

            s->add("ac");
            const char* exp2[] = {"aa", "ab", "ac", NOT, "xy", nullptr};
            expectToPattern(*s, "[a-z{aa}{ab}{ac}]", exp2);

            s->applyPattern(u"[a-z {\\{l} {r\\}}]", ec);
            if (U_FAILURE(ec)) break;
            const char* exp3[] = {"{l", "r}", NOT, "xy", nullptr};
            expectToPattern(*s, u"[a-z{r\\}}{\\{l}]", exp3);

            s->add("[]");
            const char* exp4[] = {"{l", "r}", "[]", NOT, "xy", nullptr};
            expectToPattern(*s, u"[a-z{\\[\\]}{r\\}}{\\{l}]", exp4);

            s->applyPattern(u"[a-z {\\u4E01\\u4E02}{\\n\\r}]", ec);
            if (U_FAILURE(ec)) break;
            const char* exp5[] = {"\\u4E01\\u4E02", "\n\r", nullptr};
            expectToPattern(*s, u"[a-z{\\u000A\\u000D}{\\u4E01\\u4E02}]", exp5);

            // j2189
            s->clear();
            s->add(UnicodeString("abc", ""));
            s->add(UnicodeString("abc", ""));
            const char* exp6[] = {"abc", NOT, "ab", nullptr};
            expectToPattern(*s, "[{abc}]", exp6);

            break;
        }

        if (U_FAILURE(ec)) errln("FAIL: pattern parse error");
        delete s;
    }
 
    // JB#3400: For 2 character ranges prefer [ab] to [a-b]
    UnicodeSet s;
    s.add(u'a', u'b');
    expectToPattern(s, "[ab]", nullptr);
}
    
UBool UnicodeSetTest::toPatternAux(UChar32 start, UChar32 end) {

    // use Integer.toString because Utility.hex doesn't handle ints
    UnicodeString pat = "";
    // TODO do these in hex
    //String source = "0x" + Integer.toString(start,16).toUpperCase();
    //if (start != end) source += "..0x" + Integer.toString(end,16).toUpperCase();
    UnicodeString source;
    source = source + static_cast<uint32_t>(start);
    if (start != end) 
        source = source + ".." + static_cast<uint32_t>(end);
    UnicodeSet testSet;
    testSet.add(start, end);
    return checkPat(source, testSet);
}
    
UBool UnicodeSetTest::checkPat(const UnicodeString& source,
                               const UnicodeSet& testSet) {
    // What we want to make sure of is that a pattern generated
    // by toPattern(), with or without escaped unprintables, can
    // be passed back into the UnicodeSet constructor.
    UnicodeString pat0;

    testSet.toPattern(pat0, true);
    
    if (!checkPat(source + " (escaped)", testSet, pat0)) return false;
    
    //String pat1 = unescapeLeniently(pat0);
    //if (!checkPat(source + " (in code)", testSet, pat1)) return false;
    
    UnicodeString pat2; 
    testSet.toPattern(pat2, false);
    if (!checkPat(source, testSet, pat2)) return false;
    
    //String pat3 = unescapeLeniently(pat2);
    // if (!checkPat(source + " (in code)", testSet, pat3)) return false;
    
    //logln(source + " => " + pat0 + ", " + pat1 + ", " + pat2 + ", " + pat3);
    logln(UnicodeString(source) + " => " + pat0 + ", " + pat2);
    return true;
}

UBool UnicodeSetTest::checkPat(const UnicodeString& source,
                               const UnicodeSet& testSet,
                               const UnicodeString& pat) {
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeSet testSet2(pat, ec);
    if (testSet2 != testSet) {
        errln(UnicodeString("Fail toPattern: ") + source + " => " + pat);
        return false;
    }
    return true;
}

void
UnicodeSetTest::TestPatterns() {
    UnicodeSet set;
    expectPattern(set, UnicodeString("[[a-m]&[d-z]&[k-y]]", ""),  "km");
    expectPattern(set, UnicodeString("[[a-z]-[m-y]-[d-r]]", ""),  "aczz");
    expectPattern(set, UnicodeString("[a\\-z]", ""),  "--aazz");
    expectPattern(set, UnicodeString("[-az]", ""),  "--aazz");
    expectPattern(set, UnicodeString("[az-]", ""),  "--aazz");
    expectPattern(set, UnicodeString("[[[a-z]-[aeiou]i]]", ""), "bdfnptvz");

    // Throw in a test of complement
    set.complement();
    UnicodeString exp;
    exp.append(static_cast<char16_t>(0x0000)).append("aeeoouu").append(static_cast<char16_t>(u'z' + 1)).append(static_cast<char16_t>(0xFFFF));
    expectPairs(set, exp);
}

void
UnicodeSetTest::TestCategories() {
    UErrorCode status = U_ZERO_ERROR;
    const char* pat = " [:Lu:] "; // Whitespace ok outside [:..:]
    UnicodeSet set(pat, status);
    if (U_FAILURE(status)) {
        dataerrln(UnicodeString("Fail: Can't construct set with ") + pat + " - " + UnicodeString(u_errorName(status)));
        return;
    } else {
        expectContainment(set, pat, "ABC", "abc");
    }

    UChar32 i;
    int32_t failures = 0;
    // Make sure generation of L doesn't pollute cached Lu set
    // First generate L, then Lu
    set.applyPattern("[:L:]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    for (i=0; i<0x200; ++i) {
        UBool l = u_isalpha(static_cast<char16_t>(i));
        if (l != set.contains(i)) {
            errln(UnicodeString("FAIL: L contains ") + static_cast<unsigned short>(i) + " = " +
                  set.contains(i));
            if (++failures == 10) break;
        }
    }
    
    set.applyPattern("[:Lu:]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    for (i=0; i<0x200; ++i) {
        UBool lu = (u_charType(static_cast<char16_t>(i)) == U_UPPERCASE_LETTER);
        if (lu != set.contains(i)) {
            errln(UnicodeString("FAIL: Lu contains ") + static_cast<unsigned short>(i) + " = " +
                  set.contains(i));
            if (++failures == 20) break;
        }
    }
}
void
UnicodeSetTest::TestCloneEqualHash() {
    UErrorCode status = U_ZERO_ERROR;
    // set1 and set2 used to be built with the obsolete constructor taking
    // UCharCategory values; replaced with pattern constructors
    // markus 20030502
    UnicodeSet *set1=new UnicodeSet(u"\\p{Lowercase Letter}", status); //  :Ll: Letter, lowercase
    UnicodeSet *set1a=new UnicodeSet(u"[:Ll:]", status); //  Letter, lowercase
    if (U_FAILURE(status)){
        dataerrln(UnicodeString("FAIL: Can't construst set with category->Ll") + " - " + UnicodeString(u_errorName(status)));
        return;
    }
    UnicodeSet *set2=new UnicodeSet(u"\\p{Decimal Number}", status);   //Number, Decimal digit
    UnicodeSet *set2a=new UnicodeSet(u"[:Nd:]", status);   //Number, Decimal digit
    if (U_FAILURE(status)){
        errln(UnicodeString("FAIL: Can't construct set with category->Nd"));
        return;
    }

    if (*set1 != *set1a) {
        errln("FAIL: category constructor for Ll broken");
    }
    if (*set2 != *set2a) {
        errln("FAIL: category constructor for Nd broken");
    }
    delete set1a;
    delete set2a;

    logln("Testing copy construction");
    UnicodeSet *set1copy=new UnicodeSet(*set1);
    if(*set1 != *set1copy || *set1 == *set2 || 
        getPairs(*set1) != getPairs(*set1copy) ||
        set1->hashCode() != set1copy->hashCode()){
        errln("FAIL : Error in copy construction");
        return;
    }

    logln("Testing =operator");
    UnicodeSet set1equal=*set1;
    UnicodeSet set2equal=*set2;
    if(set1equal != *set1 || set1equal != *set1copy || set2equal != *set2 || 
        set2equal == *set1 || set2equal == *set1copy || set2equal == set1equal){
        errln("FAIL: Error in =operator");
    }

    logln("Testing clone()");
    UnicodeSet *set1clone=set1->clone();
    UnicodeSet *set2clone=set2->clone();
    if(*set1clone != *set1 || *set1clone != *set1copy || *set1clone != set1equal || 
        *set2clone != *set2 || *set2clone == *set1copy || *set2clone != set2equal || 
        *set2clone == *set1 || *set2clone == set1equal || *set2clone == *set1clone){
        errln("FAIL: Error in clone");
    }

    logln("Testing hashcode");
    if(set1->hashCode() != set1equal.hashCode() || set1->hashCode() != set1clone->hashCode() ||
        set2->hashCode() != set2equal.hashCode() || set2->hashCode() != set2clone->hashCode() ||
        set1copy->hashCode() != set1equal.hashCode() || set1copy->hashCode() != set1clone->hashCode() ||
        set1->hashCode() == set2->hashCode()  || set1copy->hashCode() == set2->hashCode() ||
        set2->hashCode() == set1clone->hashCode() || set2->hashCode() == set1equal.hashCode() ){
        errln("FAIL: Error in hashCode()");
    }

    delete set1;
    delete set1copy;
    delete set2;
    delete set1clone;
    delete set2clone;


}
void
UnicodeSetTest::TestAddRemove() {
    UnicodeSet set; // Construct empty set
    doAssert(set.isEmpty() == true, "set should be empty");
    doAssert(set.size() == 0, "size should be 0");
    set.complement();
    doAssert(set.size() == 0x110000, "size should be 0x110000");
    set.clear();
    set.add(0x0061, 0x007a);
    expectPairs(set, "az");
    doAssert(set.isEmpty() == false, "set should not be empty");
    doAssert(set.size() != 0, "size should not be equal to 0");
    doAssert(set.size() == 26, "size should be equal to 26");
    set.remove(0x006d, 0x0070);
    expectPairs(set, "alqz");
    doAssert(set.size() == 22, "size should be equal to 22");
    set.remove(0x0065, 0x0067);
    expectPairs(set, "adhlqz");
    doAssert(set.size() == 19, "size should be equal to 19");
    set.remove(0x0064, 0x0069);
    expectPairs(set, "acjlqz");
    doAssert(set.size() == 16, "size should be equal to 16");
    set.remove(0x0063, 0x0072);
    expectPairs(set, "absz");
    doAssert(set.size() == 10, "size should be equal to 10");
    set.add(0x0066, 0x0071);
    expectPairs(set, "abfqsz");
    doAssert(set.size() == 22, "size should be equal to 22");
    set.remove(0x0061, 0x0067);
    expectPairs(set, "hqsz");
    set.remove(0x0061, 0x007a);
    expectPairs(set, "");
    doAssert(set.isEmpty() == true, "set should be empty");
    doAssert(set.size() == 0, "size should be 0");
    set.add(0x0061);
    doAssert(set.isEmpty() == false, "set should not be empty");
    doAssert(set.size() == 1, "size should not be equal to 1");
    set.add(0x0062);
    set.add(0x0063);
    expectPairs(set, "ac");
    doAssert(set.size() == 3, "size should not be equal to 3");
    set.add(0x0070);
    set.add(0x0071);
    expectPairs(set, "acpq");
    doAssert(set.size() == 5, "size should not be equal to 5");
    set.clear();
    expectPairs(set, "");
    doAssert(set.isEmpty() == true, "set should be empty");
    doAssert(set.size() == 0, "size should be 0");

    // Try removing an entire set from another set
    expectPattern(set, "[c-x]", "cx");
    UnicodeSet set2;
    expectPattern(set2, "[f-ky-za-bc[vw]]", "acfkvwyz");
    set.removeAll(set2);
    expectPairs(set, "deluxx");

    // Try adding an entire set to another set
    expectPattern(set, "[jackiemclean]", "aacceein");
    expectPattern(set2, "[hitoshinamekatajamesanderson]", "aadehkmort");
    set.addAll(set2);
    expectPairs(set, "aacehort");
    doAssert(set.containsAll(set2) == true, "set should contain all the elements in set2");

    // Try retaining an set of elements contained in another set (intersection)
    UnicodeSet set3;
    expectPattern(set3, "[a-c]", "ac");
    doAssert(set.containsAll(set3) == false, "set doesn't contain all the elements in set3");
    set3.remove(0x0062);
    expectPairs(set3, "aacc");
    doAssert(set.containsAll(set3) == true, "set should contain all the elements in set3");
    set.retainAll(set3);
    expectPairs(set, "aacc");
    doAssert(set.size() == set3.size(), "set.size() should be set3.size()");
    doAssert(set.containsAll(set3) == true, "set should contain all the elements in set3");
    set.clear();
    doAssert(set.size() != set3.size(), "set.size() != set3.size()");

    // Test commutativity
    expectPattern(set, "[hitoshinamekatajamesanderson]", "aadehkmort");
    expectPattern(set2, "[jackiemclean]", "aacceein");
    set.addAll(set2);
    expectPairs(set, "aacehort");
    doAssert(set.containsAll(set2) == true, "set should contain all the elements in set2");




}

/**
 * Make sure minimal representation is maintained.
 */
void UnicodeSetTest::TestMinimalRep() {
    UErrorCode status = U_ZERO_ERROR;
    // This is pretty thoroughly tested by checkCanonicalRep()
    // run against the exhaustive operation results.  Use the code
    // here for debugging specific spot problems.

    // 1 overlap against 2
    UnicodeSet set("[h-km-q]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    UnicodeSet set2("[i-o]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.addAll(set2);
    expectPairs(set, "hq");
    // right
    set.applyPattern("[a-m]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set2.applyPattern("[e-o]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.addAll(set2);
    expectPairs(set, "ao");
    // left
    set.applyPattern("[e-o]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set2.applyPattern("[a-m]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.addAll(set2);
    expectPairs(set, "ao");
    // 1 overlap against 3
    set.applyPattern("[a-eg-mo-w]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set2.applyPattern("[d-q]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.addAll(set2);
    expectPairs(set, "aw");
}

void UnicodeSetTest::TestAPI() {
    UErrorCode status = U_ZERO_ERROR;
    // default ct
    UnicodeSet set;
    if (!set.isEmpty() || set.getRangeCount() != 0) {
        errln(UnicodeString("FAIL, set should be empty but isn't: ") + set);
    }

    // clear(), isEmpty()
    set.add(0x0061);
    if (set.isEmpty()) {
        errln(UnicodeString("FAIL, set shouldn't be empty but is: ") + set);
    }
    set.clear();
    if (!set.isEmpty()) {
        errln(UnicodeString("FAIL, set should be empty but isn't: ") + set);
    }

    // size()
    set.clear();
    if (set.size() != 0) {
        errln(UnicodeString("FAIL, size should be 0, but is ") + set.size() +
              ": " + set);
    }
    set.add(0x0061);
    if (set.size() != 1) {
        errln(UnicodeString("FAIL, size should be 1, but is ") + set.size() +
              ": " + set);
    }
    set.add(0x0031, 0x0039);
    if (set.size() != 10) {
        errln(UnicodeString("FAIL, size should be 10, but is ") + set.size() +
              ": " + set);
    }

    // contains(first, last)
    set.clear();
    set.applyPattern("[A-Y 1-8 b-d l-y]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    for (int32_t i = 0; i<set.getRangeCount(); ++i) {
        UChar32 a = set.getRangeStart(i);
        UChar32 b = set.getRangeEnd(i);
        if (!set.contains(a, b)) {
            errln(UnicodeString("FAIL, should contain ") + static_cast<unsigned short>(a) + '-' + static_cast<unsigned short>(b) +
                  " but doesn't: " + set);
        }
        if (set.contains(static_cast<UChar32>(a - 1), b)) {
            errln(UnicodeString("FAIL, shouldn't contain ") +
                  static_cast<unsigned short>(a - 1) + '-' + static_cast<unsigned short>(b) +
                  " but does: " + set);
        }
        if (set.contains(a, static_cast<UChar32>(b + 1))) {
            errln(UnicodeString("FAIL, shouldn't contain ") +
                  static_cast<unsigned short>(a) + '-' + static_cast<unsigned short>(b + 1) +
                  " but does: " + set);
        }
    }

    // Ported InversionList test.
    UnicodeSet a(static_cast<UChar32>(3), static_cast<UChar32>(10));
    UnicodeSet b(static_cast<UChar32>(7), static_cast<UChar32>(15));
    UnicodeSet c;

    logln(UnicodeString("a [3-10]: ") + a);
    logln(UnicodeString("b [7-15]: ") + b);
    c = a;
    c.addAll(b);
    UnicodeSet exp(static_cast<UChar32>(3), static_cast<UChar32>(15));
    if (c == exp) {
        logln(UnicodeString("c.set(a).add(b): ") + c);
    } else {
        errln(UnicodeString("FAIL: c.set(a).add(b) = ") + c + ", expect " + exp);
    }
    c.complement();
    exp.set(static_cast<UChar32>(0), static_cast<UChar32>(2));
    exp.add(static_cast<UChar32>(16), UnicodeSet::MAX_VALUE);
    if (c == exp) {
        logln(UnicodeString("c.complement(): ") + c);
    } else {
        errln(UnicodeString("FAIL: c.complement() = ") + c + ", expect " + exp);
    }
    c.complement();
    exp.set(static_cast<UChar32>(3), static_cast<UChar32>(15));
    if (c == exp) {
        logln(UnicodeString("c.complement(): ") + c);
    } else {
        errln(UnicodeString("FAIL: c.complement() = ") + c + ", expect " + exp);
    }
    c = a;
    c.complementAll(b);
    exp.set(static_cast<UChar32>(3), static_cast<UChar32>(6));
    exp.add(static_cast<UChar32>(11), static_cast<UChar32>(15));
    if (c == exp) {
        logln(UnicodeString("c.set(a).exclusiveOr(b): ") + c);
    } else {
        errln(UnicodeString("FAIL: c.set(a).exclusiveOr(b) = ") + c + ", expect " + exp);
    }

    exp = c;
    bitsToSet(setToBits(c), c);
    if (c == exp) {
        logln(UnicodeString("bitsToSet(setToBits(c)): ") + c);
    } else {
        errln(UnicodeString("FAIL: bitsToSet(setToBits(c)) = ") + c + ", expect " + exp);
    }

    // Additional tests for coverage JB#2118
    //UnicodeSet::complement(class UnicodeString const &)
    //UnicodeSet::complementAll(class UnicodeString const &)
    //UnicodeSet::containsNone(class UnicodeSet const &)
    //UnicodeSet::containsNone(long,long)
    //UnicodeSet::containsSome(class UnicodeSet const &)
    //UnicodeSet::containsSome(long,long)
    //UnicodeSet::removeAll(class UnicodeString const &)
    //UnicodeSet::retain(long)
    //UnicodeSet::retainAll(class UnicodeString const &)
    //UnicodeSet::serialize(unsigned short *,long,enum UErrorCode &)
    //UnicodeSetIterator::getString()
    set.clear();
    set.complement("ab");
    exp.applyPattern("[{ab}]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (set != exp) { errln("FAIL: complement(\"ab\")"); return; }
    
    UnicodeSetIterator iset(set);
    if (!iset.next() || !iset.isString()) {
        errln("FAIL: UnicodeSetIterator::next/isString");
    } else if (iset.getString() != "ab") {
        errln("FAIL: UnicodeSetIterator::getString");
    }

    set.add(u'a', u'z');
    set.complementAll("alan");
    exp.applyPattern("[{ab}b-kmo-z]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (set != exp) { errln("FAIL: complementAll(\"alan\")"); return; }

    exp.applyPattern("[a-z]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (set.containsNone(exp)) { errln("FAIL: containsNone(UnicodeSet)"); }
    if (!set.containsSome(exp)) { errln("FAIL: containsSome(UnicodeSet)"); }
    exp.applyPattern("[aln]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (!set.containsNone(exp)) { errln("FAIL: containsNone(UnicodeSet)"); }
    if (set.containsSome(exp)) { errln("FAIL: containsSome(UnicodeSet)"); }

    if (set.containsNone(u'a', u'z')) {
        errln("FAIL: containsNone(UChar32, UChar32)");
    }
    if (!set.containsSome(u'a', u'z')) {
        errln("FAIL: containsSome(UChar32, UChar32)");
    }
    if (!set.containsNone(u'A', u'Z')) {
        errln("FAIL: containsNone(UChar32, UChar32)");
    }
    if (set.containsSome(u'A', u'Z')) {
        errln("FAIL: containsSome(UChar32, UChar32)");
    }

    set.removeAll("liu");
    exp.applyPattern("[{ab}b-hj-kmo-tv-z]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (set != exp) { errln("FAIL: removeAll(\"liu\")"); return; }

    set.retainAll("star");
    exp.applyPattern("[rst]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (set != exp) { errln("FAIL: retainAll(\"star\")"); return; }

    set.retain(u's');
    exp.applyPattern("[s]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (set != exp) { errln("FAIL: retain('s')"); return; }

    // ICU 2.6 coverage tests
    // public final UnicodeSet retain(String s);
    // public final UnicodeSet remove(int c);
    // public final UnicodeSet remove(String s);
    // public int hashCode();
    set.applyPattern(u"[a-z{ab}{cd}]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.retain(u"cd");
    exp.applyPattern(u"[{cd}]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (set != exp) { errln("FAIL: (with cd).retain(\"cd\")"); return; }

    set.applyPattern(u"[a-z{ab}{yz}]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.retain(u"cd");
    exp.clear();
    if (set != exp) { errln("FAIL: (without cd).retain(\"cd\")"); return; }

    set.applyPattern(u"[a-z{ab}{cd}]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    set.remove(u'c');
    exp.applyPattern(u"[abd-z{ab}{cd}]", status);
    if (set != exp) { errln("FAIL: remove('c')"); return; }

    set.remove(u"cd");
    exp.applyPattern(u"[abd-z{ab}]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    if (set != exp) { errln("FAIL: remove(\"cd\")"); return; }

    set.applyPattern("[s]", status);
    if (U_FAILURE(status)) { errln("FAIL"); return; }
    uint16_t buf[32];
    int32_t slen = set.serialize(buf, UPRV_LENGTHOF(buf), status);
    if (U_FAILURE(status)) { errln("FAIL: serialize"); return; }
    if (slen != 3 || buf[0] != 2 || buf[1] != u's' || buf[2] != u't') {
        errln("FAIL: serialize");
        return;
    }

    // Conversions to and from USet
    UnicodeSet *uniset = &set;
    USet *uset = uniset->toUSet();
    TEST_ASSERT((void *)uset == (void *)uniset);
    UnicodeSet *setx = UnicodeSet::fromUSet(uset);
    TEST_ASSERT((void *)setx == (void *)uset);
    const UnicodeSet *constSet = uniset;
    const USet *constUSet = constSet->toUSet();
    TEST_ASSERT((void *)constUSet == (void *)constSet);
    const UnicodeSet *constSetx = UnicodeSet::fromUSet(constUSet);
    TEST_ASSERT((void *)constSetx == (void *)constUSet);

    // span(UnicodeString) and spanBack(UnicodeString) convenience methods
    UnicodeString longString=u"aaaaaaaaaabbbbbbbbbbcccccccccc";
    UnicodeSet ac(0x61, 0x63);
    ac.remove(0x62).freeze();
    if( ac.span(longString, -5, USET_SPAN_CONTAINED)!=10 ||
        ac.span(longString, 0, USET_SPAN_CONTAINED)!=10 ||
        ac.span(longString, 5, USET_SPAN_CONTAINED)!=10 ||
        ac.span(longString, 10, USET_SPAN_CONTAINED)!=10 ||
        ac.span(longString, 15, USET_SPAN_CONTAINED)!=15 ||
        ac.span(longString, 20, USET_SPAN_CONTAINED)!=30 ||
        ac.span(longString, 25, USET_SPAN_CONTAINED)!=30 ||
        ac.span(longString, 30, USET_SPAN_CONTAINED)!=30 ||
        ac.span(longString, 35, USET_SPAN_CONTAINED)!=30 ||
        ac.span(longString, INT32_MAX, USET_SPAN_CONTAINED)!=30
    ) {
        errln("UnicodeSet.span(UnicodeString, ...) returns incorrect end indexes");
    }
    if( ac.spanBack(longString, -5, USET_SPAN_CONTAINED)!=0 ||
        ac.spanBack(longString, 0, USET_SPAN_CONTAINED)!=0 ||
        ac.spanBack(longString, 5, USET_SPAN_CONTAINED)!=0 ||
        ac.spanBack(longString, 10, USET_SPAN_CONTAINED)!=0 ||
        ac.spanBack(longString, 15, USET_SPAN_CONTAINED)!=15 ||
        ac.spanBack(longString, 20, USET_SPAN_CONTAINED)!=20 ||
        ac.spanBack(longString, 25, USET_SPAN_CONTAINED)!=20 ||
        ac.spanBack(longString, 30, USET_SPAN_CONTAINED)!=20 ||
        ac.spanBack(longString, 35, USET_SPAN_CONTAINED)!=20 ||
        ac.spanBack(longString, INT32_MAX, USET_SPAN_CONTAINED)!=20
    ) {
        errln("UnicodeSet.spanBack(UnicodeString, ...) returns incorrect start indexes");
    }
}

void UnicodeSetTest::TestIteration() {
    UErrorCode ec = U_ZERO_ERROR;
    int i = 0;
    int outerLoop;
    
    // 6 code points, 3 ranges, 2 strings, 8 total elements
    //   Iteration will access them in sorted order -  a, b, c, y, z, U0001abcd, "str1", "str2"
    UnicodeSet set(u"[zabyc\\U0001abcd{str1}{str2}]", ec);
    TEST_ASSERT_SUCCESS(ec);
    UnicodeSetIterator it(set);

    for (outerLoop=0; outerLoop<3; outerLoop++) {
        // Run the test multiple times, to check that iterator.reset() is working.
        for (i=0; i<10; i++) {
            UBool         nextv        = it.next();
            UBool         isString     = it.isString();
            int32_t       codePoint    = it.getCodepoint();
            //int32_t       codePointEnd = it.getCodepointEnd();
            UnicodeString s   = it.getString();
            switch (i) {
            case 0:
                TEST_ASSERT(nextv == true);
                TEST_ASSERT(isString == false);
                TEST_ASSERT(codePoint==0x61);
                TEST_ASSERT(s == "a");
                break;
            case 1:
                TEST_ASSERT(nextv == true);
                TEST_ASSERT(isString == false);
                TEST_ASSERT(codePoint==0x62);
                TEST_ASSERT(s == "b");
                break;
            case 2:
                TEST_ASSERT(nextv == true);
                TEST_ASSERT(isString == false);
                TEST_ASSERT(codePoint==0x63);
                TEST_ASSERT(s == "c");
                break;
            case 3:
                TEST_ASSERT(nextv == true);
                TEST_ASSERT(isString == false);
                TEST_ASSERT(codePoint==0x79);
                TEST_ASSERT(s == "y");
                break;
            case 4:
                TEST_ASSERT(nextv == true);
                TEST_ASSERT(isString == false);
                TEST_ASSERT(codePoint==0x7a);
                TEST_ASSERT(s == "z");
                break;
            case 5:
                TEST_ASSERT(nextv == true);
                TEST_ASSERT(isString == false);
                TEST_ASSERT(codePoint==0x1abcd);
                TEST_ASSERT(s == UnicodeString((UChar32)0x1abcd));
                break;
            case 6:
                TEST_ASSERT(nextv == true);
                TEST_ASSERT(isString == true);
                TEST_ASSERT(s == "str1");
                break;
            case 7:
                TEST_ASSERT(nextv == true);
                TEST_ASSERT(isString == true);
                TEST_ASSERT(s == "str2");
                break;
            case 8:
                TEST_ASSERT(nextv == false);
                break;
            case 9:
                TEST_ASSERT(nextv == false);
                break;
            }
        }
        it.reset();  // prepare to run the iteration again.
    }
}
                



void UnicodeSetTest::TestStrings() {
    UErrorCode ec = U_ZERO_ERROR;
    
    UnicodeSet* testList[] = {
        UnicodeSet::createFromAll("abc"),
        new UnicodeSet("[a-c]", ec),
        
        &(UnicodeSet::createFrom("ch")->add('a','z').add("ll")),
        new UnicodeSet("[{ll}{ch}a-z]", ec),
    
        UnicodeSet::createFrom("ab}c"),
        new UnicodeSet("[{ab\\}c}]", ec),

        &((new UnicodeSet('a','z'))->add('A', 'Z').retain('M','m').complement('X')), 
        new UnicodeSet("[[a-zA-Z]&[M-m]-[X]]", ec),

        nullptr
    };

    if (U_FAILURE(ec)) {
        errln("FAIL: couldn't construct test sets");
    }
    assertFalse("[a-c].hasStrings()", testList[0]->hasStrings());
    assertTrue("[{ll}{ch}a-z].hasStrings()", testList[2]->hasStrings());

    for (int32_t i = 0; testList[i] != nullptr; i+=2) {
        if (U_SUCCESS(ec)) {
            UnicodeString pat0, pat1;
            testList[i]->toPattern(pat0, true);
            testList[i+1]->toPattern(pat1, true);
            if (*testList[i] == *testList[i+1]) {
                logln(UnicodeString("Ok: ") + pat0 + " == " + pat1);
            } else {
                logln(UnicodeString("FAIL: ") + pat0 + " != " + pat1);
            }
        }
        delete testList[i];
        delete testList[i+1];
    }
}

/**
 * Test the [:Latin:] syntax.
 */
void UnicodeSetTest::TestScriptSet() {
    expectContainment(u"[:Latin:]", "aA", CharsToUnicodeString("\\u0391\\u03B1"));

    expectContainment(u"[:Greek:]", CharsToUnicodeString("\\u0391\\u03B1"), "aA");
    
    /* Jitterbug 1423 */
    expectContainment(u"[[:Common:][:Inherited:]]", CharsToUnicodeString("\\U00003099\\U0001D169\\u0000"), "aA");

}

/**
 * Test the [:Latin:] syntax.
 */
void UnicodeSetTest::TestPropertySet() {
    static const char* const DATA[] = {
        // Pattern, Chars IN, Chars NOT in

        "[:Latin:]",
        "aA",
        "\\u0391\\u03B1",

        "[\\p{Greek}]",
        "\\u0391\\u03B1",
        "aA",

        "\\P{ GENERAL Category = upper case letter }",
        "abc",
        "ABC",

#if !UCONFIG_NO_NORMALIZATION
        // Combining class: @since ICU 2.2
        // Check both symbolic and numeric
        "\\p{ccc=Nukta}",
        "\\u0ABC",
        "abc",

        "\\p{Canonical Combining Class = 11}",
        "\\u05B1",
        "\\u05B2",

        "[:c c c = iota subscript :]",
        "\\u0345",
        "xyz",
#endif

        // Bidi class: @since ICU 2.2
        "\\p{bidiclass=lefttoright}",
        "abc",
        "\\u0671\\u0672",

        // Binary properties: @since ICU 2.2
        "\\p{ideographic}",
        "\\u4E0A",
        "x",

        "[:math=false:]",
        "q)*(",
        // weiv: )(and * were removed from math in Unicode 4.0.1
        //"(*+)",
        "+<>^",

        // JB#1767 \N{}, \p{ASCII}
        "[:Ascii:]",
        "abc\\u0000\\u007F",
        "\\u0080\\u4E00",
        
        "[\\N{ latin small letter  a  }[:name= latin small letter z:]]",
        "az",
        "qrs",

        // JB#2015
        "[:any:]",
        "a\\U0010FFFF",
        "",

        "[:nv=0.5:]",
        "\\u00BD\\u0F2A",
        "\\u00BC",

        // JB#2653: Age
        "[:Age=1.1:]",
        "\\u03D6", // 1.1
        "\\u03D8\\u03D9", // 3.2
        
        "[:Age=3.1:]",
        "\\u1800\\u3400\\U0002f800",
        "\\u0220\\u034f\\u30ff\\u33ff\\ufe73\\U00010000\\U00050000",

        // JB#2350: Case_Sensitive
        "[:Case Sensitive:]",
        "A\\u1FFC\\U00010410",
        ";\\u00B4\\U00010500",

        // JB#2832: C99-compatibility props
        "[:blank:]",
        " \\u0009",
        "1-9A-Z",

        "[:graph:]",
        "19AZ",
        " \\u0003\\u0007\\u0009\\u000A\\u000D",

        "[:punct:]",
        "!@#%&*()[]{}-_\\/;:,.?'\"",
        "09azAZ",

        "[:xdigit:]",
        "09afAF",
        "gG!",

        // Regex compatibility test
        "[-b]", // leading '-' is literal
        "-b",
        "ac",

        "[^-b]", // leading '-' is literal
        "ac",
        "-b",

        "[b-]", // trailing '-' is literal
        "-b",
        "ac",

        "[^b-]", // trailing '-' is literal
        "ac",
        "-b",

        "[a-b-]", // trailing '-' is literal
        "ab-",
        "c=",
        
        "[[a-q]&[p-z]-]", // trailing '-' is literal
        "pq-",
        "or=",

        "[\\s|\\)|:|$|\\>]", // from regex tests
        "s|):$>",
        "abc",

        "[\\uDC00cd]", // JB#2906: isolated trail at start
        "cd\\uDC00",
        "ab\\uD800\\U00010000",
        
        "[ab\\uD800]", // JB#2906: isolated trail at start
        "ab\\uD800",
        "cd\\uDC00\\U00010000",
        
        "[ab\\uD800cd]", // JB#2906: isolated lead in middle
        "abcd\\uD800",
        "ef\\uDC00\\U00010000",
        
        "[ab\\uDC00cd]", // JB#2906: isolated trail in middle
        "abcd\\uDC00",
        "ef\\uD800\\U00010000",

#if !UCONFIG_NO_NORMALIZATION
        "[:^lccc=0:]", // Lead canonical class
        "\\u0300\\u0301",
        "abcd\\u00c0\\u00c5",

        "[:^tccc=0:]", // Trail canonical class
        "\\u0300\\u0301\\u00c0\\u00c5",
        "abcd",

        "[[:^lccc=0:][:^tccc=0:]]", // Lead and trail canonical class
        "\\u0300\\u0301\\u00c0\\u00c5",
        "abcd",

        "[[:^lccc=0:]-[:^tccc=0:]]", // Stuff that starts with an accent but ends with a base (none right now)
        "",
        "abcd\\u0300\\u0301\\u00c0\\u00c5",
        
        "[[:ccc=0:]-[:lccc=0:]-[:tccc=0:]]", // Weirdos. Complete canonical class is zero, but both lead and trail are not
        "\\u0F73\\u0F75\\u0F81",
        "abcd\\u0300\\u0301\\u00c0\\u00c5",
#endif /* !UCONFIG_NO_NORMALIZATION */

        "[:Assigned:]",
        "A\\uE000\\uF8FF\\uFDC7\\U00010000\\U0010FFFD",
        "\\u0558\\uFDD3\\uFFFE\\U00050005",

        // Script_Extensions, new in Unicode 6.0
        "[:scx=Arab:]",
        "\\u061E\\u061F\\u0620\\u0621\\u063F\\u0640\\u0650\\u065E\\uFDF1\\uFDF2\\uFDF3",
        "\\uFDEF\\uFEFE",

        // U+FDF2 has Script=Arabic and also Arab in its Script_Extensions,
        // so scx-sc is missing U+FDF2.
        "[[:Script_Extensions=Arabic:]-[:Arab:]]",
        "\\u0640\\u064B\\u0650\\u0655",
        "\\uFDF2"
    };

    static const int32_t DATA_LEN = UPRV_LENGTHOF(DATA);

    for (int32_t i=0; i<DATA_LEN; i+=3) {  
        expectContainment(UnicodeString(DATA[i], -1, US_INV), CharsToUnicodeString(DATA[i+1]),
                          CharsToUnicodeString(DATA[i+2]));
    }
}

/**
  * Test that Posix style character classes [:digit:], etc.
  *   have the Unicode definitions from TR 18.
  */
void UnicodeSetTest::TestPosixClasses() {
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:alpha:]", status);
        UnicodeSet s2(u"\\p{Alphabetic}", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:lower:]", status);
        UnicodeSet s2(u"\\p{lowercase}", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:upper:]", status);
        UnicodeSet s2(u"\\p{Uppercase}", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:punct:]", status);
        UnicodeSet s2(u"\\p{gc=Punctuation}", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:digit:]", status);
        UnicodeSet s2(u"\\p{gc=DecimalNumber}", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:xdigit:]", status);
        UnicodeSet s2(u"[\\p{DecimalNumber}\\p{HexDigit}]", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:alnum:]", status);
        UnicodeSet s2(u"[\\p{Alphabetic}\\p{DecimalNumber}]", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:space:]", status);
        UnicodeSet s2(u"\\p{Whitespace}", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:blank:]", status);
        TEST_ASSERT_SUCCESS(status);
        UnicodeSet s2(u"[\\p{Whitespace}-[\\u000a\\u000B\\u000c\\u000d\\u0085\\p{LineSeparator}\\p{ParagraphSeparator}]]",
            status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:cntrl:]", status);
        TEST_ASSERT_SUCCESS(status);
        UnicodeSet s2(u"\\p{Control}", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:graph:]", status);
        TEST_ASSERT_SUCCESS(status);
        UnicodeSet s2(u"[^\\p{Whitespace}\\p{Control}\\p{Surrogate}\\p{Unassigned}]", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
    {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeSet s1("[:print:]", status);
        TEST_ASSERT_SUCCESS(status);
        UnicodeSet s2(u"[[:graph:][:blank:]-[\\p{Control}]]", status);
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(s1==s2);
    }
}
/**
 * Test cloning of UnicodeSet.  For C++, we test the copy constructor.
 */
void UnicodeSetTest::TestClone() {
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeSet s("[abcxyz]", ec);
    UnicodeSet t(s);
    expectContainment(t, "abc", "def");
}

/**
 * Test the indexOf() and charAt() methods.
 */
void UnicodeSetTest::TestIndexOf() {
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeSet set("[a-cx-y3578]", ec);
    if (U_FAILURE(ec)) {
        errln("FAIL: UnicodeSet constructor");
        return;
    }
    for (int32_t i=0; i<set.size(); ++i) {
        UChar32 c = set.charAt(i);
        if (set.indexOf(c) != i) {
            errln("FAIL: charAt(%d) = %X => indexOf() => %d",
                i, c, set.indexOf(c));
        }
    }
    UChar32 c = set.charAt(set.size());
    if (c != -1) {
        errln("FAIL: charAt(<out of range>) = %X", c);
    }
    int32_t j = set.indexOf(u'q');
    if (j != -1) {
        errln(UnicodeString("FAIL: indexOf('q') = ") + j);
    }
}

/**
 * Test closure API.
 */
void UnicodeSetTest::TestCloseOver() {
    static constexpr char CASE[] = {static_cast<char>(USET_CASE_INSENSITIVE)};
    static constexpr char CASE_MAPPINGS[] = {static_cast<char>(USET_ADD_CASE_MAPPINGS)};
    static constexpr char SIMPLE_CASE_INSENSITIVE[] = {static_cast<char>(USET_SIMPLE_CASE_INSENSITIVE)};
    static const char* DATA[] = {
        // selector, input, output
        CASE,
        "[aq\\u00DF{Bc}{bC}{Fi}]",
        "[aAqQ\\u00DF\\u1E9E\\uFB01{ss}{bc}{fi}]",  // U+1E9E LATIN CAPITAL LETTER SHARP S is new in Unicode 5.1

        SIMPLE_CASE_INSENSITIVE,
        "[aq\\u00DF{Bc}{bC}{Fi}]",
        "[aAqQ\\u00DF\\u1E9E{bc}{fi}]",

        CASE,
        "[\\u01F1]", // 'DZ'
        "[\\u01F1\\u01F2\\u01F3]",

        SIMPLE_CASE_INSENSITIVE,
        "[\\u01F1]", // 'DZ'
        "[\\u01F1\\u01F2\\u01F3]",

        CASE,
        "[\\u1FB4]",
        "[\\u1FB4{\\u03AC\\u03B9}]",

        SIMPLE_CASE_INSENSITIVE,
        "[\\u1FB4]",
        "[\\u1FB4]",

        CASE,
        "[{F\\uFB01}]",
        "[\\uFB03{ffi}]",

        CASE, // make sure binary search finds limits
        "[a\\uFF3A]",
        "[aA\\uFF3A\\uFF5A]",

        CASE,
        "[a-z]","[A-Za-z\\u017F\\u212A]",

        SIMPLE_CASE_INSENSITIVE,
        "[a-z]","[A-Za-z\\u017F\\u212A]",

        CASE,
        "[abc]","[A-Ca-c]",
        CASE,
        "[ABC]","[A-Ca-c]",

        CASE, "[i]", "[iI]",

        CASE, "[\\u0130]",          "[\\u0130{i\\u0307}]", // dotted I
        CASE, "[{i\\u0307}]",       "[\\u0130{i\\u0307}]", // i with dot

        CASE, "[\\u0131]",          "[\\u0131]", // dotless i

        CASE, "[\\u0390]",          "[\\u0390\\u1FD3{\\u03B9\\u0308\\u0301}]",

        CASE, "[\\u03c2]",          "[\\u03a3\\u03c2\\u03c3]", // sigmas

        CASE, "[\\u03f2]",          "[\\u03f2\\u03f9]", // lunate sigmas

        CASE, "[\\u03f7]",          "[\\u03f7\\u03f8]",

        CASE, "[\\u1fe3]",          "[\\u03b0\\u1fe3{\\u03c5\\u0308\\u0301}]",

        CASE, "[\\ufb05]",          "[\\ufb05\\ufb06{st}]",
        CASE, "[{st}]",             "[\\ufb05\\ufb06{st}]",

        CASE, "[\\U0001044F]",      "[\\U00010427\\U0001044F]",

        CASE, "[{a\\u02BE}]",       "[\\u1E9A{a\\u02BE}]", // first in sorted table

        CASE, "[{\\u1f7c\\u03b9}]", "[\\u1ff2{\\u1f7c\\u03b9}]", // last in sorted table

#if !UCONFIG_NO_FILE_IO
        CASE_MAPPINGS,
        "[aq\\u00DF{Bc}{bC}{Fi}]",
        "[aAqQ\\u00DF{ss}{Ss}{SS}{Bc}{BC}{bC}{bc}{FI}{Fi}{fi}]",
#endif

        CASE_MAPPINGS,
        "[\\u01F1]", // 'DZ'
        "[\\u01F1\\u01F2\\u01F3]",

        CASE_MAPPINGS,
        "[a-z]",
        "[A-Za-z]",

        nullptr
    };

    UnicodeSet s;
    UnicodeSet t;
    UnicodeString buf;
    for (int32_t i=0; DATA[i]!=nullptr; i+=3) {
        int32_t selector = DATA[i][0];
        UnicodeString pat(DATA[i+1], -1, US_INV);
        UnicodeString exp(DATA[i+2], -1, US_INV);

        UErrorCode ec = U_ZERO_ERROR;
        s.applyPattern(pat, ec);
        s.closeOver(selector);
        t.applyPattern(exp, ec);
        if (U_FAILURE(ec)) {
            errln("FAIL: applyPattern failed");
            continue;
        }
        if (s == t) {
            logln(UnicodeString("Ok: ") + pat + ".closeOver(" + selector + ") => " + exp);
        } else {
            dataerrln(UnicodeString("FAIL: ") + pat + ".closeOver(" + selector + ") => " +
                  s.toPattern(buf, true) + ", expected " + exp);
        }
    }

    // Test the pattern API
    UErrorCode ec = U_ZERO_ERROR;
    s.applyPattern("[abc]", USET_CASE_INSENSITIVE, nullptr, ec);
    if (U_FAILURE(ec)) {
        errln("FAIL: applyPattern failed");
    } else {
        expectContainment(s, "abcABC", "defDEF");
    }
    UnicodeSet v("[^abc]", USET_CASE_INSENSITIVE, nullptr, ec);
    if (U_FAILURE(ec)) {
        errln("FAIL: constructor failed");
    } else {
        expectContainment(v, "defDEF", "abcABC");
    }
    UnicodeSet cm("[abck]", USET_ADD_CASE_MAPPINGS, nullptr, ec);
    if (U_FAILURE(ec)) {
        errln("FAIL: construct w/case mappings failed");
    } else {
        expectContainment(cm, "abckABCK", CharsToUnicodeString("defDEF\\u212A"));
    }
}

namespace {

void addIfAbsent(const std::unordered_multimap<UChar32, UChar32> &closure, UChar32 c, UChar32 t,
                 std::unordered_multimap<UChar32, UChar32> &additions) {
    for (auto it = closure.find(c);; ++it) {
        if (it == closure.end() || it->first != c) {
            // absent
            additions.insert({c, t});
            break;
        } else if (it->second == t) {
            // present
            break;
        }
    }
}

}  // namespace

void UnicodeSetTest::TestCloseOverSimpleCaseFolding() {
    IcuTestErrorCode errorCode(*this, "TestCloseOverSimpleCaseFolding");
    const UnicodeSet *sensitive =
        UnicodeSet::fromUSet(u_getBinaryPropertySet(UCHAR_CASE_SENSITIVE, errorCode));
    if (errorCode.errIfFailureAndReset("u_getBinaryPropertySet(UCHAR_CASE_SENSITIVE) failed")) {
        return;
    }
    // Compute the scf=Simple_Case_Folding closure:
    // For each scf(c)=t, start with mappings c->t and t->c.
    std::unordered_multimap<UChar32, UChar32> closure;
    UnicodeSetIterator iter(*sensitive);
    while (iter.next()) {
        UChar32 c = iter.getCodepoint();
        UChar32 scfChar = u_foldCase(c, U_FOLD_CASE_DEFAULT);
        if (scfChar != c) {
            closure.insert({c, scfChar});
            closure.insert({scfChar, c});
        }
    }
    // Complete the closure: Add mappings of mappings.
    for (;;) {
        std::unordered_multimap<UChar32, UChar32> additions;
        // for each mapping c->t
        for (auto mapping : closure) {
            UChar32 c = mapping.first;
            UChar32 t = mapping.second;
            // enumerate each t->u
            for (auto it = closure.find(t); it != closure.end() && it->first == t; ++it) {
                UChar32 u = it->second;
                if (u != c) {
                    addIfAbsent(closure, c, u, additions);
                    addIfAbsent(closure, u, c, additions);
                }
            }
        }
        if (additions.empty()) {
            break;  // The closure is complete.
        }
        closure.insert(additions.begin(), additions.end());
    }
    // Compare closeOver(USET_SIMPLE_CASE_INSENSITIVE) with an unoptimized implementation.
    // Here we focus on single code points as input.
    // Other examples, including strings, are tested in TestCloseOver().
    int32_t errors = 0;
    iter.reset();
    UnicodeSet set, expected;
    while (iter.next()) {
        UChar32 c = iter.getCodepoint();
        // closeOver()
        set.clear().add(c);
        set.closeOver(USET_SIMPLE_CASE_INSENSITIVE);
        // From-first-principles implementation.
        expected.clear().add(c);
        for (auto it = closure.find(c); it != closure.end() && it->first == c; ++it) {
            expected.add(it->second);
        }
        // compare
        if (!checkEqual(expected, set, "closeOver() vs. test impl")) {
            errln("    c=U+%04X", c);
            if (++errors == 10) {
                break;
            }
        }
    }
}

void UnicodeSetTest::TestCloseOverLargeSets() {
    IcuTestErrorCode errorCode(*this, "TestCloseOverLargeSets");
    // Check that an optimization for large sets does not change the result.

    // Most code points except ones that are boring for case mappings.
    UnicodeSet manyCp(u"[^[:C:][:Ideographic:][:Hang:]]", errorCode);
    // Main Unihan block.
    constexpr UChar32 LARGE_START = 0x4E00;
    constexpr UChar32 LARGE_END = 0x9FFF;

    static constexpr int32_t OPTIONS[] = {
        USET_CASE_INSENSITIVE, USET_ADD_CASE_MAPPINGS, USET_SIMPLE_CASE_INSENSITIVE
    };
    UnicodeSet input, small, large;
    for (int32_t option : OPTIONS) {
        UnicodeSetIterator iter(manyCp);
        while (iter.next()) {
            UChar32 c = iter.getCodepoint();
            input.clear().add(c);
            small = input;
            small.closeOver(option);
            large = input;
            large.add(LARGE_START, LARGE_END);
            large.closeOver(option);
            large.remove(LARGE_START, LARGE_END);
            if (!checkEqual(small, large, "small != large")) {
                errln("    option=%d c=U+%04X", option, c);
                break;
            }
        }
    }
}

void UnicodeSetTest::TestEscapePattern() {
    const char pattern[] =
        "[\\uFEFF \\u200A-\\u200E \\U0001D173-\\U0001D17A \\U000F0000-\\U000FFFFD ]";
    const char exp[] =
        "[\\u200A-\\u200E\\uFEFF\\U0001D173-\\U0001D17A\\U000F0000-\\U000FFFFD]";
    // We test this with two passes; in the second pass we
    // pre-unescape the pattern.  Since U+200E is Pattern_White_Space,
    // this fails -- which is what we expect.
    for (int32_t pass=1; pass<=2; ++pass) {
        UErrorCode ec = U_ZERO_ERROR;
        UnicodeString pat(pattern, -1, US_INV);
        if (pass==2) {
            pat = pat.unescape();
        }
        // Pattern is only good for pass 1
        UBool isPatternValid = (pass==1);

        UnicodeSet set(pat, ec);
        if (U_SUCCESS(ec) != isPatternValid){
            errln(UnicodeString("FAIL: applyPattern(") +
                  escape(pat) + ") => " +
                  u_errorName(ec));
            continue;
        }
        if (U_FAILURE(ec)) {
            continue;
        }
        if (set.contains(u'\u0644')){
            errln(UnicodeString("FAIL: ") + escape(pat) + " contains(U+0664)");
        }

        UnicodeString newpat;
        set.toPattern(newpat, true);
        if (newpat == UnicodeString(exp, -1, US_INV)) {
            logln(escape(pat) + " => " + newpat);
        } else {
            errln(UnicodeString("FAIL: ") + escape(pat) + " => " + newpat);
        }

        for (int32_t i=0; i<set.getRangeCount(); ++i) {
            UnicodeString str("Range ");
            str.append(static_cast<char16_t>(u'0' + i))
                .append(": ")
                .append(set.getRangeStart(i))
                .append(" - ")
                .append(set.getRangeEnd(i));
            str = str + " (" + set.getRangeStart(i) + " - " +
                set.getRangeEnd(i) + ")";
            if (set.getRangeStart(i) < 0) {
                errln(UnicodeString("FAIL: ") + escape(str));
            } else {
                logln(escape(str));
            }
        }
    }
}

void UnicodeSetTest::expectRange(const UnicodeString& label,
                                 const UnicodeSet& set,
                                 UChar32 start, UChar32 end) {
    UnicodeSet exp(start, end);
    UnicodeString pat;
    if (set == exp) {
        logln(label + " => " + set.toPattern(pat, true));
    } else {
        UnicodeString xpat;
        errln(UnicodeString("FAIL: ") + label + " => " +
              set.toPattern(pat, true) +
              ", expected " + exp.toPattern(xpat, true));
    }
}

void UnicodeSetTest::TestInvalidCodePoint() {

    const UChar32 DATA[] = {
        // Test range             Expected range
        0, 0x10FFFF,              0, 0x10FFFF,
        static_cast<UChar32>(-1), 8, 0, 8,
        8, 0x110000,              8, 0x10FFFF
    };
    const int32_t DATA_LENGTH = UPRV_LENGTHOF(DATA);

    UnicodeString pat;
    int32_t i;

    for (i=0; i<DATA_LENGTH; i+=4) {
        UChar32 start  = DATA[i];
        UChar32 end    = DATA[i+1];
        UChar32 xstart = DATA[i+2];
        UChar32 xend   = DATA[i+3];

        // Try various API using the test code points

        UnicodeSet set(start, end);
        expectRange(UnicodeString("ct(") + start + "," + end + ")",
                    set, xstart, xend);
        
        set.clear();
        set.set(start, end);
        expectRange(UnicodeString("set(") + start + "," + end + ")",
                    set, xstart, xend);
        
        UBool b = set.contains(start);
        b = set.contains(start, end);
        b = set.containsNone(start, end);
        b = set.containsSome(start, end);
        (void)b;   // Suppress set but not used warning.

        /*int32_t index = set.indexOf(start);*/
        
        set.clear();
        set.add(start);
        set.add(start, end);
        expectRange(UnicodeString("add(") + start + "," + end + ")",
                    set, xstart, xend);

        set.set(0, 0x10FFFF);
        set.retain(start, end);
        expectRange(UnicodeString("retain(") + start + "," + end + ")",
                    set, xstart, xend);
        set.retain(start);

        set.set(0, 0x10FFFF);
        set.remove(start);
        set.remove(start, end);
        set.complement();
        expectRange(UnicodeString("!remove(") + start + "," + end + ")",
                    set, xstart, xend);

        set.set(0, 0x10FFFF);
        set.complement(start, end);
        set.complement();
        expectRange(UnicodeString("!complement(") + start + "," + end + ")",
                    set, xstart, xend);
        set.complement(start);
    }

    const UChar32 DATA2[] = {
        0,
        0x10FFFF,
        static_cast<UChar32>(-1),
        0x110000
    };
    const int32_t DATA2_LENGTH = UPRV_LENGTHOF(DATA2);

    for (i=0; i<DATA2_LENGTH; ++i) {
        UChar32 c = DATA2[i], end = 0x10FFFF;
        UBool valid = (c >= 0 && c <= 0x10FFFF);

        UnicodeSet set(0, 0x10FFFF);

        // For single-codepoint contains, invalid codepoints are NOT contained
        UBool b = set.contains(c);
        if (b == valid) {
            logln(UnicodeString("[\\u0000-\\U0010FFFF].contains(") + c +
                  ") = " + b);
        } else {
            errln(UnicodeString("FAIL: [\\u0000-\\U0010FFFF].contains(") + c +
                  ") = " + b);
        }

        // For codepoint range contains, containsNone, and containsSome,
        // invalid or empty (start > end) ranges have UNDEFINED behavior.
        b = set.contains(c, end);
        logln(UnicodeString("* [\\u0000-\\U0010FFFF].contains(") + c +
              "," + end + ") = " + b);

        b = set.containsNone(c, end);
        logln(UnicodeString("* [\\u0000-\\U0010FFFF].containsNone(") + c +
              "," + end + ") = " + b);

        b = set.containsSome(c, end);
        logln(UnicodeString("* [\\u0000-\\U0010FFFF].containsSome(") + c +
              "," + end + ") = " + b);

        int32_t index = set.indexOf(c);
        if ((index >= 0) == valid) {
            logln(UnicodeString("[\\u0000-\\U0010FFFF].indexOf(") + c +
                  ") = " + index);
        } else {
            errln(UnicodeString("FAIL: [\\u0000-\\U0010FFFF].indexOf(") + c +
                  ") = " + index);
        }
    }
}

// Used by TestSymbolTable
class TokenSymbolTable : public SymbolTable {
public:
    Hashtable contents;

    TokenSymbolTable(UErrorCode& ec) : contents(false, ec) {
        contents.setValueDeleter(uprv_deleteUObject);
    }

    ~TokenSymbolTable() {}

    /**
     * (Non-SymbolTable API) Add the given variable and value to
     * the table.  Variable should NOT contain leading '$'.
     */
    void add(const UnicodeString& var, const UnicodeString& value,
             UErrorCode& ec) {
        if (U_SUCCESS(ec)) {
            contents.put(var, new UnicodeString(value), ec);
        }
    }

    /**
     * SymbolTable API
     */
    virtual const UnicodeString* lookup(const UnicodeString& s) const override {
        return static_cast<const UnicodeString*>(contents.get(s));
    }

    /**
     * SymbolTable API
     */
    virtual const UnicodeFunctor* lookupMatcher(UChar32 /*ch*/) const override {
        return nullptr;
    }

    /**
     * SymbolTable API
     */
    virtual UnicodeString parseReference(const UnicodeString& text,
                                         ParsePosition& pos, int32_t limit) const override {
        int32_t start = pos.getIndex();
        int32_t i = start;
        UnicodeString result;
        while (i < limit) {
            char16_t c = text.charAt(i);
            if ((i==start && !u_isIDStart(c)) || !u_isIDPart(c)) {
                break;
            }
            ++i;
        }
        if (i == start) { // No valid name chars
            return result; // Indicate failure with empty string
        }
        pos.setIndex(i);
        text.extractBetween(start, i, result);
        return result;
    }
};

void UnicodeSetTest::TestSymbolTable() {
    // Multiple test cases can be set up here.  Each test case
    // is terminated by null:
    // var, value, var, value,..., input pat., exp. output pat., null
    const char* DATA[] = {
        "us", "a-z", "[0-1$us]", "[0-1a-z]", nullptr,
        "us", "[a-z]", "[0-1$us]", "[0-1[a-z]]", nullptr,
        "us", "\\[a\\-z\\]", "[0-1$us]", "[-01\\[\\]az]", nullptr,
        nullptr
    };

    for (int32_t i=0; DATA[i]!=nullptr; ++i) {
        UErrorCode ec = U_ZERO_ERROR;
        TokenSymbolTable sym(ec);
        if (U_FAILURE(ec)) {
            errln("FAIL: couldn't construct TokenSymbolTable");
            continue;
        }

        // Set up variables
        while (DATA[i+2] != nullptr) {
            sym.add(UnicodeString(DATA[i], -1, US_INV), UnicodeString(DATA[i+1], -1, US_INV), ec);
            if (U_FAILURE(ec)) {
                errln("FAIL: couldn't add to TokenSymbolTable");
                continue;
            }
            i += 2;
        }

        // Input pattern and expected output pattern
        UnicodeString inpat = UnicodeString(DATA[i], -1, US_INV), exppat = UnicodeString(DATA[i+1], -1, US_INV);
        i += 2;

        ParsePosition pos(0);
        UnicodeSet us(inpat, pos, USET_IGNORE_SPACE, &sym, ec);
        if (U_FAILURE(ec)) {
            errln("FAIL: couldn't construct UnicodeSet");
            continue;
        }

        // results
        if (pos.getIndex() != inpat.length()) {
            errln(UnicodeString("Failed to read to end of string \"")
                  + inpat + "\": read to "
                  + pos.getIndex() + ", length is "
                  + inpat.length());
        }

        UnicodeSet us2(exppat, ec);
        if (U_FAILURE(ec)) {
            errln("FAIL: couldn't construct expected UnicodeSet");
            continue;
        }
        
        UnicodeString a, b;
        if (us != us2) {
            errln(UnicodeString("Failed, got ") + us.toPattern(a, true) +
                  ", expected " + us2.toPattern(b, true));
        } else {
            logln(UnicodeString("Ok, got ") + us.toPattern(a, true));
        }
    }
}

void UnicodeSetTest::TestSurrogate() {
    const char* DATA[] = {
        // These should all behave identically
        "[abc\\uD800\\uDC00]",
        // "[abc\uD800\uDC00]", // Can't do this on C -- only Java
        "[abc\\U00010000]",
        nullptr
    };
    for (int i = 0; DATA[i] != nullptr; ++i) {
        UErrorCode ec = U_ZERO_ERROR;
        logln(UnicodeString("Test pattern ") + i + " :" + UnicodeString(DATA[i], -1, US_INV));
        UnicodeString str = UnicodeString(DATA[i], -1, US_INV);
        UnicodeSet set(str, ec);
        if (U_FAILURE(ec)) {
            errln("FAIL: UnicodeSet constructor");
            continue;
        }
        expectContainment(set,
                          CharsToUnicodeString("abc\\U00010000"),
                          CharsToUnicodeString("\\uD800;\\uDC00")); // split apart surrogate-pair
        if (set.size() != 4) {
            errln(UnicodeString("FAIL: ") + UnicodeString(DATA[i], -1, US_INV) + ".size() == " +
                  set.size() + ", expected 4");
        }

        {
          UErrorCode subErr = U_ZERO_ERROR;
          checkRoundTrip(set);
          checkSerializeRoundTrip(set, subErr);
        }
    }
}

void UnicodeSetTest::TestExhaustive() {
    // exhaustive tests. Simulate UnicodeSets with integers.
    // That gives us very solid tests (except for large memory tests).

    int32_t limit = 128;

    UnicodeSet x, y, z, aa;

    for (int32_t i = 0; i < limit; ++i) {
        bitsToSet(i, x);
        logln(UnicodeString("Testing ") + i + ", " + x);
        _testComplement(i, x, y);

        UnicodeSet &toTest = bitsToSet(i, aa);

        // AS LONG AS WE ARE HERE, check roundtrip
        checkRoundTrip(toTest);
        UErrorCode ec = U_ZERO_ERROR;
        checkSerializeRoundTrip(toTest, ec);

        for (int32_t j = 0; j < limit; ++j) {
            _testAdd(i,j,  x,y,z);
            _testXor(i,j,  x,y,z);
            _testRetain(i,j,  x,y,z);
            _testRemove(i,j,  x,y,z);
        }
    }
}

void UnicodeSetTest::_testComplement(int32_t a, UnicodeSet& x, UnicodeSet& z) {
    bitsToSet(a, x);
    z = x;
    z.complement();
    int32_t c = setToBits(z);
    if (c != (~a)) {
        errln(UnicodeString("FAILED: add: ~") + x + " != " + z);
        errln(UnicodeString("FAILED: add: ~") + a + " != " + c);
    }
    checkCanonicalRep(z, UnicodeString("complement ") + a);
}

void UnicodeSetTest::_testAdd(int32_t a, int32_t b, UnicodeSet& x, UnicodeSet& y, UnicodeSet& z) {
    bitsToSet(a, x);
    bitsToSet(b, y);
    z = x;
    z.addAll(y);
    int32_t c = setToBits(z);
    if (c != (a | b)) {
        errln(UnicodeString("FAILED: add: ") + x + " | " + y + " != " + z);
        errln(UnicodeString("FAILED: add: ") + a + " | " + b + " != " + c);
    }
    checkCanonicalRep(z, UnicodeString("add ") + a + "," + b);
}

void UnicodeSetTest::_testRetain(int32_t a, int32_t b, UnicodeSet& x, UnicodeSet& y, UnicodeSet& z) {
    bitsToSet(a, x);
    bitsToSet(b, y);
    z = x;
    z.retainAll(y);
    int32_t c = setToBits(z);
    if (c != (a & b)) {
        errln(UnicodeString("FAILED: retain: ") + x + " & " + y + " != " + z);
        errln(UnicodeString("FAILED: retain: ") + a + " & " + b + " != " + c);
    }
    checkCanonicalRep(z, UnicodeString("retain ") + a + "," + b);
}

void UnicodeSetTest::_testRemove(int32_t a, int32_t b, UnicodeSet& x, UnicodeSet& y, UnicodeSet& z) {
    bitsToSet(a, x);
    bitsToSet(b, y);
    z = x;
    z.removeAll(y);
    int32_t c = setToBits(z);
    if (c != (a &~ b)) {
        errln(UnicodeString("FAILED: remove: ") + x + " &~ " + y + " != " + z);
        errln(UnicodeString("FAILED: remove: ") + a + " &~ " + b + " != " + c);
    }
    checkCanonicalRep(z, UnicodeString("remove ") + a + "," + b);
}

void UnicodeSetTest::_testXor(int32_t a, int32_t b, UnicodeSet& x, UnicodeSet& y, UnicodeSet& z) {
    bitsToSet(a, x);
    bitsToSet(b, y);
    z = x;
    z.complementAll(y);
    int32_t c = setToBits(z);
    if (c != (a ^ b)) {
        errln(UnicodeString("FAILED: complement: ") + x + " ^ " + y + " != " + z);
        errln(UnicodeString("FAILED: complement: ") + a + " ^ " + b + " != " + c);
    }
    checkCanonicalRep(z, UnicodeString("complement ") + a + "," + b);
}

/**
 * Check that ranges are monotonically increasing and non-
 * overlapping.
 */
void UnicodeSetTest::checkCanonicalRep(const UnicodeSet& set, const UnicodeString& msg) {
    int32_t n = set.getRangeCount();
    if (n < 0) {
        errln(UnicodeString("FAIL result of ") + msg +
              ": range count should be >= 0 but is " +
              n /*+ " for " + set.toPattern())*/);
        return;
    }
    UChar32 last = 0;
    for (int32_t i=0; i<n; ++i) {
        UChar32 start = set.getRangeStart(i);
        UChar32 end = set.getRangeEnd(i);
        if (start > end) {
            errln(UnicodeString("FAIL result of ") + msg +
                  ": range " + (i+1) +
                  " start > end: " + static_cast<int>(start) + ", " + static_cast<int>(end) +
                  " for " + set);
        }
        if (i > 0 && start <= last) {
            errln(UnicodeString("FAIL result of ") + msg +
                  ": range " + (i+1) +
                  " overlaps previous range: " + static_cast<int>(start) + ", " + static_cast<int>(end) +
                  " for " + set);
        }
        last = end;
    }
}

/**
 * Convert a bitmask to a UnicodeSet.
 */
UnicodeSet& UnicodeSetTest::bitsToSet(int32_t a, UnicodeSet& result) {
    result.clear();
    for (UChar32 i = 0; i < 32; ++i) {
        if ((a & (1<<i)) != 0) {
            result.add(i);
        }
    }
    return result;
}

/**
 * Convert a UnicodeSet to a bitmask.  Only the characters
 * U+0000 to U+0020 are represented in the bitmask.
 */
int32_t UnicodeSetTest::setToBits(const UnicodeSet& x) {
    int32_t result = 0;
    for (int32_t i = 0; i < 32; ++i) {
        if (x.contains(static_cast<UChar32>(i))) {
            result |= (1<<i);
        }
    }
    return result;
}

/**
 * Return the representation of an inversion list based UnicodeSet
 * as a pairs list.  Ranges are listed in ascending Unicode order.
 * For example, the set [a-zA-M3] is represented as "33AMaz".
 */
UnicodeString UnicodeSetTest::getPairs(const UnicodeSet& set) {
    UnicodeString pairs;
    for (int32_t i=0; i<set.getRangeCount(); ++i) {
        UChar32 start = set.getRangeStart(i);
        UChar32 end = set.getRangeEnd(i);
        if (end > 0xFFFF) {
            end = 0xFFFF;
            i = set.getRangeCount(); // Should be unnecessary
        }
        pairs.append(static_cast<char16_t>(start)).append(static_cast<char16_t>(end));
    }
    return pairs;
}

/**
 * Basic consistency check for a few items.
 * That the iterator works, and that we can create a pattern and
 * get the same thing back
 */
void UnicodeSetTest::checkRoundTrip(const UnicodeSet& s) {
    {
        UnicodeSet t(s);
        checkEqual(s, t, "copy ct");
    }

    {
        UnicodeSet t(0xabcd, 0xdef0);  // dummy contents should be overwritten
        t = s;
        checkEqual(s, t, "operator=");
    }

    {
        UnicodeSet t;
        copyWithIterator(t, s, false);
        checkEqual(s, t, "iterator roundtrip");
    }

    {
        UnicodeSet t;
        copyWithIterator(t, s, true); // try range
        checkEqual(s, t, "iterator roundtrip");
    }

    {
        UnicodeSet t;
        UnicodeString pat;
        UErrorCode ec = U_ZERO_ERROR;
        s.toPattern(pat, false);
        t.applyPattern(pat, ec);
        if (U_FAILURE(ec)) {
            errln("FAIL: toPattern(escapeUnprintable=false), applyPattern - %s", u_errorName(ec));
            return;
        } else {
            checkEqual(s, t, "toPattern(false)");
        }
    }

    {
        UnicodeSet t;
        UnicodeString pat;
        UErrorCode ec = U_ZERO_ERROR;
        s.toPattern(pat, true);
        t.applyPattern(pat, ec);
        if (U_FAILURE(ec)) {
            errln("FAIL: toPattern(escapeUnprintable=true), applyPattern - %s", u_errorName(ec));
            return;
        } else {
            checkEqual(s, t, "toPattern(true)");
        }
    }
}

void UnicodeSetTest::checkSerializeRoundTrip(const UnicodeSet& t, UErrorCode &status) {
  if(U_FAILURE(status)) return;
  int32_t len = t.serialize(serializeBuffer.getAlias(), serializeBuffer.getCapacity(), status);
  if(status == U_BUFFER_OVERFLOW_ERROR) {
    status = U_ZERO_ERROR;
    serializeBuffer.resize(len);
    len = t.serialize(serializeBuffer.getAlias(), serializeBuffer.getCapacity(), status);
    // let 2nd error stand
  }
  if(U_FAILURE(status)) {
    errln("checkSerializeRoundTrip: error %s serializing buffer\n", u_errorName(status));
    return;
  }
  UnicodeSet deserialized(serializeBuffer.getAlias(), len, UnicodeSet::kSerialized, status);
  if(U_FAILURE(status)) {
    errln("checkSerializeRoundTrip: error %s deserializing buffer: buf %p len %d, original %d\n", u_errorName(status), serializeBuffer.getAlias(), len, t.getRangeCount());
    return;
  }

  checkEqual(t, deserialized, "Set was unequal when deserialized");
}

void UnicodeSetTest::copyWithIterator(UnicodeSet& t, const UnicodeSet& s, UBool withRange) {
    t.clear();
    UnicodeSetIterator it(s);
    if (withRange) {
        while (it.nextRange()) {
            if (it.isString()) {
                t.add(it.getString());
            } else {
                t.add(it.getCodepoint(), it.getCodepointEnd());
            }
        }
    } else {
        while (it.next()) {
            if (it.isString()) {
                t.add(it.getString());
            } else {
                t.add(it.getCodepoint());
            }
        }
    }
}
    
UBool UnicodeSetTest::checkEqual(const UnicodeSet& s, const UnicodeSet& t, const char* message) {
    return checkEqual(*this, s, t, message);
}

UBool UnicodeSetTest::checkEqual(
        IntlTest& intlTest,
        const UnicodeSet& s, const UnicodeSet& t, const char* message) {
    intlTest.assertEquals(UnicodeString("RangeCount: ","") + message, s.getRangeCount(), t.getRangeCount());
    intlTest.assertEquals(UnicodeString("size: ","") + message, s.size(), t.size());
    UnicodeString source; s.toPattern(source, true);
    UnicodeString result; t.toPattern(result, true);
    if (s != t) {
        intlTest.errln(UnicodeString("FAIL: ") + message
              + "\nsource = " + source
              + "\nresult = " + result
              );
        return false;
    } else {
        intlTest.logln(UnicodeString("Ok: ") + message
              + "\nsource = " + source
              + "\nresult = " + result
              );
    }
    return true;
}

void
UnicodeSetTest::expectContainment(const UnicodeString& pat,
                                  const UnicodeString& charsIn,
                                  const UnicodeString& charsOut) {
    UErrorCode ec = U_ZERO_ERROR;
    UnicodeSet set(pat, ec);
    if (U_FAILURE(ec)) {
        dataerrln(UnicodeString("FAIL: pattern \"") +
              pat + "\" => " + u_errorName(ec));
        return;
    }
    expectContainment(set, pat, charsIn, charsOut);
}

void
UnicodeSetTest::expectContainment(const UnicodeSet& set,
                                  const UnicodeString& charsIn,
                                  const UnicodeString& charsOut) {
    UnicodeString pat;
    set.toPattern(pat);
    expectContainment(set, pat, charsIn, charsOut);
}

void
UnicodeSetTest::expectContainment(const UnicodeSet& set,
                                  const UnicodeString& setName,
                                  const UnicodeString& charsIn,
                                  const UnicodeString& charsOut) {
    UnicodeString bad;
    UChar32 c;
    int32_t i;

    for (i=0; i<charsIn.length(); i+=U16_LENGTH(c)) {
        c = charsIn.char32At(i);
        if (!set.contains(c)) {
            bad.append(c);
        }
    }
    if (bad.length() > 0) {
        errln(UnicodeString("Fail: set ") + setName + " does not contain " + prettify(bad) +
              ", expected containment of " + prettify(charsIn));
    } else {
        logln(UnicodeString("Ok: set ") + setName + " contains " + prettify(charsIn));
    }

    bad.truncate(0);
    for (i=0; i<charsOut.length(); i+=U16_LENGTH(c)) {
        c = charsOut.char32At(i);
        if (set.contains(c)) {
            bad.append(c);
        }
    }
    if (bad.length() > 0) {
        errln(UnicodeString("Fail: set ") + setName + " contains " + prettify(bad) +
              ", expected non-containment of " + prettify(charsOut));
    } else {
        logln(UnicodeString("Ok: set ") + setName + " does not contain " + prettify(charsOut));
    }
}

void
UnicodeSetTest::expectPattern(UnicodeSet& set,
                              const UnicodeString& pattern,
                              const UnicodeString& expectedPairs){
    UErrorCode status = U_ZERO_ERROR;
    set.applyPattern(pattern, status);
    if (U_FAILURE(status)) {
        errln(UnicodeString("FAIL: applyPattern(\"") + pattern +
              "\") failed");
        return;
    } else {
        if (getPairs(set) != expectedPairs ) {
            errln(UnicodeString("FAIL: applyPattern(\"") + pattern +
                  "\") => pairs \"" +
                  escape(getPairs(set)) + "\", expected \"" +
                  escape(expectedPairs) + "\"");
        } else {
            logln(UnicodeString("Ok:   applyPattern(\"") + pattern +
                  "\") => pairs \"" +
                  escape(getPairs(set)) + "\"");
        }
    }
    // the result of calling set.toPattern(), which is the string representation of
    // this set(set), is passed to a  UnicodeSet constructor, and tested that it 
    // will produce another set that is equal to this one.
    UnicodeString temppattern;
    set.toPattern(temppattern);
    UnicodeSet *tempset=new UnicodeSet(temppattern, status);
    if (U_FAILURE(status)) {
        errln(UnicodeString("FAIL: applyPattern(\""+ pattern + "\").toPattern() => " + temppattern + " => invalid pattern"));
        return;
    }
    if(*tempset != set || getPairs(*tempset) != getPairs(set)){
        errln(UnicodeString("FAIL: applyPattern(\""+ pattern + "\").toPattern() => " + temppattern + " => pairs \""+ escape(getPairs(*tempset)) + "\", expected pairs \"" +
            escape(getPairs(set)) + "\""));
    } else{
        logln(UnicodeString("Ok:   applyPattern(\""+ pattern + "\").toPattern() => " + temppattern + " => pairs \"" + escape(getPairs(*tempset)) + "\""));
    }

    delete tempset;

}

void
UnicodeSetTest::expectPairs(const UnicodeSet& set, const UnicodeString& expectedPairs) {
    if (getPairs(set) != expectedPairs) {
        errln(UnicodeString("FAIL: Expected pair list \"") +
              escape(expectedPairs) + "\", got \"" +
              escape(getPairs(set)) + "\"");
    }
}

void UnicodeSetTest::expectToPattern(const UnicodeSet& set,
                                     const UnicodeString& expPat,
                                     const char** expStrings) {
    UnicodeString pat;
    set.toPattern(pat, true);
    if (pat == expPat) {
        logln(UnicodeString("Ok:   toPattern() => \"") + pat + "\"");
    } else {
        errln(UnicodeString("FAIL: toPattern() => \"") + pat + "\", expected \"" + expPat + "\"");
        return;
    }
    if (expStrings == nullptr) {
        return;
    }
    UBool in = true;
    for (int32_t i=0; expStrings[i] != nullptr; ++i) {
        if (expStrings[i] == NOT) { // sic; pointer comparison
            in = false;
            continue;
        }
        UnicodeString s = CharsToUnicodeString(expStrings[i]);
        UBool contained = set.contains(s);
        if (contained == in) {
            logln(UnicodeString("Ok: ") + expPat +
                  (contained ? " contains {" : " does not contain {") +
                  escape(expStrings[i]) + "}");
        } else {
            errln(UnicodeString("FAIL: ") + expPat +
                  (contained ? " contains {" : " does not contain {") +
                  escape(expStrings[i]) + "}");
        }
    }
}

static char16_t toHexString(int32_t i) { return static_cast<char16_t>(i + (i < 10 ? u'0' : (u'A' - 10))); }

void
UnicodeSetTest::doAssert(UBool condition, const char *message)
{
    if (!condition) {
        errln(UnicodeString("ERROR : ") + message);
    }
}

UnicodeString
UnicodeSetTest::escape(const UnicodeString& s) {
    UnicodeString buf;
    for (int32_t i=0; i<s.length(); )
    {
        UChar32 c = s.char32At(i);
        if (0x0020 <= c && c <= 0x007F) {
            buf += c;
        } else {
            if (c <= 0xFFFF) {
                buf += u"\\u";
            } else {
                buf += u"\\U";
                buf += toHexString((c & 0xF0000000) >> 28);
                buf += toHexString((c & 0x0F000000) >> 24);
                buf += toHexString((c & 0x00F00000) >> 20);
                buf += toHexString((c & 0x000F0000) >> 16);
            }
            buf += toHexString((c & 0xF000) >> 12);
            buf += toHexString((c & 0x0F00) >> 8);
            buf += toHexString((c & 0x00F0) >> 4);
            buf += toHexString(c & 0x000F);
        }
        i += U16_LENGTH(c);
    }
    return buf;
}

void UnicodeSetTest::TestFreezable() {
    UErrorCode errorCode=U_ZERO_ERROR;
    UnicodeString idPattern=UNICODE_STRING("[:ID_Continue:]", 15);
    UnicodeSet idSet(idPattern, errorCode);
    if(U_FAILURE(errorCode)) {
        dataerrln("FAIL: unable to create UnicodeSet([:ID_Continue:]) - %s", u_errorName(errorCode));
        return;
    }

    UnicodeString wsPattern=UNICODE_STRING("[:White_Space:]", 15);
    UnicodeSet wsSet(wsPattern, errorCode);
    if(U_FAILURE(errorCode)) {
        dataerrln("FAIL: unable to create UnicodeSet([:White_Space:]) - %s", u_errorName(errorCode));
        return;
    }

    idSet.add(idPattern);
    UnicodeSet frozen(idSet);
    frozen.freeze();

    if(idSet.isFrozen() || !frozen.isFrozen()) {
        errln("FAIL: isFrozen() is wrong");
    }
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: a copy-constructed frozen set differs from its original");
    }

    frozen=wsSet;
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: a frozen set was modified by operator=");
    }

    UnicodeSet frozen2(frozen);
    if(frozen2!=frozen || frozen2!=idSet) {
        errln("FAIL: a copied frozen set differs from its frozen original");
    }
    if(!frozen2.isFrozen()) {
        errln("FAIL: copy-constructing a frozen set results in a thawed one");
    }
    UnicodeSet frozen3(5, 55);  // Set to some values to really test assignment below, not copy construction.
    if(frozen3.contains(0, 4) || !frozen3.contains(5, 55) || frozen3.contains(56, 0x10ffff)) {
        errln("FAIL: UnicodeSet(5, 55) failed");
    }
    frozen3=frozen;
    if(!frozen3.isFrozen()) {
        errln("FAIL: copying a frozen set results in a thawed one");
    }

    UnicodeSet *cloned=frozen.clone();
    if(!cloned->isFrozen() || *cloned!=frozen || cloned->containsSome(0xd802, 0xd805)) {
        errln("FAIL: clone() failed");
    }
    cloned->add(0xd802, 0xd805);
    if(cloned->containsSome(0xd802, 0xd805)) {
        errln("FAIL: unable to modify clone");
    }
    delete cloned;

    UnicodeSet *thawed=frozen.cloneAsThawed();
    if(thawed->isFrozen() || *thawed!=frozen || thawed->containsSome(0xd802, 0xd805)) {
        errln("FAIL: cloneAsThawed() failed");
    }
    thawed->add(0xd802, 0xd805);
    if(!thawed->contains(0xd802, 0xd805)) {
        errln("FAIL: unable to modify thawed clone");
    }
    delete thawed;

    frozen.set(5, 55);
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::set() modified a frozen set");
    }

    frozen.clear();
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::clear() modified a frozen set");
    }

    frozen.closeOver(USET_CASE_INSENSITIVE);
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::closeOver() modified a frozen set");
    }

    frozen.compact();
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::compact() modified a frozen set");
    }

    ParsePosition pos;
    frozen.
        applyPattern(wsPattern, errorCode).
        applyPattern(wsPattern, USET_IGNORE_SPACE, nullptr, errorCode).
        applyPattern(wsPattern, pos, USET_IGNORE_SPACE, nullptr, errorCode).
        applyIntPropertyValue(UCHAR_CANONICAL_COMBINING_CLASS, 230, errorCode).
        applyPropertyAlias(u"Assigned", UnicodeString(), errorCode);
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::applyXYZ() modified a frozen set");
    }

    frozen.
        add(0xd800).
        add(0xd802, 0xd805).
        add(wsPattern).
        addAll(idPattern).
        addAll(wsSet);
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::addXYZ() modified a frozen set");
    }

    frozen.
        retain(0x62).
        retain(0x64, 0x69).
        retainAll(wsPattern).
        retainAll(wsSet);
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::retainXYZ() modified a frozen set");
    }

    frozen.
        remove(0x62).
        remove(0x64, 0x69).
        remove(idPattern).
        removeAll(idPattern).
        removeAll(idSet);
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::removeXYZ() modified a frozen set");
    }

    frozen.
        complement().
        complement(0x62).
        complement(0x64, 0x69).
        complement(idPattern).
        complementAll(idPattern).
        complementAll(idSet);
    if(frozen!=idSet || !(frozen==idSet)) {
        errln("FAIL: UnicodeSet::complementXYZ() modified a frozen set");
    }
}

// Test span() etc. -------------------------------------------------------- ***

// Append the UTF-8 version of the string to t and return the appended UTF-8 length.
static int32_t
appendUTF8(const char16_t *s, int32_t length, char *t, int32_t capacity) {
    UErrorCode errorCode=U_ZERO_ERROR;
    int32_t length8=0;
    u_strToUTF8(t, capacity, &length8, s, length, &errorCode);
    if(U_SUCCESS(errorCode)) {
        return length8;
    } else {
        // The string contains an unpaired surrogate.
        // Ignore this string.
        return 0;
    }
}

class UnicodeSetWithStringsIterator;

// Make the strings in a UnicodeSet easily accessible.
class UnicodeSetWithStrings {
public:
    UnicodeSetWithStrings(const UnicodeSet &normalSet) :
            set(normalSet), stringsLength(0), hasSurrogates(false) {
        int32_t size=set.size();
        if(size>0 && set.charAt(size-1)<0) {
            // If a set's last element is not a code point, then it must contain strings.
            // Iterate over the set, skip all code point ranges, and cache the strings.
            // Convert them to UTF-8 for spanUTF8().
            UnicodeSetIterator iter(set);
            const UnicodeString *s;
            char *s8=utf8;
            int32_t length8, utf8Count=0;
            while(iter.nextRange() && stringsLength<UPRV_LENGTHOF(strings)) {
                if(iter.isString()) {
                    // Store the pointer to the set's string element
                    // which we happen to know is a stable pointer.
                    strings[stringsLength]=s=&iter.getString();
                    utf8Count+=
                        utf8Lengths[stringsLength]=length8=
                        appendUTF8(s->getBuffer(), s->length(),
                                   s8, static_cast<int32_t>(sizeof(utf8) - utf8Count));
                    if(length8==0) {
                        hasSurrogates=true;  // Contains unpaired surrogates.
                    }
                    s8+=length8;
                    ++stringsLength;
                }
            }
        }
    }

    const UnicodeSet &getSet() const {
        return set;
    }

    UBool hasStrings() const {
        return stringsLength > 0;
    }

    UBool hasStringsWithSurrogates() const {
        return hasSurrogates;
    }

private:
    friend class UnicodeSetWithStringsIterator;

    const UnicodeSet &set;

    const UnicodeString *strings[20];
    int32_t stringsLength;
    UBool hasSurrogates;

    char utf8[1024];
    int32_t utf8Lengths[20];
};

class UnicodeSetWithStringsIterator {
public:
    UnicodeSetWithStringsIterator(const UnicodeSetWithStrings &set) :
            fSet(set), nextStringIndex(0), nextUTF8Start(0) {
    }

    void reset() {
        nextStringIndex=nextUTF8Start=0;
    }

    const UnicodeString *nextString() {
        if(nextStringIndex<fSet.stringsLength) {
            return fSet.strings[nextStringIndex++];
        } else {
            return nullptr;
        }
    }

    // Do not mix with calls to nextString().
    const char *nextUTF8(int32_t &length) {
        if(nextStringIndex<fSet.stringsLength) {
            const char *s8=fSet.utf8+nextUTF8Start;
            nextUTF8Start+=length=fSet.utf8Lengths[nextStringIndex++];
            return s8;
        } else {
            length=0;
            return nullptr;
        }
    }

private:
    const UnicodeSetWithStrings &fSet;
    int32_t nextStringIndex;
    int32_t nextUTF8Start;
};

// Compare 16-bit Unicode strings (which may be malformed UTF-16)
// at code point boundaries.
// That is, each edge of a match must not be in the middle of a surrogate pair.
static inline UBool
matches16CPB(const char16_t *s, int32_t start, int32_t limit, const UnicodeString &t) {
    s+=start;
    limit-=start;
    int32_t length=t.length();
    return 0==t.compare(s, length) &&
           !(0<start && U16_IS_LEAD(s[-1]) && U16_IS_TRAIL(s[0])) &&
           !(length<limit && U16_IS_LEAD(s[length-1]) && U16_IS_TRAIL(s[length]));
}

// Implement span() with contains() for comparison.
static int32_t containsSpanUTF16(const UnicodeSetWithStrings &set, const char16_t *s, int32_t length,
                                 USetSpanCondition spanCondition) {
    const UnicodeSet &realSet(set.getSet());
    if(!set.hasStrings()) {
        if(spanCondition!=USET_SPAN_NOT_CONTAINED) {
            spanCondition=USET_SPAN_CONTAINED;  // Pin to 0/1 values.
        }

        UChar32 c;
        int32_t start=0, prev;
        while((prev=start)<length) {
            U16_NEXT(s, start, length, c);
            if(realSet.contains(c)!=spanCondition) {
                break;
            }
        }
        return prev;
    } else if(spanCondition==USET_SPAN_NOT_CONTAINED) {
        UnicodeSetWithStringsIterator iter(set);
        UChar32 c;
        int32_t start, next;
        for(start=next=0; start<length;) {
            U16_NEXT(s, next, length, c);
            if(realSet.contains(c)) {
                break;
            }
            const UnicodeString *str;
            iter.reset();
            while((str=iter.nextString())!=nullptr) {
                if(str->length()<=(length-start) && matches16CPB(s, start, length, *str)) {
                    // spanNeedsStrings=true;
                    return start;
                }
            }
            start=next;
        }
        return start;
    } else /* USET_SPAN_CONTAINED or USET_SPAN_SIMPLE */ {
        UnicodeSetWithStringsIterator iter(set);
        UChar32 c;
        int32_t start, next, maxSpanLimit=0;
        for(start=next=0; start<length;) {
            U16_NEXT(s, next, length, c);
            if(!realSet.contains(c)) {
                next=start;  // Do not span this single, not-contained code point.
            }
            const UnicodeString *str;
            iter.reset();
            while((str=iter.nextString())!=nullptr) {
                if(str->length()<=(length-start) && matches16CPB(s, start, length, *str)) {
                    // spanNeedsStrings=true;
                    int32_t matchLimit=start+str->length();
                    if(matchLimit==length) {
                        return length;
                    }
                    if(spanCondition==USET_SPAN_CONTAINED) {
                        // Iterate for the shortest match at each position.
                        // Recurse for each but the shortest match.
                        if(next==start) {
                            next=matchLimit;  // First match from start.
                        } else {
                            if(matchLimit<next) {
                                // Remember shortest match from start for iteration.
                                int32_t temp=next;
                                next=matchLimit;
                                matchLimit=temp;
                            }
                            // Recurse for non-shortest match from start.
                            int32_t spanLength=containsSpanUTF16(set, s+matchLimit, length-matchLimit,
                                                                 USET_SPAN_CONTAINED);
                            if((matchLimit+spanLength)>maxSpanLimit) {
                                maxSpanLimit=matchLimit+spanLength;
                                if(maxSpanLimit==length) {
                                    return length;
                                }
                            }
                        }
                    } else /* spanCondition==USET_SPAN_SIMPLE */ {
                        if(matchLimit>next) {
                            // Remember longest match from start.
                            next=matchLimit;
                        }
                    }
                }
            }
            if(next==start) {
                break;  // No match from start.
            }
            start=next;
        }
        if(start>maxSpanLimit) {
            return start;
        } else {
            return maxSpanLimit;
        }
    }
}

static int32_t containsSpanBackUTF16(const UnicodeSetWithStrings &set, const char16_t *s, int32_t length,
                                     USetSpanCondition spanCondition) {
    if(length==0) {
        return 0;
    }
    const UnicodeSet &realSet(set.getSet());
    if(!set.hasStrings()) {
        if(spanCondition!=USET_SPAN_NOT_CONTAINED) {
            spanCondition=USET_SPAN_CONTAINED;  // Pin to 0/1 values.
        }

        UChar32 c;
        int32_t prev=length;
        do {
            U16_PREV(s, 0, length, c);
            if(realSet.contains(c)!=spanCondition) {
                break;
            }
        } while((prev=length)>0);
        return prev;
    } else if(spanCondition==USET_SPAN_NOT_CONTAINED) {
        UnicodeSetWithStringsIterator iter(set);
        UChar32 c;
        int32_t prev=length, length0=length;
        do {
            U16_PREV(s, 0, length, c);
            if(realSet.contains(c)) {
                break;
            }
            const UnicodeString *str;
            iter.reset();
            while((str=iter.nextString())!=nullptr) {
                if(str->length()<=prev && matches16CPB(s, prev-str->length(), length0, *str)) {
                    // spanNeedsStrings=true;
                    return prev;
                }
            }
        } while((prev=length)>0);
        return prev;
    } else /* USET_SPAN_CONTAINED or USET_SPAN_SIMPLE */ {
        UnicodeSetWithStringsIterator iter(set);
        UChar32 c;
        int32_t prev=length, minSpanStart=length, length0=length;
        do {
            U16_PREV(s, 0, length, c);
            if(!realSet.contains(c)) {
                length=prev;  // Do not span this single, not-contained code point.
            }
            const UnicodeString *str;
            iter.reset();
            while((str=iter.nextString())!=nullptr) {
                if(str->length()<=prev && matches16CPB(s, prev-str->length(), length0, *str)) {
                    // spanNeedsStrings=true;
                    int32_t matchStart=prev-str->length();
                    if(matchStart==0) {
                        return 0;
                    }
                    if(spanCondition==USET_SPAN_CONTAINED) {
                        // Iterate for the shortest match at each position.
                        // Recurse for each but the shortest match.
                        if(length==prev) {
                            length=matchStart;  // First match from prev.
                        } else {
                            if(matchStart>length) {
                                // Remember shortest match from prev for iteration.
                                int32_t temp=length;
                                length=matchStart;
                                matchStart=temp;
                            }
                            // Recurse for non-shortest match from prev.
                            int32_t spanStart=containsSpanBackUTF16(set, s, matchStart,
                                                                    USET_SPAN_CONTAINED);
                            if(spanStart<minSpanStart) {
                                minSpanStart=spanStart;
                                if(minSpanStart==0) {
                                    return 0;
                                }
                            }
                        }
                    } else /* spanCondition==USET_SPAN_SIMPLE */ {
                        if(matchStart<length) {
                            // Remember longest match from prev.
                            length=matchStart;
                        }
                    }
                }
            }
            if(length==prev) {
                break;  // No match from prev.
            }
        } while((prev=length)>0);
        if(prev<minSpanStart) {
            return prev;
        } else {
            return minSpanStart;
        }
    }
}

static int32_t containsSpanUTF8(const UnicodeSetWithStrings &set, const char *s, int32_t length,
                                USetSpanCondition spanCondition) {
    const UnicodeSet &realSet(set.getSet());
    if(!set.hasStrings()) {
        if(spanCondition!=USET_SPAN_NOT_CONTAINED) {
            spanCondition=USET_SPAN_CONTAINED;  // Pin to 0/1 values.
        }

        UChar32 c;
        int32_t start=0, prev;
        while((prev=start)<length) {
            U8_NEXT_OR_FFFD(s, start, length, c);
            if(realSet.contains(c)!=spanCondition) {
                break;
            }
        }
        return prev;
    } else if(spanCondition==USET_SPAN_NOT_CONTAINED) {
        UnicodeSetWithStringsIterator iter(set);
        UChar32 c;
        int32_t start, next;
        for(start=next=0; start<length;) {
            U8_NEXT_OR_FFFD(s, next, length, c);
            if(realSet.contains(c)) {
                break;
            }
            const char *s8;
            int32_t length8;
            iter.reset();
            while((s8=iter.nextUTF8(length8))!=nullptr) {
                if(length8!=0 && length8<=(length-start) && 0==memcmp(s+start, s8, length8)) {
                    // spanNeedsStrings=true;
                    return start;
                }
            }
            start=next;
        }
        return start;
    } else /* USET_SPAN_CONTAINED or USET_SPAN_SIMPLE */ {
        UnicodeSetWithStringsIterator iter(set);
        UChar32 c;
        int32_t start, next, maxSpanLimit=0;
        for(start=next=0; start<length;) {
            U8_NEXT_OR_FFFD(s, next, length, c);
            if(!realSet.contains(c)) {
                next=start;  // Do not span this single, not-contained code point.
            }
            const char *s8;
            int32_t length8;
            iter.reset();
            while((s8=iter.nextUTF8(length8))!=nullptr) {
                if(length8!=0 && length8<=(length-start) && 0==memcmp(s+start, s8, length8)) {
                    // spanNeedsStrings=true;
                    int32_t matchLimit=start+length8;
                    if(matchLimit==length) {
                        return length;
                    }
                    if(spanCondition==USET_SPAN_CONTAINED) {
                        // Iterate for the shortest match at each position.
                        // Recurse for each but the shortest match.
                        if(next==start) {
                            next=matchLimit;  // First match from start.
                        } else {
                            if(matchLimit<next) {
                                // Remember shortest match from start for iteration.
                                int32_t temp=next;
                                next=matchLimit;
                                matchLimit=temp;
                            }
                            // Recurse for non-shortest match from start.
                            int32_t spanLength=containsSpanUTF8(set, s+matchLimit, length-matchLimit,
                                                                USET_SPAN_CONTAINED);
                            if((matchLimit+spanLength)>maxSpanLimit) {
                                maxSpanLimit=matchLimit+spanLength;
                                if(maxSpanLimit==length) {
                                    return length;
                                }
                            }
                        }
                    } else /* spanCondition==USET_SPAN_SIMPLE */ {
                        if(matchLimit>next) {
                            // Remember longest match from start.
                            next=matchLimit;
                        }
                    }
                }
            }
            if(next==start) {
                break;  // No match from start.
            }
            start=next;
        }
        if(start>maxSpanLimit) {
            return start;
        } else {
            return maxSpanLimit;
        }
    }
}

static int32_t containsSpanBackUTF8(const UnicodeSetWithStrings &set, const char *s, int32_t length,
                                    USetSpanCondition spanCondition) {
    if(length==0) {
        return 0;
    }
    const UnicodeSet &realSet(set.getSet());
    if(!set.hasStrings()) {
        if(spanCondition!=USET_SPAN_NOT_CONTAINED) {
            spanCondition=USET_SPAN_CONTAINED;  // Pin to 0/1 values.
        }

        UChar32 c;
        int32_t prev=length;
        do {
            U8_PREV_OR_FFFD(s, 0, length, c);
            if(realSet.contains(c)!=spanCondition) {
                break;
            }
        } while((prev=length)>0);
        return prev;
    } else if(spanCondition==USET_SPAN_NOT_CONTAINED) {
        UnicodeSetWithStringsIterator iter(set);
        UChar32 c;
        int32_t prev=length;
        do {
            U8_PREV_OR_FFFD(s, 0, length, c);
            if(realSet.contains(c)) {
                break;
            }
            const char *s8;
            int32_t length8;
            iter.reset();
            while((s8=iter.nextUTF8(length8))!=nullptr) {
                if(length8!=0 && length8<=prev && 0==memcmp(s+prev-length8, s8, length8)) {
                    // spanNeedsStrings=true;
                    return prev;
                }
            }
        } while((prev=length)>0);
        return prev;
    } else /* USET_SPAN_CONTAINED or USET_SPAN_SIMPLE */ {
        UnicodeSetWithStringsIterator iter(set);
        UChar32 c;
        int32_t prev=length, minSpanStart=length;
        do {
            U8_PREV_OR_FFFD(s, 0, length, c);
            if(!realSet.contains(c)) {
                length=prev;  // Do not span this single, not-contained code point.
            }
            const char *s8;
            int32_t length8;
            iter.reset();
            while((s8=iter.nextUTF8(length8))!=nullptr) {
                if(length8!=0 && length8<=prev && 0==memcmp(s+prev-length8, s8, length8)) {
                    // spanNeedsStrings=true;
                    int32_t matchStart=prev-length8;
                    if(matchStart==0) {
                        return 0;
                    }
                    if(spanCondition==USET_SPAN_CONTAINED) {
                        // Iterate for the shortest match at each position.
                        // Recurse for each but the shortest match.
                        if(length==prev) {
                            length=matchStart;  // First match from prev.
                        } else {
                            if(matchStart>length) {
                                // Remember shortest match from prev for iteration.
                                int32_t temp=length;
                                length=matchStart;
                                matchStart=temp;
                            }
                            // Recurse for non-shortest match from prev.
                            int32_t spanStart=containsSpanBackUTF8(set, s, matchStart,
                                                                   USET_SPAN_CONTAINED);
                            if(spanStart<minSpanStart) {
                                minSpanStart=spanStart;
                                if(minSpanStart==0) {
                                    return 0;
                                }
                            }
                        }
                    } else /* spanCondition==USET_SPAN_SIMPLE */ {
                        if(matchStart<length) {
                            // Remember longest match from prev.
                            length=matchStart;
                        }
                    }
                }
            }
            if(length==prev) {
                break;  // No match from prev.
            }
        } while((prev=length)>0);
        if(prev<minSpanStart) {
            return prev;
        } else {
            return minSpanStart;
        }
    }
}

// spans to be performed and compared
enum {
    SPAN_UTF16          =1,
    SPAN_UTF8           =2,
    SPAN_UTFS           =3,

    SPAN_SET            =4,
    SPAN_COMPLEMENT     =8,
    SPAN_POLARITY       =0xc,

    SPAN_FWD            =0x10,
    SPAN_BACK           =0x20,
    SPAN_DIRS           =0x30,

    SPAN_CONTAINED      =0x100,
    SPAN_SIMPLE         =0x200,
    SPAN_CONDITION      =0x300,

    SPAN_ALL            =0x33f
};

static inline USetSpanCondition invertSpanCondition(USetSpanCondition spanCondition, USetSpanCondition contained) {
    return spanCondition == USET_SPAN_NOT_CONTAINED ? contained : USET_SPAN_NOT_CONTAINED;
}

static inline int32_t slen(const void *s, UBool isUTF16) {
    return isUTF16 ? u_strlen(static_cast<const char16_t*>(s)) : static_cast<int32_t>(strlen(static_cast<const char*>(s)));
}

/*
 * Count spans on a string with the method according to type and set the span limits.
 * The set may be the complement of the original.
 * When using spanBack() and comparing with span(), use a span condition for the first spanBack()
 * according to the expected number of spans.
 * Sets typeName to an empty string if there is no such type.
 * Returns -1 if the span option is filtered out.
 */
static int32_t getSpans(const UnicodeSetWithStrings &set, UBool isComplement,
                        const void *s, int32_t length, UBool isUTF16,
                        uint32_t whichSpans,
                        int type, const char *&typeName,
                        int32_t limits[], int32_t limitsCapacity,
                        int32_t expectCount) {
    const UnicodeSet &realSet(set.getSet());
    int32_t start, count;
    USetSpanCondition spanCondition, firstSpanCondition, contained;
    UBool isForward;

    if(type<0 || 7<type) {
        typeName="";
        return 0;
    }

    static const char *const typeNames16[]={
        "contains", "contains(LM)",
        "span", "span(LM)",
        "containsBack", "containsBack(LM)",
        "spanBack", "spanBack(LM)"
    };

    static const char *const typeNames8[]={
        "containsUTF8", "containsUTF8(LM)",
        "spanUTF8", "spanUTF8(LM)",
        "containsBackUTF8", "containsBackUTF8(LM)", // not implemented
        "spanBackUTF8", "spanBackUTF8(LM)"
    };

    typeName= isUTF16 ? typeNames16[type] : typeNames8[type];

    // filter span options
    if(type<=3) {
        // span forward
        if((whichSpans&SPAN_FWD)==0) {
            return -1;
        }
        isForward=true;
    } else {
        // span backward
        if((whichSpans&SPAN_BACK)==0) {
            return -1;
        }
        isForward=false;
    }
    if((type&1)==0) {
        // use USET_SPAN_CONTAINED
        if((whichSpans&SPAN_CONTAINED)==0) {
            return -1;
        }
        contained=USET_SPAN_CONTAINED;
    } else {
        // use USET_SPAN_SIMPLE
        if((whichSpans&SPAN_SIMPLE)==0) {
            return -1;
        }
        contained=USET_SPAN_SIMPLE;
    }

    // Default first span condition for going forward with an uncomplemented set.
    spanCondition=USET_SPAN_NOT_CONTAINED;
    if(isComplement) {
        spanCondition=invertSpanCondition(spanCondition, contained);
    }

    // First span condition for span(), used to terminate the spanBack() iteration.
    firstSpanCondition=spanCondition;

    // spanBack(): Its initial span condition is span()'s last span condition,
    // which is the opposite of span()'s first span condition
    // if we expect an even number of spans.
    // (The loop inverts spanCondition (expectCount-1) times
    // before the expectCount'th span() call.)
    // If we do not compare forward and backward directions, then we do not have an
    // expectCount and just start with firstSpanCondition.
    if(!isForward && (whichSpans&SPAN_FWD)!=0 && (expectCount&1)==0) {
        spanCondition=invertSpanCondition(spanCondition, contained);
    }

    count=0;
    switch(type) {
    case 0:
    case 1:
        start=0;
        if(length<0) {
            length=slen(s, isUTF16);
        }
        for(;;) {
            start += isUTF16 ? containsSpanUTF16(set, static_cast<const char16_t*>(s) + start, length - start, spanCondition) :
                               containsSpanUTF8(set, static_cast<const char*>(s) + start, length - start, spanCondition);
            if(count<limitsCapacity) {
                limits[count]=start;
            }
            ++count;
            if(start>=length) {
                break;
            }
            spanCondition=invertSpanCondition(spanCondition, contained);
        }
        break;
    case 2:
    case 3:
        start=0;
        for(;;) {
            start += isUTF16 ? realSet.span(static_cast<const char16_t*>(s) + start, length >= 0 ? length - start : length, spanCondition) :
                               realSet.spanUTF8(static_cast<const char*>(s) + start, length >= 0 ? length - start : length, spanCondition);
            if(count<limitsCapacity) {
                limits[count]=start;
            }
            ++count;
            if(length>=0 ? start>=length :
                           isUTF16 ? static_cast<const char16_t*>(s)[start] == 0 :
                                     static_cast<const char*>(s)[start] == 0
            ) {
                break;
            }
            spanCondition=invertSpanCondition(spanCondition, contained);
        }
        break;
    case 4:
    case 5:
        if(length<0) {
            length=slen(s, isUTF16);
        }
        for(;;) {
            ++count;
            if(count<=limitsCapacity) {
                limits[limitsCapacity-count]=length;
            }
            length = isUTF16 ? containsSpanBackUTF16(set, static_cast<const char16_t*>(s), length, spanCondition) :
                               containsSpanBackUTF8(set, static_cast<const char*>(s), length, spanCondition);
            if(length==0 && spanCondition==firstSpanCondition) {
                break;
            }
            spanCondition=invertSpanCondition(spanCondition, contained);
        }
        if(count<limitsCapacity) {
            memmove(limits, limits+(limitsCapacity-count), count*4);
        }
        break;
    case 6:
    case 7:
        for(;;) {
            ++count;
            if(count<=limitsCapacity) {
                limits[limitsCapacity-count]= length >=0 ? length : slen(s, isUTF16);
            }
            // Note: Length<0 is tested only for the first spanBack().
            // If we wanted to keep length<0 for all spanBack()s, we would have to
            // temporarily modify the string by placing a NUL where the previous spanBack() stopped.
            length = isUTF16 ? realSet.spanBack(static_cast<const char16_t*>(s), length, spanCondition) :
                               realSet.spanBackUTF8(static_cast<const char*>(s), length, spanCondition);
            if(length==0 && spanCondition==firstSpanCondition) {
                break;
            }
            spanCondition=invertSpanCondition(spanCondition, contained);
        }
        if(count<limitsCapacity) {
            memmove(limits, limits+(limitsCapacity-count), count*4);
        }
        break;
    default:
        typeName="";
        return -1;
    }

    return count;
}

// sets to be tested; odd index=isComplement
enum {
    SLOW,
    SLOW_NOT,
    FAST,
    FAST_NOT,
    SET_COUNT
};

static const char *const setNames[SET_COUNT]={
    "slow",
    "slow.not",
    "fast",
    "fast.not"
};

/*
 * Verify that we get the same results whether we look at text with contains(),
 * span() or spanBack(), using unfrozen or frozen versions of the set,
 * and using the set or its complement (switching the spanConditions accordingly).
 * The latter verifies that
 *   set.span(spanCondition) == set.complement().span(!spanCondition).
 *
 * The expectLimits[] are either provided by the caller (with expectCount>=0)
 * or returned to the caller (with an input expectCount<0).
 */
void UnicodeSetTest::testSpan(const UnicodeSetWithStrings *sets[4],
                              const void *s, int32_t length, UBool isUTF16,
                              uint32_t whichSpans,
                              int32_t expectLimits[], int32_t &expectCount,
                              const char *testName, int32_t index) {
    int32_t limits[500];
    int32_t limitsCount;
    int i, j;

    const char *typeName;
    int type;

    for(i=0; i<SET_COUNT; ++i) {
        if((i&1)==0) {
            // Even-numbered sets are original, uncomplemented sets.
            if((whichSpans&SPAN_SET)==0) {
                continue;
            }
        } else {
            // Odd-numbered sets are complemented.
            if((whichSpans&SPAN_COMPLEMENT)==0) {
                continue;
            }
        }
        for(type=0;; ++type) {
            limitsCount = getSpans(*sets[i], static_cast<UBool>(i & 1),
                                 s, length, isUTF16,
                                 whichSpans,
                                 type, typeName,
                                 limits, UPRV_LENGTHOF(limits), expectCount);
            if(typeName[0]==0) {
                break; // All types tried.
            }
            if(limitsCount<0) {
                continue; // Span option filtered out.
            }
            if(expectCount<0) {
                expectCount=limitsCount;
                if(limitsCount>UPRV_LENGTHOF(limits)) {
                    errln("FAIL: %s[0x%lx].%s.%s span count=%ld > %ld capacity - too many spans",
                          testName, static_cast<long>(index), setNames[i], typeName, static_cast<long>(limitsCount), static_cast<long>(UPRV_LENGTHOF(limits)));
                    return;
                }
                memcpy(expectLimits, limits, limitsCount*4);
            } else if(limitsCount!=expectCount) {
                errln("FAIL: %s[0x%lx].%s.%s span count=%ld != %ld",
                      testName, static_cast<long>(index), setNames[i], typeName, static_cast<long>(limitsCount), static_cast<long>(expectCount));
            } else {
                for(j=0; j<limitsCount; ++j) {
                    if(limits[j]!=expectLimits[j]) {
                        errln("FAIL: %s[0x%lx].%s.%s span count=%ld limits[%d]=%ld != %ld",
                              testName, static_cast<long>(index), setNames[i], typeName, static_cast<long>(limitsCount),
                              j, static_cast<long>(limits[j]), static_cast<long>(expectLimits[j]));
                        break;
                    }
                }
            }
        }
    }

    // Compare span() with containsAll()/containsNone(),
    // but only if we have expectLimits[] from the uncomplemented set.
    if(isUTF16 && (whichSpans&SPAN_SET)!=0) {
        const char16_t* s16 = static_cast<const char16_t*>(s);
        UnicodeString string;
        int32_t prev=0, limit, length;
        for(i=0; i<expectCount; ++i) {
            limit=expectLimits[i];
            length=limit-prev;
            if(length>0) {
                string.setTo(false, s16+prev, length);  // read-only alias
                if(i&1) {
                    if(!sets[SLOW]->getSet().containsAll(string)) {
                        errln("FAIL: %s[0x%lx].%s.containsAll(%ld..%ld)==false contradicts span()",
                              testName, static_cast<long>(index), setNames[SLOW], static_cast<long>(prev), static_cast<long>(limit));
                        return;
                    }
                    if(!sets[FAST]->getSet().containsAll(string)) {
                        errln("FAIL: %s[0x%lx].%s.containsAll(%ld..%ld)==false contradicts span()",
                              testName, static_cast<long>(index), setNames[FAST], static_cast<long>(prev), static_cast<long>(limit));
                        return;
                    }
                } else {
                    if(!sets[SLOW]->getSet().containsNone(string)) {
                        errln("FAIL: %s[0x%lx].%s.containsNone(%ld..%ld)==false contradicts span()",
                              testName, static_cast<long>(index), setNames[SLOW], static_cast<long>(prev), static_cast<long>(limit));
                        return;
                    }
                    if(!sets[FAST]->getSet().containsNone(string)) {
                        errln("FAIL: %s[0x%lx].%s.containsNone(%ld..%ld)==false contradicts span()",
                              testName, static_cast<long>(index), setNames[FAST], static_cast<long>(prev), static_cast<long>(limit));
                        return;
                    }
                }
            }
            prev=limit;
        }
    }
}

// Specifically test either UTF-16 or UTF-8.
void UnicodeSetTest::testSpan(const UnicodeSetWithStrings *sets[4],
                              const void *s, int32_t length, UBool isUTF16,
                              uint32_t whichSpans,
                              const char *testName, int32_t index) {
    int32_t expectLimits[500];
    int32_t expectCount=-1;
    testSpan(sets, s, length, isUTF16, whichSpans, expectLimits, expectCount, testName, index);
}

UBool stringContainsUnpairedSurrogate(const char16_t *s, int32_t length) {
    char16_t c, c2;

    if(length>=0) {
        while(length>0) {
            c=*s++;
            --length;
            if(0xd800<=c && c<0xe000) {
                if(c>=0xdc00 || length==0 || !U16_IS_TRAIL(c2=*s++)) {
                    return true;
                }
                --length;
            }
        }
    } else {
        while((c=*s++)!=0) {
            if(0xd800<=c && c<0xe000) {
                if(c>=0xdc00 || !U16_IS_TRAIL(c2=*s++)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Test both UTF-16 and UTF-8 versions of span() etc. on the same sets and text,
// unless either UTF is turned off in whichSpans.
// Testing UTF-16 and UTF-8 together requires that surrogate code points
// have the same contains(c) value as U+FFFD.
void UnicodeSetTest::testSpanBothUTFs(const UnicodeSetWithStrings *sets[4],
                                      const char16_t *s16, int32_t length16,
                                      uint32_t whichSpans,
                                      const char *testName, int32_t index) {
    int32_t expectLimits[500];
    int32_t expectCount;

    expectCount=-1;  // Get expectLimits[] from testSpan().

    if((whichSpans&SPAN_UTF16)!=0) {
        testSpan(sets, s16, length16, true, whichSpans, expectLimits, expectCount, testName, index);
    }
    if((whichSpans&SPAN_UTF8)==0) {
        return;
    }

    // Convert s16[] and expectLimits[] to UTF-8.
    uint8_t s8[3000];
    int32_t offsets[3000];

    const char16_t *s16Limit=s16+length16;
    char* t = reinterpret_cast<char*>(s8);
    char *tLimit=t+sizeof(s8);
    int32_t *o=offsets;
    UErrorCode errorCode=U_ZERO_ERROR;

    // Convert with substitution: Turn unpaired surrogates into U+FFFD.
    ucnv_fromUnicode(openUTF8Converter(), &t, tLimit, &s16, s16Limit, o, true, &errorCode);
    if(U_FAILURE(errorCode)) {
        errln("FAIL: %s[0x%lx] ucnv_fromUnicode(to UTF-8) fails with %s",
              testName, static_cast<long>(index), u_errorName(errorCode));
        ucnv_resetFromUnicode(utf8Cnv);
        return;
    }
    int32_t length8 = static_cast<int32_t>(t - reinterpret_cast<char*>(s8));

    // Convert expectLimits[].
    int32_t i, j, expect;
    for(i=j=0; i<expectCount; ++i) {
        expect=expectLimits[i];
        if(expect==length16) {
            expectLimits[i]=length8;
        } else {
            while(offsets[j]<expect) {
                ++j;
            }
            expectLimits[i]=j;
        }
    }

    testSpan(sets, s8, length8, false, whichSpans, expectLimits, expectCount, testName, index);
}

static UChar32 nextCodePoint(UChar32 c) {
    // Skip some large and boring ranges.
    switch(c) {
    case 0x3441:
        return 0x4d7f;
    case 0x5100:
        return 0x9f00;
    case 0xb040:
        return 0xd780;
    case 0xe041:
        return 0xf8fe;
    case 0x10100:
        return 0x20000;
    case 0x20041:
        return 0xe0000;
    case 0xe0101:
        return 0x10fffd;
    default:
        return c+1;
    }
}

// Verify that all implementations represent the same set.
void UnicodeSetTest::testSpanContents(const UnicodeSetWithStrings *sets[4], uint32_t whichSpans, const char *testName) {
    // contains(U+FFFD) is inconsistent with contains(some surrogates),
    // or the set contains strings with unpaired surrogates which don't translate to valid UTF-8:
    // Skip the UTF-8 part of the test - if the string contains surrogates -
    // because it is likely to produce a different result.
    UBool inconsistentSurrogates=
            (!(sets[0]->getSet().contains(0xfffd) ?
               sets[0]->getSet().contains(0xd800, 0xdfff) :
               sets[0]->getSet().containsNone(0xd800, 0xdfff)) ||
             sets[0]->hasStringsWithSurrogates());

    char16_t s[1000];
    int32_t length=0;
    uint32_t localWhichSpans;

    UChar32 c, first;
    for(first=c=0;; c=nextCodePoint(c)) {
        if(c>0x10ffff || length>(UPRV_LENGTHOF(s)-U16_MAX_LENGTH)) {
            localWhichSpans=whichSpans;
            if(stringContainsUnpairedSurrogate(s, length) && inconsistentSurrogates) {
                localWhichSpans&=~SPAN_UTF8;
            }
            testSpanBothUTFs(sets, s, length, localWhichSpans, testName, first);
            if(c>0x10ffff) {
                break;
            }
            length=0;
            first=c;
        }
        U16_APPEND_UNSAFE(s, length, c);
    }
}

// Test with a particular, interesting string.
// Specify length and try NUL-termination.
void UnicodeSetTest::testSpanUTF16String(const UnicodeSetWithStrings *sets[4], uint32_t whichSpans, const char *testName) {
    static const char16_t s[]={
        0x61, 0x62, 0x20,                       // Latin, space
        0x3b1, 0x3b2, 0x3b3,                    // Greek
        0xd900,                                 // lead surrogate
        0x3000, 0x30ab, 0x30ad,                 // wide space, Katakana
        0xdc05,                                 // trail surrogate
        0xa0, 0xac00, 0xd7a3,                   // nbsp, Hangul
        0xd900, 0xdc05,                         // unassigned supplementary
        0xd840, 0xdfff, 0xd860, 0xdffe,         // Han supplementary
        0xd7a4, 0xdc05, 0xd900, 0x2028,         // unassigned, surrogates in wrong order, LS
        0                                       // NUL
    };

    if((whichSpans&SPAN_UTF16)==0) {
        return;
    }
    testSpan(sets, s, -1, true, (whichSpans&~SPAN_UTF8), testName, 0);
    testSpan(sets, s, UPRV_LENGTHOF(s)-1, true, (whichSpans&~SPAN_UTF8), testName, 1);
}

void UnicodeSetTest::testSpanUTF8String(const UnicodeSetWithStrings *sets[4], uint32_t whichSpans, const char *testName) {
    static const char s[]={
        "abc"                                   // Latin

        /* trail byte in lead position */
        "\x80"

        " "                                     // space

        /* truncated multi-byte sequences */
        "\xd0"
        "\xe0"
        "\xe1"
        "\xed"
        "\xee"
        "\xf0"
        "\xf1"
        "\xf4"
        "\xf8"
        "\xfc"

        "\xCE\xB1\xCE\xB2\xCE\xB3"              // Greek

        /* trail byte in lead position */
        "\x80"

        "\xe0\x80"
        "\xe0\xa0"
        "\xe1\x80"
        "\xed\x80"
        "\xed\xa0"
        "\xee\x80"
        "\xf0\x80"
        "\xf0\x90"
        "\xf1\x80"
        "\xf4\x80"
        "\xf4\x90"
        "\xf8\x80"
        "\xfc\x80"

        "\xE3\x80\x80\xE3\x82\xAB\xE3\x82\xAD"  // wide space, Katakana

        /* trail byte in lead position */
        "\x80"

        "\xf0\x80\x80"
        "\xf0\x90\x80"
        "\xf1\x80\x80"
        "\xf4\x80\x80"
        "\xf4\x90\x80"
        "\xf8\x80\x80"
        "\xfc\x80\x80"

        "\xC2\xA0\xEA\xB0\x80\xED\x9E\xA3"      // nbsp, Hangul

        /* trail byte in lead position */
        "\x80"

        "\xf8\x80\x80\x80"
        "\xfc\x80\x80\x80"

        "\xF1\x90\x80\x85"                      // unassigned supplementary

        /* trail byte in lead position */
        "\x80"

        "\xfc\x80\x80\x80\x80"

        "\xF0\xA0\x8F\xBF\xF0\xA8\x8F\xBE"      // Han supplementary

        /* trail byte in lead position */
        "\x80"

        /* complete sequences but non-shortest forms or out of range etc. */
        "\xc0\x80"
        "\xe0\x80\x80"
        "\xed\xa0\x80"
        "\xf0\x80\x80\x80"
        "\xf4\x90\x80\x80"
        "\xf8\x80\x80\x80\x80"
        "\xfc\x80\x80\x80\x80\x80"
        "\xfe"
        "\xff"

        /* trail byte in lead position */
        "\x80"

        "\xED\x9E\xA4\xE2\x80\xA8"              // unassigned, LS, NUL-terminated
    };

    if((whichSpans&SPAN_UTF8)==0) {
        return;
    }
    testSpan(sets, s, -1, false, (whichSpans&~SPAN_UTF16), testName, 0);
    testSpan(sets, s, UPRV_LENGTHOF(s)-1, false, (whichSpans&~SPAN_UTF16), testName, 1);
}

// Take a set of span options and multiply them so that
// each portion only has one of the options a, b and c.
// If b==0, then the set of options is just modified with mask and a.
// If b!=0 and c==0, then the set of options is just modified with mask, a and b.
static int32_t
addAlternative(uint32_t whichSpans[], int32_t whichSpansCount,
               uint32_t mask, uint32_t a, uint32_t b, uint32_t c) {
    uint32_t s;
    int32_t i;

    for(i=0; i<whichSpansCount; ++i) {
        s=whichSpans[i]&mask;
        whichSpans[i]=s|a;
        if(b!=0) {
            whichSpans[whichSpansCount+i]=s|b;
            if(c!=0) {
                whichSpans[2*whichSpansCount+i]=s|c;
            }
        }
    }
    return b==0 ? whichSpansCount : c==0 ? 2*whichSpansCount : 3*whichSpansCount;
}

#define _63_a "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
#define _64_a "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
#define _63_b "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
#define _64_b "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"

void UnicodeSetTest::TestSpan() {
    // "[...]" is a UnicodeSet pattern.
    // "*" performs tests on all Unicode code points and on a selection of
    //   malformed UTF-8/16 strings.
    // "-options" limits the scope of testing for the current set.
    //   By default, the test verifies that equivalent boundaries are found
    //   for UTF-16 and UTF-8, going forward and backward,
    //   alternating USET_SPAN_NOT_CONTAINED with
    //   either USET_SPAN_CONTAINED or USET_SPAN_SIMPLE.
    //   Single-character options:
    //     8 -- UTF-16 and UTF-8 boundaries may differ.
    //          Cause: contains(U+FFFD) is inconsistent with contains(some surrogates),
    //          or the set contains strings with unpaired surrogates
    //          which do not translate to valid UTF-8.
    //     c -- set.span() and set.complement().span() boundaries may differ.
    //          Cause: Set strings are not complemented.
    //     b -- span() and spanBack() boundaries may differ.
    //          Cause: Strings in the set overlap, and spanBack(USET_SPAN_CONTAINED)
    //          and spanBack(USET_SPAN_SIMPLE) are defined to
    //          match with non-overlapping substrings.
    //          For example, with a set containing "ab" and "ba",
    //          span() of "aba" yields boundaries { 0, 2, 3 }
    //          because the initial "ab" matches from 0 to 2,
    //          while spanBack() yields boundaries { 0, 1, 3 }
    //          because the final "ba" matches from 1 to 3.
    //     l -- USET_SPAN_CONTAINED and USET_SPAN_SIMPLE boundaries may differ.
    //          Cause: Strings in the set overlap, and a longer match may
    //          require a sequence including non-longest substrings.
    //          For example, with a set containing "ab", "abc" and "cd",
    //          span(contained) of "abcd" spans the entire string
    //          but span(longest match) only spans the first 3 characters.
    //   Each "-options" first resets all options and then applies the specified options.
    //   A "-" without options resets the options.
    //   The options are also reset for each new set.
    // Other strings will be spanned.
    static const char *const testdata[]={
        "[:ID_Continue:]",
        "*",
        "[:White_Space:]",
        "*",
        "[]",
        "*",
        "[\\u0000-\\U0010FFFF]",
        "*",
        "[\\u0000\\u0080\\u0800\\U00010000]",
        "*",
        "[\\u007F\\u07FF\\uFFFF\\U0010FFFF]",
        "*",
        "[[[:ID_Continue:]-[\\u30ab\\u30ad]]{\\u3000\\u30ab}{\\u3000\\u30ab\\u30ad}]",
        "-c",
        "*",
        "[[[:ID_Continue:]-[\\u30ab\\u30ad]]{\\u30ab\\u30ad}{\\u3000\\u30ab\\u30ad}]",
        "-c",
        "*",

        // Overlapping strings cause overlapping attempts to match.
        "[x{xy}{xya}{axy}{ax}]",
        "-cl",

        // More repetitions of "xya" would take too long with the recursive
        // reference implementation.
        // containsAll()=false
        // test_string 0x14
        "xx"
        "xyaxyaxyaxya"  // set.complement().span(longest match) will stop here.
        "xx"            // set.complement().span(contained) will stop between the two 'x'es.
        "xyaxyaxyaxya"
        "xx"
        "xyaxyaxyaxya"  // span() ends here.
        "aaa",

        // containsAll()=true
        // test_string 0x15
        "xx"
        "xyaxyaxyaxya"
        "xx"
        "xyaxyaxyaxya"
        "xx"
        "xyaxyaxyaxy",

        "-bc",
        // test_string 0x17
        "byayaxya",  // span() -> { 4, 7, 8 }  spanBack() -> { 5, 8 }
        "-c",
        "byayaxy",   // span() -> { 4, 7 }     complement.span() -> { 7 }
        "byayax",    // span() -> { 4, 6 }     complement.span() -> { 6 }
        "-",
        "byaya",     // span() -> { 5 }
        "byay",      // span() -> { 4 }
        "bya",       // span() -> { 3 }

        // span(longest match) will not span the whole string.
        "[a{ab}{bc}]",
        "-cl",
        // test_string 0x21
        "abc",

        "[a{ab}{abc}{cd}]",
        "-cl",
        "acdabcdabccd",

        // spanBack(longest match) will not span the whole string.
        "[c{ab}{bc}]",
        "-cl",
        "abc",

        "[d{cd}{bcd}{ab}]",
        "-cl",
        "abbcdabcdabd",

        // Test with non-ASCII set strings - test proper handling of surrogate pairs
        // and UTF-8 trail bytes.
        // Copies of above test sets and strings, but transliterated to have
        // different code points with similar trail units.
        // Previous: a      b         c            d
        // Unicode:  042B   30AB      200AB        204AB
        // UTF-16:   042B   30AB      D840 DCAB    D841 DCAB
        // UTF-8:    D0 AB  E3 82 AB  F0 A0 82 AB  F0 A0 92 AB
        "[\\u042B{\\u042B\\u30AB}{\\u042B\\u30AB\\U000200AB}{\\U000200AB\\U000204AB}]",
        "-cl",
        "\\u042B\\U000200AB\\U000204AB\\u042B\\u30AB\\U000200AB\\U000204AB\\u042B\\u30AB\\U000200AB\\U000200AB\\U000204AB",

        "[\\U000204AB{\\U000200AB\\U000204AB}{\\u30AB\\U000200AB\\U000204AB}{\\u042B\\u30AB}]",
        "-cl",
        "\\u042B\\u30AB\\u30AB\\U000200AB\\U000204AB\\u042B\\u30AB\\U000200AB\\U000204AB\\u042B\\u30AB\\U000204AB",

        // Stress bookkeeping and recursion.
        // The following strings are barely doable with the recursive
        // reference implementation.
        // The not-contained character at the end prevents an early exit from the span().
        "[b{bb}]",
        "-c",
        // test_string 0x33
        "bbbbbbbbbbbbbbbbbbbbbbbb-",
        // On complement sets, span() and spanBack() get different results
        // because b is not in the complement set and there is an odd number of b's
        // in the test string.
        "-bc",
        "bbbbbbbbbbbbbbbbbbbbbbbbb-",

        // Test with set strings with an initial or final code point span
        // longer than 254.
        "[a{" _64_a _64_a _64_a _64_a "b}"
          "{a" _64_b _64_b _64_b _64_b "}]",
        "-c",
        _64_a _64_a _64_a _63_a "b",
        _64_a _64_a _64_a _64_a "b",
        _64_a _64_a _64_a _64_a "aaaabbbb",
        "a" _64_b _64_b _64_b _63_b,
        "a" _64_b _64_b _64_b _64_b,
        "aaaabbbb" _64_b _64_b _64_b _64_b,

        // Test with strings containing unpaired surrogates.
        // They are not representable in UTF-8, and a leading trail surrogate
        // and a trailing lead surrogate must not match in the middle of a proper surrogate pair.
        // U+20001 == \\uD840\\uDC01
        // U+20400 == \\uD841\\uDC00
        "[a\\U00020001\\U00020400{ab}{b\\uD840}{\\uDC00a}]",
        "-8cl",
        "aaab\\U00020001ba\\U00020400aba\\uD840ab\\uD840\\U00020000b\\U00020000a\\U00020000\\uDC00a\\uDC00babbb"
    };
    uint32_t whichSpans[96]={ SPAN_ALL };
    int32_t whichSpansCount=1;

    UnicodeSet *sets[SET_COUNT]={ nullptr };
    const UnicodeSetWithStrings *sets_with_str[SET_COUNT]={ nullptr };

    char testName[1024];
    char *testNameLimit=testName;

    int32_t i, j;
    for(i=0; i<UPRV_LENGTHOF(testdata); ++i) {
        const char *s=testdata[i];
        if(s[0]=='[') {
            // Create new test sets from this pattern.
            for(j=0; j<SET_COUNT; ++j) {
                delete sets_with_str[j];
                delete sets[j];
            }
            UErrorCode errorCode=U_ZERO_ERROR;
            sets[SLOW]=new UnicodeSet(UnicodeString(s, -1, US_INV).unescape(), errorCode);
            if(U_FAILURE(errorCode)) {
                dataerrln("FAIL: Unable to create UnicodeSet(%s) - %s", s, u_errorName(errorCode));
                break;
            }
            sets[SLOW_NOT]=new UnicodeSet(*sets[SLOW]);
            sets[SLOW_NOT]->complement();
            // Intermediate set: Test cloning of a frozen set.
            UnicodeSet *fast=new UnicodeSet(*sets[SLOW]);
            fast->freeze();
            sets[FAST]=fast->clone();
            delete fast;
            UnicodeSet *fastNot=new UnicodeSet(*sets[SLOW_NOT]);
            fastNot->freeze();
            sets[FAST_NOT]=fastNot->clone();
            delete fastNot;

            for(j=0; j<SET_COUNT; ++j) {
                sets_with_str[j]=new UnicodeSetWithStrings(*sets[j]);
            }

            strcpy(testName, s);
            testNameLimit=strchr(testName, 0);
            *testNameLimit++=':';
            *testNameLimit=0;

            whichSpans[0]=SPAN_ALL;
            whichSpansCount=1;
        } else if(s[0]=='-') {
            whichSpans[0]=SPAN_ALL;
            whichSpansCount=1;

            while(*++s!=0) {
                switch(*s) {
                case 'c':
                    whichSpansCount=addAlternative(whichSpans, whichSpansCount,
                                                   ~SPAN_POLARITY,
                                                   SPAN_SET,
                                                   SPAN_COMPLEMENT,
                                                   0);
                    break;
                case 'b':
                    whichSpansCount=addAlternative(whichSpans, whichSpansCount,
                                                   ~SPAN_DIRS,
                                                   SPAN_FWD,
                                                   SPAN_BACK,
                                                   0);
                    break;
                case 'l':
                    // test USET_SPAN_CONTAINED FWD & BACK, and separately
                    // USET_SPAN_SIMPLE only FWD, and separately
                    // USET_SPAN_SIMPLE only BACK
                    whichSpansCount=addAlternative(whichSpans, whichSpansCount,
                                                   ~(SPAN_DIRS|SPAN_CONDITION),
                                                   SPAN_DIRS|SPAN_CONTAINED,
                                                   SPAN_FWD|SPAN_SIMPLE,
                                                   SPAN_BACK|SPAN_SIMPLE);
                    break;
                case '8':
                    whichSpansCount=addAlternative(whichSpans, whichSpansCount,
                                                   ~SPAN_UTFS,
                                                   SPAN_UTF16,
                                                   SPAN_UTF8,
                                                   0);
                    break;
                default:
                    errln("FAIL: unrecognized span set option in \"%s\"", testdata[i]);
                    break;
                }
            }
        } else if(0==strcmp(s, "*")) {
            strcpy(testNameLimit, "bad_string");
            for(j=0; j<whichSpansCount; ++j) {
                if(whichSpansCount>1) {
                    snprintf(testNameLimit+10 /* strlen("bad_string") */,
                             sizeof(testName) - (testNameLimit+10-testName),
                            "%%0x%3x",
                            whichSpans[j]);
                }
                testSpanUTF16String(sets_with_str, whichSpans[j], testName);
                testSpanUTF8String(sets_with_str, whichSpans[j], testName);
            }

            strcpy(testNameLimit, "contents");
            for(j=0; j<whichSpansCount; ++j) {
                if(whichSpansCount>1) {
                    snprintf(testNameLimit+8 /* strlen("contents") */,
                            sizeof(testName) - (testNameLimit+8-testName),
                            "%%0x%3x",
                            whichSpans[j]);
                }
                testSpanContents(sets_with_str, whichSpans[j], testName);
            }
        } else {
            UnicodeString string=UnicodeString(s, -1, US_INV).unescape();
            strcpy(testNameLimit, "test_string");
            for(j=0; j<whichSpansCount; ++j) {
                if(whichSpansCount>1) {
                    snprintf(testNameLimit+11 /* strlen("test_string") */,
                            sizeof(testName) - (testNameLimit+11-testName),
                            "%%0x%3x",
                            whichSpans[j]);
                }
                testSpanBothUTFs(sets_with_str, string.getBuffer(), string.length(), whichSpans[j], testName, i);
            }
        }
    }
    for(j=0; j<SET_COUNT; ++j) {
        delete sets_with_str[j];
        delete sets[j];
    }
}

// Test select patterns and strings, and test USET_SPAN_SIMPLE.
void UnicodeSetTest::TestStringSpan() {
    static const char *pattern="[x{xy}{xya}{axy}{ax}]";
    static const char *const string=
        "xx"
        "xyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxya"
        "xx"
        "xyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxya"
        "xx"
        "xyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxyaxy"
        "aaaa";

    UErrorCode errorCode=U_ZERO_ERROR;
    UnicodeString pattern16=UnicodeString(pattern, -1, US_INV);
    UnicodeSet set(pattern16, errorCode);
    if(U_FAILURE(errorCode)) {
        errln("FAIL: Unable to create UnicodeSet(%s) - %s", pattern, u_errorName(errorCode));
        return;
    }

    UnicodeString string16=UnicodeString(string, -1, US_INV).unescape();

    if(set.containsAll(string16)) {
        errln("FAIL: UnicodeSet(%s).containsAll(%s) should be false", pattern, string);
    }

    // Remove trailing "aaaa".
    string16.truncate(string16.length()-4);
    if(!set.containsAll(string16)) {
        errln("FAIL: UnicodeSet(%s).containsAll(%s[:-4]) should be true", pattern, string);
    }

    string16=u"byayaxya";
    const char16_t *s16=string16.getBuffer();
    int32_t length16=string16.length();
    (void)length16;   // Suppress set but not used warning.
    if( set.span(s16, 8, USET_SPAN_NOT_CONTAINED)!=4 ||
        set.span(s16, 7, USET_SPAN_NOT_CONTAINED)!=4 ||
        set.span(s16, 6, USET_SPAN_NOT_CONTAINED)!=4 ||
        set.span(s16, 5, USET_SPAN_NOT_CONTAINED)!=5 ||
        set.span(s16, 4, USET_SPAN_NOT_CONTAINED)!=4 ||
        set.span(s16, 3, USET_SPAN_NOT_CONTAINED)!=3
    ) {
        errln("FAIL: UnicodeSet(%s).span(while not) returns the wrong value", pattern);
    }

    pattern="[a{ab}{abc}{cd}]";
    pattern16=UnicodeString(pattern, -1, US_INV);
    set.applyPattern(pattern16, errorCode);
    if(U_FAILURE(errorCode)) {
        errln("FAIL: Unable to create UnicodeSet(%s) - %s", pattern, u_errorName(errorCode));
        return;
    }
    string16=u"acdabcdabccd";
    s16=string16.getBuffer();
    length16=string16.length();
    if( set.span(s16, 12, USET_SPAN_CONTAINED)!=12 ||
        set.span(s16, 12, USET_SPAN_SIMPLE)!=6 ||
        set.span(s16+7, 5, USET_SPAN_SIMPLE)!=5
    ) {
        errln("FAIL: UnicodeSet(%s).span(while longest match) returns the wrong value", pattern);
    }

    pattern="[d{cd}{bcd}{ab}]";
    pattern16=UnicodeString(pattern, -1, US_INV);
    set.applyPattern(pattern16, errorCode).freeze();
    if(U_FAILURE(errorCode)) {
        errln("FAIL: Unable to create UnicodeSet(%s) - %s", pattern, u_errorName(errorCode));
        return;
    }
    string16=u"abbcdabcdabd";
    s16=string16.getBuffer();
    length16=string16.length();
    if( set.spanBack(s16, 12, USET_SPAN_CONTAINED)!=0 ||
        set.spanBack(s16, 12, USET_SPAN_SIMPLE)!=6 ||
        set.spanBack(s16, 5, USET_SPAN_SIMPLE)!=0
    ) {
        errln("FAIL: UnicodeSet(%s).spanBack(while longest match) returns the wrong value", pattern);
    }
}

void UnicodeSetTest::TestPatternWithSurrogates() {
    IcuTestErrorCode errorCode(*this, "TestPatternWithSurrogates");
    // Regression test for ICU-11891
    UnicodeSet surrogates;
    surrogates.add(0xd000, 0xd82f);  // a range ending with a lead surrogate code point
    surrogates.add(0xd83a);  // a lead surrogate
    surrogates.add(0xdc00, 0xdfff);  // a range of trail surrogates
    UnicodeString pat;
    surrogates.toPattern(pat, false);  // bad if U+D83A is immediately followed by U+DC00
    UnicodeSet s2;
    // was: U_MALFORMED_SET
    // Java: IllegalArgumentException: Error: Invalid range at "[...\U0001E800-\uDFFF|...]"
    s2.applyPattern(pat, errorCode);
    if (errorCode.errIfFailureAndReset("surrogates (1) to/from pattern")) { return; }
    checkEqual(surrogates, s2, "surrogates (1) to/from pattern");

    // create a range of DBFF-DC00, and in the complement form a range of DC01-DC03
    surrogates.add(0xdbff).remove(0xdc01, 0xdc03);
    // add a beyond-surrogates range, up to the last code point
    surrogates.add(0x10affe, 0x10ffff);
    surrogates.toPattern(pat, false);  // bad if U+DBFF is immediately followed by U+DC00
    s2.applyPattern(pat, errorCode);
    if (errorCode.errIfFailureAndReset("surrogates (2) to/from pattern")) { return; }
    checkEqual(surrogates, s2, "surrogates (2) to/from pattern");

    // Test the toPattern() code path when the pattern is shorter in complement form:
    // [^opposite-ranges]
    surrogates.add(0, 0x6789);
    surrogates.toPattern(pat, false);
    s2.applyPattern(pat, errorCode);
    if (errorCode.errIfFailureAndReset("surrogates (3) to/from pattern")) { return; }
    checkEqual(surrogates, s2, "surrogates (3) to/from pattern");

    // Start with a pattern, in case the original pattern is kept but
    // without the extra white space.
    surrogates.applyPattern(u"[\\uD83A \\uDC00-\\uDFFF]", errorCode);
    if (errorCode.errIfFailureAndReset("surrogates from pattern")) { return; }
    surrogates.toPattern(pat, false);
    s2.applyPattern(pat, errorCode);
    if (errorCode.errIfFailureAndReset("surrogates from/to/from pattern")) { return; }
    checkEqual(surrogates, s2, "surrogates from/to/from pattern");
}

void UnicodeSetTest::TestIntOverflow() {
    // This test triggers undefined double->int conversion behavior
    // if the implementation is not careful.
    IcuTestErrorCode errorCode(*this, "TestIntOverflow");
    UnicodeSet set(u"[:ccc=2222222222222222222:]", errorCode);
    assertTrue("[:ccc=int_overflow:] -> empty set", set.isEmpty());
    assertEquals("[:ccc=int_overflow:] -> illegal argument",
                 U_ILLEGAL_ARGUMENT_ERROR, errorCode.reset());
}

void UnicodeSetTest::TestUnusedCcc() {
#if !UCONFIG_NO_NORMALIZATION
    // All numeric ccc values 0..255 are valid, but many are unused.
    IcuTestErrorCode errorCode(*this, "TestUnusedCcc");
    UnicodeSet ccc2(u"[:ccc=2:]", errorCode);
    assertSuccess("[:ccc=2:]", errorCode);
    assertTrue("[:ccc=2:] -> empty set", ccc2.isEmpty());

    UnicodeSet ccc255(u"[:ccc=255:]", errorCode);
    assertSuccess("[:ccc=255:]", errorCode);
    assertTrue("[:ccc=255:] -> empty set", ccc255.isEmpty());

    // Non-integer values and values outside 0..255 are invalid.
    UnicodeSet ccc_1(u"[:ccc=-1:]", errorCode);
    assertEquals("[:ccc=-1:] -> illegal argument",
                 U_ILLEGAL_ARGUMENT_ERROR, errorCode.reset());
    assertTrue("[:ccc=-1:] -> empty set", ccc_1.isEmpty());

    UnicodeSet ccc256(u"[:ccc=256:]", errorCode);
    assertEquals("[:ccc=256:] -> illegal argument",
                 U_ILLEGAL_ARGUMENT_ERROR, errorCode.reset());
    assertTrue("[:ccc=256:] -> empty set", ccc256.isEmpty());

    UnicodeSet ccc1_1(u"[:ccc=1.1:]", errorCode);
    assertEquals("[:ccc=1.1:] -> illegal argument",
                 U_ILLEGAL_ARGUMENT_ERROR, errorCode.reset());
    assertTrue("[:ccc=1.1:] -> empty set", ccc1_1.isEmpty());
#endif
}

void UnicodeSetTest::TestDeepPattern() {
    IcuTestErrorCode errorCode(*this, "TestDeepPattern");
    // Nested ranges are parsed via recursion which can use a lot of stack space.
    // After a reasonable limit, we should get an error.
    constexpr int32_t DEPTH = 20000;
    UnicodeString pattern, suffix;
    for (int32_t i = 0; i < DEPTH; ++i) {
        pattern.append(u"[a", 2);
        suffix.append(']');
    }
    pattern.append(suffix);
    UnicodeSet set(pattern, errorCode);
    assertTrue("[a[a[a...1000s...]]] -> error", errorCode.isFailure());
    errorCode.reset();
}

void UnicodeSetTest::TestEmptyString() {
    IcuTestErrorCode errorCode(*this, "TestEmptyString");
    // Starting with ICU 69, the empty string is allowed in UnicodeSet. ICU-13702
    UnicodeSet set(u"[{}]", errorCode);
    if (!assertSuccess("set from pattern with {}", errorCode)) { return; }
    assertTrue("set from pattern with {}", set.contains(u""));
    assertEquals("set from pattern with {}: size", 1, set.size());
    assertFalse("set from pattern with {}: isEmpty", set.isEmpty());

    // Remove, add back, ...
    assertFalse("remove empty string", set.remove(u"").contains(u""));
    assertEquals("remove empty string: size", 0, set.size());
    assertTrue("remove empty string: isEmpty", set.isEmpty());
    assertTrue("add empty string", set.add(u"").contains(u""));
    // missing API -- assertTrue("retain empty string", set.retain(u"").contains(u""));
    assertFalse("complement-remove empty string", set.complement(u"").contains(u""));
    assertTrue("complement-add empty string", set.complement(u"").contains(u""));

    assertFalse("clear", set.clear().contains(u""));
    assertTrue("add empty string 2", set.add(u"").contains(u""));
    assertFalse("removeAllStrings", set.removeAllStrings().contains(u""));
    assertTrue("add empty string 3", set.add(u"").contains(u""));
    // Note that this leaves the set containing exactly the empty string.

    // strings() access and iteration
    // no C++ equivalent for Java strings() -- assertTrue("strings()", set.strings().contains(u""));
    UnicodeSetIterator sit(set);
    assertTrue("set iterator.next()", sit.next());
    assertTrue("set iterator has empty string", sit.isString() && sit.getString().isEmpty());

    // The empty string is ignored in matching.
    set.add(u'a').add(u'c');
    assertEquals("span", 1, set.span(u"abc", 3, USET_SPAN_SIMPLE));
    assertEquals("spanBack", 2, set.spanBack(u"abc", 3, USET_SPAN_SIMPLE));
    assertTrue("containsNone", set.containsNone(u"def"));
    assertFalse("containsSome", set.containsSome(u"def"));
    set.freeze();
    assertEquals("frozen span", 1, set.span(u"abc", 3, USET_SPAN_SIMPLE));
    assertEquals("frozen spanBack", 2, set.spanBack(u"abc", 3, USET_SPAN_SIMPLE));
    assertTrue("frozen containsNone", set.containsNone(u"def"));
    assertFalse("frozen containsSome", set.containsSome(u"def"));
}

void UnicodeSetTest::assertNext(UnicodeSetIterator &iter, const UnicodeString &expected) {
    assertTrue(expected + ".next()", iter.next());
    assertEquals(expected + ".getString()", expected, iter.getString());
}

void UnicodeSetTest::TestSkipToStrings() {
    IcuTestErrorCode errorCode(*this, "TestSkipToStrings");
    UnicodeSet set(u"[0189{}{ch}]", errorCode);
    UnicodeSetIterator iter(set);
    assertNext(iter.skipToStrings(), u"");
    assertNext(iter, u"ch");
    assertFalse("no next", iter.next());

    iter.reset();
    assertNext(iter, u"0");
    assertNext(iter, u"1");
    assertNext(iter, u"8");
    assertNext(iter, u"9");
    assertNext(iter, u"");
    assertNext(iter, u"ch");
    assertFalse("no next", iter.next());

    iter.reset();
    assertNext(iter, u"0");
    iter.skipToStrings();
    assertNext(iter, u"");
    assertNext(iter, u"ch");
    assertFalse("no next", iter.next());

    iter.reset();
    iter.nextRange();
    assertNext(iter, u"8");
    iter.skipToStrings();
    assertNext(iter, u"");
    assertNext(iter, u"ch");
    assertFalse("no next", iter.next());

    iter.reset();
    iter.nextRange();
    iter.nextRange();
    iter.nextRange();
    iter.skipToStrings();
    assertNext(iter, u"ch");
    assertFalse("no next", iter.next());
}

void UnicodeSetTest::TestPatternCodePointComplement() {
    IcuTestErrorCode errorCode(*this, "TestPatternCodePointComplement");
    // ICU-21524 changes pattern ^ and equivalent functions to perform a "code point complement".
    // [^abc{ch}] = [[:Any:]-[abc{ch}]] which removes all strings.
    {
        UnicodeSet simple(u"[^abc{ch}]", errorCode);
        assertEquals("[^abc{ch}] --> lots of elements", 0x110000 - 3, simple.size());
        assertFalse("[^abc{ch}] --> no strings", simple.hasStrings());
        assertFalse("[^abc{ch}] --> no 'a'", simple.contains(u'a'));
    }

    {
        UnicodeSet notBasic(u"[:^Basic_Emoji:]", errorCode);
        if (errorCode.errDataIfFailureAndReset("[:^Basic_Emoji:]")) {
            return;
        }
        assertTrue("[:^Basic_Emoji:] --> lots of elements", notBasic.size() > 1000);
        assertFalse("[:^Basic_Emoji:] --> no strings", notBasic.hasStrings());
        assertFalse("[:^Basic_Emoji:] --> no bicycle", notBasic.contains(U'🚲'));
    }

    {
        UnicodeSet notBasic(u"[:Basic_Emoji=No:]", errorCode);
        assertTrue("[:Basic_Emoji=No:] --> lots of elements", notBasic.size() > 1000);
        assertFalse("[:Basic_Emoji=No:] --> no strings", notBasic.hasStrings());
        assertFalse("[:Basic_Emoji=No:] --> no bicycle", notBasic.contains(U'🚲'));
    }

    {
        UnicodeSet notBasic;
        notBasic.applyIntPropertyValue(UCHAR_BASIC_EMOJI, 0, errorCode);
        assertTrue("[].applyIntPropertyValue(Basic_Emoji, 0) --> lots of elements",
                notBasic.size() > 1000);
        assertFalse("[].applyIntPropertyValue(Basic_Emoji, 0) --> no strings",
                notBasic.hasStrings());
        assertFalse("[].applyIntPropertyValue(Basic_Emoji, 0) --> no bicycle",
                notBasic.contains(U'🚲'));
    }

    {
        UnicodeSet notBasic;
        notBasic.applyPropertyAlias("Basic_Emoji", "No", errorCode);
        assertTrue("[].applyPropertyAlias(Basic_Emoji, No) --> lots of elements",
                notBasic.size() > 1000);
        assertFalse("[].applyPropertyAlias(Basic_Emoji, No) --> no strings",
                notBasic.hasStrings());
        assertFalse("[].applyPropertyAlias(Basic_Emoji, No) --> no bicycle",
                notBasic.contains(U'🚲'));
    }

    // When there are strings, we must not use the complement for a more compact toPattern().
    {
        UnicodeSet set;
        set.add(0,  u'Y').add(u'b', u'q').add(u'x', 0x10ffff);
        UnicodeString pattern;
        set.toPattern(pattern, true);
        UnicodeSet set2(pattern, errorCode);
        checkEqual(set, set2, "set(with 0 & max, only code points) pattern round-trip");
        assertEquals("set(with 0 & max, only code points).toPattern()", u"[^Z-ar-w]", pattern);

        set.add("ch").add("ss");
        set.toPattern(pattern, true);
        set2 = UnicodeSet(pattern, errorCode);
        checkEqual(set, set2, "set(with 0 & max, with strings) pattern round-trip");
        assertEquals("set(with 0 & max, with strings).toPattern()",
                u"[\\u0000-Yb-qx-\\U0010FFFF{ch}{ss}]", pattern);
    }

    // The complement() API behavior does not change under this ticket.
    {
        UnicodeSet notBasic(u"[:Basic_Emoji:]", errorCode);
        notBasic.complement();
        assertTrue("[:Basic_Emoji:].complement() --> lots of elements", notBasic.size() > 1000);
        assertTrue("[:Basic_Emoji:].complement() --> has strings", notBasic.hasStrings());
        assertTrue("[:Basic_Emoji:].complement().contains(chipmunk+emoji)",
                notBasic.contains(u"🐿\uFE0F"));
        assertFalse("[:Basic_Emoji:].complement() --> no bicycle", notBasic.contains(U'🚲'));
    }
}

void UnicodeSetTest::TestCodePointIterator() {
    IcuTestErrorCode errorCode(*this, "TestCodePointIterator");
    UnicodeSet set(u"[abcçカ🚴]", errorCode);
    UnicodeString result;
    for (UChar32 c : set.codePoints()) {
        // printf("set.codePoint U+%04lx\n", (long)c);
        result.append(u' ').append(c);
    }
    assertEquals(WHERE, u" a b c ç カ 🚴", result);

    // codePoints() returns USetCodePoints for which explicit APIs are tested via USet
    // in a header-only unit test file.
}

void UnicodeSetTest::TestRangeIterator() {
    IcuTestErrorCode errorCode(*this, "TestRangeIterator");
    UnicodeSet set(u"[abcçカ🚴]", errorCode);
    UnicodeString result;
    for (auto [start, end] : set.ranges()) {
        // printf("set.range U+%04lx..U+%04lx\n", (long)start, (long)end);
        result.append(u' ').append(start).append(u'-').append(end);
    }
    assertEquals(WHERE, u" a-c ç-ç カ-カ 🚴-🚴", result);
    result.remove();
    for (auto range : set.ranges()) {
        for (UChar32 c : range) {
            // printf("set.range.c U+%04lx\n", (long)c);
            result.append(u' ').append(c);
        }
        result.append(u" |");
    }
    assertEquals(WHERE, u" a b c | ç | カ | 🚴 |", result);

    // ranges() returns USetRanges for which explicit APIs are tested via USet
    // in a header-only unit test file.
}

void UnicodeSetTest::TestStringIterator() {
    IcuTestErrorCode errorCode(*this, "TestStringIterator");
    UnicodeSet set(u"[abcçカ🚴{}{abc}{de}]", errorCode);
    UnicodeString result;
    for (auto s : set.strings()) {
        // UnicodeString us(s);
        // std::string u8;
        // printf("set.string length %ld \"%s\"\n", (long)s.length(), us.toUTF8String(u8).c_str());
        result.append(u" \"").append(s).append(u'"');
    }
    assertEquals(WHERE, uR"( "" "abc" "de")", result);

    // strings() returns USetStrins for which explicit APIs are tested via USet
    // in a header-only unit test file.
}

void UnicodeSetTest::TestElementIterator() {
    IcuTestErrorCode errorCode(*this, "TestElementIterator");
    UnicodeSet set(u"[abcçカ🚴{}{abc}{de}]", errorCode);
    UnicodeString result;
    for (auto el : set) {
        // UnicodeString us(el);
        // std::string u8;
        // printf("set.element length %ld \"%s\"\n", (long)us.length(), us.toUTF8String(u8).c_str());
        result.append(u" \"").append(el).append(u'"');
    }
    assertEquals(WHERE, uR"( "a" "b" "c" "ç" "カ" "🚴" "" "abc" "de")", result);

    // begin() & end() return USetElementIterator for which explicit APIs are tested via USet
    // in a header-only unit test file.
}
