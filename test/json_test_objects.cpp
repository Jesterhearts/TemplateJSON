#include "json_test_objects.hpp"
#include "test_common.hpp"
#include "json.hpp"

JSON_ENABLE(basic, (i));

JSON_ENABLE(nested, (basic1));

JSON_ENABLE(private_member, (member));

JSON_ENABLE(const_member, (c));

void TestNested() {
    TEST_BODY(nested);
}

void TestPrivateMember() {
    TEST_BODY(private_member);
}

void TestConstMember() {
    TEST_BODY(const_member);
}

#ifndef JSON_TEST_ALL
int main() {
    TestNested();
    TestPrivateMember();
    TestConstMember();
}
#endif
