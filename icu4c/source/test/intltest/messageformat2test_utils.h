// © 2024 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#ifndef _TESTMESSAGEFORMAT2_UTILS
#define _TESTMESSAGEFORMAT2_UTILS

#include "unicode/utypes.h"

#if !UCONFIG_NO_NORMALIZATION

#if !UCONFIG_NO_FORMATTING

#if !UCONFIG_NO_MF2

#include "unicode/locid.h"
#include "unicode/messageformat2_formattable.h"
#include "unicode/messageformat2.h"
#include "intltest.h"
#include "messageformat2_macros.h"
#include "messageformat2_serializer.h"

U_NAMESPACE_BEGIN namespace message2 {

class TestCase : public UMemory {
    private:
    /* const */ UnicodeString testName;
    /* const */ UnicodeString pattern;
    /* const */ Locale locale;
    /* const */ std::map<UnicodeString, Formattable> arguments;
    /* const */ UErrorCode expectedError;
    /* const */ bool arbitraryError = false;
    /* const */ bool expectedNoSyntaxError;
    /* const */ bool hasExpectedOutput;
    /* const */ UnicodeString expected;
    /* const */ bool hasLineNumberAndOffset;
    /* const */ uint32_t lineNumber;
    /* const */ uint32_t offset;
    /* const */ bool ignoreError;

    // Function registry is not owned by the TestCase object
    const MFFunctionRegistry* functionRegistry = nullptr;

    public:
    const UnicodeString& getPattern() const { return pattern; }
    const Locale& getLocale() const { return locale; }
    std::map<UnicodeString, Formattable> getArguments() const { return std::move(arguments); }
    const UnicodeString& getTestName() const { return testName; }
    bool expectSuccess() const {
        return (!ignoreError && U_SUCCESS(expectedError) && !arbitraryError);
    }
    bool expectFailure() const {
        return (!ignoreError && U_FAILURE(expectedError));
    }
    bool expectArbitraryError() const {
        return arbitraryError;
    }
    bool expectNoSyntaxError() const {
        return expectedNoSyntaxError;
    }
    UErrorCode expectedErrorCode() const {
        U_ASSERT(!expectSuccess());
        return expectedError;
    }
    bool lineNumberAndOffsetMatch(uint32_t actualLine, uint32_t actualOffset) const {
        return (!hasLineNumberAndOffset ||
                ((actualLine == lineNumber) && actualOffset == offset));
    }
    bool outputMatches(const UnicodeString& result) const {
        return (!hasExpectedOutput || (expected == result));
    }
    const UnicodeString& expectedOutput() const {
        U_ASSERT(hasExpectedOutput);
        return expected;
    }
    uint32_t getLineNumber() const {
        U_ASSERT(hasLineNumberAndOffset);
        return lineNumber;
    }
    uint32_t getOffset() const {
        U_ASSERT(hasLineNumberAndOffset);
        return offset;
    }
    bool hasCustomRegistry() const { return functionRegistry != nullptr; }
    const MFFunctionRegistry* getCustomRegistry() const {
        U_ASSERT(hasCustomRegistry());
        return functionRegistry;
    }
    TestCase(const TestCase&);
    TestCase& operator=(TestCase&& other) noexcept = default;
    virtual ~TestCase();

    class Builder : public UObject {
        friend class TestCase;

        public:
        Builder& setName(UnicodeString name) { testName = name; return *this; }
        Builder& setPattern(UnicodeString pat) { pattern = pat; return *this; }
        Builder& setArgument(const UnicodeString& k, const UnicodeString& val) {
            arguments[k] = Formattable(val);
            return *this;
        }
        Builder& setArgument(const UnicodeString& k, const Formattable* val, int32_t count) {
            U_ASSERT(val != nullptr);
            arguments[k] = Formattable(val, count);
            return *this;
        }
        Builder& setArgument(const UnicodeString& k, double val) {
            arguments[k] = Formattable(val);
            return *this;
        }
        Builder& setArgument(const UnicodeString& k, int64_t val) {
            arguments[k] = Formattable(val);
            return *this;
        }
        Builder& setDateArgument(const UnicodeString& k, UDate date) {
            // This ignores time zones; the data-driven tests represent date/time values
            // as a datestamp, so this code suffices to handle those.
            // Date/time literal strings would be handled using `setArgument()` with a string
            // argument.
            DateInfo dateInfo = { date, {} }; // No time zone or calendar name
            arguments[k] = Formattable(std::move(dateInfo));
            return *this;
        }
        Builder& setDecimalArgument(const UnicodeString& k, std::string_view decimal, UErrorCode& errorCode) {
            THIS_ON_ERROR(errorCode);
            arguments[k] = Formattable::forDecimal(decimal, errorCode);
            return *this;
        }
        Builder& setArgument(const UnicodeString& k, const FormattableObject* val) {
            U_ASSERT(val != nullptr);
            arguments[k] = Formattable(val);
            return *this;
        }
        Builder& clearArguments() {
            arguments.clear();
            return *this;
        }
        Builder& setExpected(UnicodeString e) {
            hasExpectedOutput = true;
            expected = e;
            return *this;
        }
        Builder& clearExpected() {
            hasExpectedOutput = false;
            return *this;
        }
        Builder& setExpectedError(UErrorCode errorCode) {
            expectedError = U_SUCCESS(errorCode) ? U_ZERO_ERROR : errorCode;
            return *this;
        }
        Builder& setExpectedAnyError() {
            arbitraryError = true;
            return *this;
        }
        Builder& setNoSyntaxError() {
            expectNoSyntaxError = true;
            return *this;
        }
        Builder& setExpectSuccess() {
            return setExpectedError(U_ZERO_ERROR);
        }
        Builder& setLocale(Locale&& loc) {
            locale = loc;
            return *this;
        }
        Builder& setExpectedLineNumberAndOffset(uint32_t line, uint32_t o) {
            hasLineNumberAndOffset = true;
            lineNumber = line;
            offset = o;
            return *this;
        }
        Builder& setIgnoreError() {
            ignoreError = true;
            return *this;
        }
        Builder& clearIgnoreError() {
            ignoreError = false;
            return *this;
        }
        Builder& setFunctionRegistry(const MFFunctionRegistry* reg) {
            U_ASSERT(reg != nullptr);
            functionRegistry = reg;
            return *this;
        }
        TestCase build() const {
            return TestCase(*this);
        }
        virtual ~Builder();

        private:
        UnicodeString testName;
        UnicodeString pattern;
        Locale locale;
        std::map<UnicodeString, Formattable> arguments;
        bool hasExpectedOutput;
        UnicodeString expected;
        UErrorCode expectedError;
        bool arbitraryError;
        bool expectNoSyntaxError;
        bool hasLineNumberAndOffset;
        uint32_t lineNumber;
        uint32_t offset;
        bool ignoreError;
        const MFFunctionRegistry* functionRegistry  = nullptr; // Not owned

        public:
        Builder() : pattern(""), locale(Locale::getDefault()), hasExpectedOutput(false), expected(""), expectedError(U_ZERO_ERROR), arbitraryError(false), expectNoSyntaxError(false), hasLineNumberAndOffset(false), ignoreError(false) {}
    };

    private:
    TestCase(const Builder& builder) :
        testName(builder.testName),
        pattern(builder.pattern),
        locale(builder.locale),
        arguments(builder.arguments),
        expectedError(builder.expectedError),
        arbitraryError(builder.arbitraryError),
        expectedNoSyntaxError(builder.expectNoSyntaxError),
        hasExpectedOutput(builder.hasExpectedOutput),
        expected(builder.expected),
        hasLineNumberAndOffset(builder.hasLineNumberAndOffset),
        lineNumber(builder.hasLineNumberAndOffset ? builder.lineNumber : 0),
        offset(builder.hasLineNumberAndOffset ? builder.offset : 0),
        ignoreError(builder.ignoreError),
        functionRegistry(builder.functionRegistry) {
        // If an error is not expected, then the expected
        // output should be present
        U_ASSERT(expectFailure() || expectNoSyntaxError() || hasExpectedOutput);
    }
}; // class TestCase

class TestUtils {
    public:

    // Runs a single test case
    static void runTestCase(IntlTest& tmsg,
                            const TestCase& testCase,
                            IcuTestErrorCode& errorCode) {
        CHECK_ERROR(errorCode);

        UParseError parseError;
	MessageFormatter::Builder mfBuilder(errorCode);
        mfBuilder.setPattern(testCase.getPattern(), parseError, errorCode).setLocale(testCase.getLocale());

        if (testCase.hasCustomRegistry()) {
            mfBuilder.setFunctionRegistry(*testCase.getCustomRegistry());
        }
        // Initially, set error behavior to strict.
        // We'll re-run to check for errors.
        mfBuilder.setErrorHandlingBehavior(MessageFormatter::U_MF_STRICT);
	MessageFormatter mf = mfBuilder.build(errorCode);
        UnicodeString result;

        // Builder should fail if a syntax error was expected
        if (!testCase.expectSuccess() && testCase.expectedErrorCode() == U_MF_SYNTAX_ERROR) {
            if (errorCode != testCase.expectedErrorCode()) {
                failExpectedFailure(tmsg, testCase, errorCode);
            }
            errorCode.reset();
            return;
        }

        if (U_SUCCESS(errorCode)) {
            result = mf.formatToString(MessageArguments(testCase.getArguments(), errorCode), errorCode);
        }

        const UnicodeString& in = mf.getNormalizedPattern();
        UnicodeString out;
        if (!roundTrip(in, mf.getDataModel(), out)
            // For now, don't round-trip messages with these errors,
            // since duplicate options are dropped
            && (testCase.expectSuccess() ||
                (testCase.expectedErrorCode() != U_MF_DUPLICATE_OPTION_NAME_ERROR))) {
            failRoundTrip(tmsg, testCase, in, out);
        }

        if (testCase.expectNoSyntaxError()) {
            if (errorCode == U_MF_SYNTAX_ERROR) {
                failSyntaxError(tmsg, testCase);
            }
            errorCode.reset();
            return;
        }
        if (testCase.expectSuccess() && U_FAILURE(errorCode)) {
            failExpectedSuccess(tmsg, testCase, errorCode, parseError.line, parseError.offset);
            return;
        }
        if (testCase.expectArbitraryError() && U_SUCCESS(errorCode)) {
            failExpectedArbitraryError(tmsg, testCase);
        }
        if (testCase.expectFailure() && errorCode != testCase.expectedErrorCode()) {
            failExpectedFailure(tmsg, testCase, errorCode);
            return;
        }
        if (!testCase.lineNumberAndOffsetMatch(parseError.line, parseError.offset)) {
            failWrongOffset(tmsg, testCase, parseError.line, parseError.offset);
        }
        if (U_FAILURE(errorCode) && !testCase.expectSuccess()
            && testCase.expectedErrorCode() != U_MF_SYNTAX_ERROR) {
            // Re-run the formatter if there was an error,
            // in order to get best-effort output
            errorCode.reset();
            mfBuilder.setErrorHandlingBehavior(MessageFormatter::U_MF_BEST_EFFORT);
            mf = mfBuilder.build(errorCode);
            if (U_SUCCESS(errorCode)) {
                result = mf.formatToString(MessageArguments(testCase.getArguments(), errorCode), errorCode);
            }
            if (U_FAILURE(errorCode)) {
                // Must be a non-MF2 error code
                U_ASSERT(!(errorCode >= U_MF_UNRESOLVED_VARIABLE_ERROR
                           && errorCode <= U_FMT_PARSE_ERROR_LIMIT));
            }
            // Re-run the formatter
            result = mf.formatToString(MessageArguments(testCase.getArguments(), errorCode), errorCode);
        }
        if (!testCase.outputMatches(result)) {
            failWrongOutput(tmsg, testCase, result);
            return;
        }
        errorCode.reset();
    }

    static bool roundTrip(const UnicodeString& normalizedInput, const MFDataModel& dataModel, UnicodeString& result) {
        Serializer(dataModel, result).serialize();
        return (normalizedInput == result);
    }

    static void failSyntaxError(IntlTest& tmsg, const TestCase& testCase) {
        tmsg.dataerrln(testCase.getTestName());
        tmsg.logln(testCase.getTestName() + " failed test with pattern: " + testCase.getPattern() + " and error code U_MF_SYNTAX_ERROR; expected no syntax error");
    }

    static void failExpectedSuccess(IntlTest& tmsg, const TestCase& testCase, IcuTestErrorCode& errorCode, int32_t line, int32_t offset) {
        tmsg.dataerrln(testCase.getTestName());
        tmsg.logln(testCase.getTestName() + " failed test with pattern: " + testCase.getPattern() + " and error code " + UnicodeString(u_errorName(errorCode)));
        tmsg.dataerrln("line = %d offset = %d", line, offset);
        errorCode.reset();
    }
    static void failExpectedFailure(IntlTest& tmsg, const TestCase& testCase, IcuTestErrorCode& errorCode) {
        tmsg.dataerrln(testCase.getTestName());
        tmsg.errln(testCase.getTestName() + " failed test with wrong error code; pattern: " + testCase.getPattern() + " and error code " + UnicodeString(u_errorName(errorCode)) + " and expected error code: " + UnicodeString(u_errorName(testCase.expectedErrorCode())));
        errorCode.reset();
    }
    static void failExpectedArbitraryError(IntlTest& tmsg, const TestCase& testCase) {
        tmsg.dataerrln(testCase.getTestName());
        tmsg.errln(testCase.getTestName() + " succeeded although any error was expected; pattern: " + testCase.getPattern());
    }
    static void failWrongOutput(IntlTest& tmsg, const TestCase& testCase, const UnicodeString& result) {
        tmsg.dataerrln(testCase.getTestName());
        tmsg.logln(testCase.getTestName() + " failed test with wrong output; pattern: " + testCase.getPattern() + " and expected output = " + testCase.expectedOutput() + " and actual output = " + result);
    }

    static void failRoundTrip(IntlTest& tmsg, const TestCase& testCase, const UnicodeString& in, const UnicodeString& output) {
        tmsg.dataerrln(testCase.getTestName());
        tmsg.logln(testCase.getTestName() + " failed test with wrong output; normalized input = " + in + " serialized data model = " + output);
    }

    static void failWrongOffset(IntlTest& tmsg, const TestCase& testCase, uint32_t actualLine, uint32_t actualOffset) {
        tmsg.dataerrln("Test failed with wrong line or character offset in parse error; expected (line %d, offset %d), got (line %d, offset %d)", testCase.getLineNumber(), testCase.getOffset(),
                  actualLine, actualOffset);
        tmsg.logln(UnicodeString(testCase.getTestName()) + " pattern = " + testCase.getPattern() + " - failed by returning the wrong line number or offset in the parse error");
    }
}; // class TestUtils

} // namespace message2
U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_MF2 */

#endif /* #if !UCONFIG_NO_FORMATTING */

#endif /* #if !UCONFIG_NO_NORMALIZATION */

#endif
