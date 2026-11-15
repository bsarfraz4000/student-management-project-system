// StudentStore.h
// ---------------------------------------------------------------------------
// Data layer of the Student Record Management System.
// Pure standard C++ (no GUI code) so it can be tested on its own.
// Records are stored in a text file, one student per line, with the fields
// separated by commas:
//     Name,RollNo,FatherName,Caste,District,GPA
// Fields that contain a comma or a double quote are wrapped in "quotes".
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <string>
#include <vector>

// One student record (the six fields required by the assignment).
struct Student {
    std::string name;
    std::string rollNo;      // unique identifier
    std::string fatherName;
    std::string caste;
    std::string district;
    double      gpa = 0.0;
};

class StudentStore {
public:
    explicit StudentStore(const std::string& filePath);

    // File operations -------------------------------------------------------
    bool load();               // read all records (a missing file = empty list)
    bool save() const;         // write all records back to the text file

    // Record operations (each one saves the file immediately) ---------------
    bool add(const Student& s, std::string& error);
    bool update(const Student& s, std::string& error);        // matched by rollNo
    bool remove(const std::string& rollNo, std::string& error);

    // Queries ---------------------------------------------------------------
    const std::vector<Student>& all() const { return records_; }
    std::size_t count() const { return records_.size(); }
    int  findByRoll(const std::string& rollNo) const;         // -1 if not found
    const std::string& filePath() const { return path_; }
    int  skippedLines() const { return skipped_; }            // bad lines ignored by load()

    // Helpers (public so they can be unit-tested) ---------------------------
    static bool        validate(const Student& s, std::string& error);
    static Student     normalise(const Student& s);           // trims every field
    static std::string toCsvLine(const Student& s);
    static bool        fromCsvLine(const std::string& line, Student& out);

private:
    std::string          path_;
    std::vector<Student> records_;
    int                  skipped_ = 0;
};
