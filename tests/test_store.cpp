// test_store.cpp - console tests for the file-handling layer (no GUI needed).
// Build:  g++ -std=c++17 -Isrc tests/test_store.cpp src/StudentStore.cpp -o store_tests
#include "StudentStore.h"

#include <cstdio>
#include <fstream>
#include <iostream>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { ++failures; std::cout << "FAIL line " << __LINE__ << ": " #cond "\n"; } } while (0)

static Student make(const char* name, const char* roll, double gpa) {
    return Student{name, roll, "Father of " + std::string(name), "Soomro", "Khairpur", gpa};
}

int main() {
    const std::string file = "test_students.txt";
    std::remove(file.c_str());
    std::string err;

    // 1. Missing file -> empty list
    StudentStore s(file);
    CHECK(s.load());
    CHECK(s.count() == 0);

    // 2. Add + persistence
    CHECK(s.add(make("Ali Raza", "K26SW-001", 3.5), err));
    CHECK(s.add(make("Sara Khan", "K26SW-002", 3.9), err));
    CHECK(s.add(make("Ahmed", "K26SW-003", 2.75), err));
    StudentStore reload(file);
    CHECK(reload.load());
    CHECK(reload.count() == 3);
    CHECK(reload.all()[1].name == "Sara Khan");
    CHECK(reload.all()[2].gpa == 2.75);

    // 3. Duplicate roll number is rejected
    CHECK(!s.add(make("Other", "K26SW-001", 3.0), err));
    CHECK(s.count() == 3);

    // 4. Validation
    CHECK(!s.add(make("", "K26SW-010", 3.0), err));
    CHECK(!s.add(make("Bilal", "", 3.0), err));
    CHECK(!s.add(make("Bilal", "K26SW-011", 4.5), err));
    CHECK(!s.add(make("Bilal", "K26SW-012", -1), err));

    // 5. Update by roll number
    Student u = make("Ali Raza Updated", "K26SW-001", 3.8);
    CHECK(s.update(u, err));
    StudentStore r2(file); r2.load();
    CHECK(r2.all()[0].name == "Ali Raza Updated");
    CHECK(r2.all()[0].gpa == 3.8);
    CHECK(!s.update(make("Ghost", "NOPE", 3.0), err));

    // 6. Delete
    CHECK(s.remove("K26SW-002", err));
    StudentStore r3(file); r3.load();
    CHECK(r3.count() == 2);
    CHECK(r3.findByRoll("K26SW-002") == -1);
    CHECK(!s.remove("K26SW-002", err));

    // 7. Commas and quotes inside a field survive a round trip
    Student tricky = make("Khan, Muhammad \"Moon\"", "K26SW-099", 3.0);
    CHECK(s.add(tricky, err));
    StudentStore r4(file); r4.load();
    int i = r4.findByRoll("K26SW-099");
    CHECK(i >= 0 && r4.all()[i].name == "Khan, Muhammad \"Moon\"");

    // 8. Damaged lines are skipped, not fatal
    { std::ofstream bad(file, std::ios::app); bad << "this,is,broken\n\n\"unclosed,1,2,3,4,5\n"; }
    StudentStore r5(file); CHECK(r5.load());
    CHECK(r5.count() == 3);
    CHECK(r5.skippedLines() == 2);

    // 9. File looks like the assignment wants it (comma separated)
    std::cout << "--- " << file << " ---\n";
    { std::ifstream in(file); std::string line; while (std::getline(in, line)) std::cout << line << "\n"; }

    std::remove(file.c_str());
    std::cout << (failures == 0 ? "ALL TESTS PASSED\n" : "SOME TESTS FAILED\n");
    return failures;
}
