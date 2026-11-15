// StudentStore.cpp - implementation of the file-handling / data layer.
#include "StudentStore.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>

namespace {

// Remove leading and trailing white space.
std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    std::size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

// Wrap a field in quotes when it contains a comma, a quote or a line break.
std::string csvEscape(const std::string& f) {
    if (f.find_first_of(",\"\r\n") == std::string::npos) return f;
    std::string out = "\"";
    for (char c : f) {
        if (c == '"') out += "\"\"";   // a quote inside a field is doubled
        else          out += c;
    }
    out += '"';
    return out;
}

// Split one CSV line into fields (understands "quoted, fields").
std::vector<std::string> splitCsv(const std::string& line, bool& ok) {
    std::vector<std::string> fields;
    std::string cur;
    bool inQuotes = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { cur += '"'; ++i; }
                else inQuotes = false;
            } else cur += c;
        } else {
            if (c == '"')      inQuotes = true;
            else if (c == ',') { fields.push_back(cur); cur.clear(); }
            else               cur += c;
        }
    }
    ok = !inQuotes;            // an unclosed quote means a damaged line
    fields.push_back(cur);
    return fields;
}

// Convert text to a double using the "C" locale (so 3.5 is never 3,5).
bool parseDouble(const std::string& text, double& value) {
    std::istringstream in(trim(text));
    in.imbue(std::locale::classic());
    in >> value;
    if (in.fail()) return false;
    in >> std::ws;
    return in.eof();
}

bool hasLineBreak(const std::string& s) {
    return s.find_first_of("\r\n") != std::string::npos;
}

} // namespace

// ---------------------------------------------------------------------------
StudentStore::StudentStore(const std::string& filePath) : path_(filePath) {}

// ---------------------------------------------------------------------------
Student StudentStore::normalise(const Student& s) {
    Student t = s;
    t.name = trim(s.name);
    t.rollNo = trim(s.rollNo);
    t.fatherName = trim(s.fatherName);
    t.caste = trim(s.caste);
    t.district = trim(s.district);
    return t;
}

bool StudentStore::validate(const Student& s, std::string& error) {
    struct Field { const char* label; const std::string* value; };
    const Field fields[] = {
        {"Name", &s.name}, {"Roll Number", &s.rollNo}, {"Father's Name", &s.fatherName},
        {"Caste", &s.caste}, {"District", &s.district}};

    for (const Field& f : fields) {
        if (f.value->empty()) { error = std::string(f.label) + " cannot be empty."; return false; }
        if (hasLineBreak(*f.value)) { error = std::string(f.label) + " cannot contain line breaks."; return false; }
    }
    if (std::isnan(s.gpa) || s.gpa < 0.0 || s.gpa > 4.0) {
        error = "GPA must be a number between 0.0 and 4.0.";
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
std::string StudentStore::toCsvLine(const Student& s) {
    std::ostringstream gpa;
    gpa.imbue(std::locale::classic());
    gpa << std::fixed << std::setprecision(2) << s.gpa;

    return csvEscape(s.name) + "," + csvEscape(s.rollNo) + "," + csvEscape(s.fatherName) + "," +
           csvEscape(s.caste) + "," + csvEscape(s.district) + "," + gpa.str();
}

bool StudentStore::fromCsvLine(const std::string& line, Student& out) {
    bool ok = true;
    std::vector<std::string> f = splitCsv(line, ok);
    if (!ok || f.size() != 6) return false;

    Student s;
    s.name = f[0]; s.rollNo = f[1]; s.fatherName = f[2]; s.caste = f[3]; s.district = f[4];
    if (!parseDouble(f[5], s.gpa)) return false;
    out = normalise(s);
    return true;
}

// ---------------------------------------------------------------------------
bool StudentStore::load() {
    records_.clear();
    skipped_ = 0;

    std::ifstream in(path_);
    if (!in.is_open()) return true;          // no file yet = no records yet

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();   // Windows line ends
        if (trim(line).empty()) continue;

        Student s;
        std::string err;
        if (fromCsvLine(line, s) && validate(s, err)) records_.push_back(s);
        else ++skipped_;                      // ignore damaged lines instead of crashing
    }
    return true;
}

bool StudentStore::save() const {
    // Write to a temporary file first so the real file is never left half-written.
    const std::string tmp = path_ + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out.is_open()) return false;
        for (const Student& s : records_) out << toCsvLine(s) << '\n';
        out.flush();
        if (!out) return false;
    }
    std::remove(path_.c_str());               // needed on Windows (rename won't overwrite)
    return std::rename(tmp.c_str(), path_.c_str()) == 0;
}

// ---------------------------------------------------------------------------
int StudentStore::findByRoll(const std::string& rollNo) const {
    const std::string key = trim(rollNo);
    for (std::size_t i = 0; i < records_.size(); ++i)
        if (records_[i].rollNo == key) return static_cast<int>(i);
    return -1;
}

bool StudentStore::add(const Student& input, std::string& error) {
    Student s = normalise(input);
    if (!validate(s, error)) return false;
    if (findByRoll(s.rollNo) >= 0) {
        error = "A student with Roll Number '" + s.rollNo + "' already exists.";
        return false;
    }
    records_.push_back(s);
    if (!save()) {
        records_.pop_back();                  // undo so memory matches the file
        error = "Could not write to file: " + path_;
        return false;
    }
    return true;
}

bool StudentStore::update(const Student& input, std::string& error) {
    Student s = normalise(input);
    if (!validate(s, error)) return false;
    int idx = findByRoll(s.rollNo);
    if (idx < 0) { error = "No student with Roll Number '" + s.rollNo + "' was found."; return false; }

    Student old = records_[idx];
    records_[idx] = s;
    if (!save()) {
        records_[idx] = old;
        error = "Could not write to file: " + path_;
        return false;
    }
    return true;
}

bool StudentStore::remove(const std::string& rollNo, std::string& error) {
    int idx = findByRoll(rollNo);
    if (idx < 0) { error = "No student with Roll Number '" + trim(rollNo) + "' was found."; return false; }

    Student old = records_[idx];
    records_.erase(records_.begin() + idx);
    if (!save()) {
        records_.insert(records_.begin() + idx, old);
        error = "Could not write to file: " + path_;
        return false;
    }
    return true;
}
