# Student Record Management System (C++ / Qt)

GUI application for adding, updating, deleting and browsing student records.
Course: SW112 - Programming Fundamentals (Open Ended Lab), MUET SZAB Campus Khairpur Mir's.

## Features
- Form fields: Name, Roll Number, Father's Name, Caste, District, GPA
- Buttons: **Add**, **Update**, **Delete**, **Next**
- Records are saved in `students.txt`, one student per line, fields separated by commas:
  `Name,RollNo,FatherName,Caste,District,GPA`
- Start-up: Add enabled, Update and Delete disabled
- Add: validates, saves, shows a confirmation message
- Next: shows the next record (wraps to the first); Add is disabled, Update and Delete are enabled
- Update: changes the displayed record, using the Roll Number as the unique identifier (Roll No is read-only in this mode)
- Delete: asks for confirmation, removes the displayed record from the file

### Extra features (creativity)
- Previous button, Clear / New button (back to Add mode), search by Roll No
- Duplicate Roll Number check, GPA range check (0.00 - 4.00), empty-field checks
- Commas and quotes inside a field are stored safely (CSV quoting)
- Safe saving (temporary file, then rename) and damaged lines are skipped instead of crashing
- Status line showing record number, total records and the file path

## Project structure
```
src/StudentStore.h/.cpp   data layer: file handling, validation (plain C++, no GUI)
src/MainWindow.h/.cpp     GUI: form, buttons, event handlers (Qt Widgets)
src/main.cpp              entry point
tests/test_store.cpp      console tests for the data layer
CMakeLists.txt            CMake build (Qt 5 or Qt 6)
StudentRecordManager.pro  qmake build (alternative)
students.txt              sample data (3 records)
```

## How to build and run

### Option A: Qt Creator (easiest)
1. Install Qt (https://www.qt.io/download-open-source) with a MinGW or MSVC kit and Qt Creator.
2. Open Qt Creator -> File -> Open File or Project -> select `CMakeLists.txt` (or `StudentRecordManager.pro`).
3. Choose a kit, click **Configure Project**, then press the green **Run** button.
4. `students.txt` is created/read in the same folder as the built `.exe`.
   Copy the sample `students.txt` there if you want the 3 sample records.

### Option B: command line
```
mkdir build && cd build
cmake ..
cmake --build .
./StudentRecordManager
```

### Run the data-layer tests (no Qt needed)
```
g++ -std=c++17 -Isrc tests/test_store.cpp src/StudentStore.cpp -o store_tests
./store_tests
```

## Upload to GitHub (deliverable 1)
```
git init
git add .
git commit -m "Student Record Management System"
git branch -M main
git remote add origin https://github.com/<your-username>/<repo-name>.git
git push -u origin main
```
