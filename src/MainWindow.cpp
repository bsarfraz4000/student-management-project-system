// MainWindow.cpp - GUI implementation.
#include "MainWindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),
      // The records file lives next to the executable.
      store_(QDir(QCoreApplication::applicationDirPath()).filePath("students.txt").toStdString()) {
    buildUi();

    store_.load();
    if (store_.skippedLines() > 0)
        warn(QString("%1 damaged line(s) in students.txt were ignored.").arg(store_.skippedLines()));

    // Start-up state required by the assignment: Add enabled, Update/Delete disabled.
    setMode(Mode::Add);
}

void MainWindow::buildUi() {
    setWindowTitle("Student Record");
    setMinimumWidth(620);

    // --- title -------------------------------------------------------------
    auto* title = new QLabel("Student Record Management System");
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);

    // --- input fields ------------------------------------------------------
    nameEdit_     = new QLineEdit; nameEdit_->setPlaceholderText("e.g. Ali Raza");
    rollEdit_     = new QLineEdit; rollEdit_->setPlaceholderText("e.g. 26SW-001 (must be unique)");
    fatherEdit_   = new QLineEdit; fatherEdit_->setPlaceholderText("e.g. Muhammad Raza");
    casteEdit_    = new QLineEdit; casteEdit_->setPlaceholderText("e.g. Soomro");
    districtEdit_ = new QLineEdit; districtEdit_->setPlaceholderText("e.g. Khairpur");
    gpaEdit_      = new QLineEdit; gpaEdit_->setPlaceholderText("0.00 - 4.00");

    auto* form = new QGridLayout;
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(12);
    const struct { const char* label; QLineEdit* edit; } rows[] = {
        {"Student Name",  nameEdit_},   {"Roll No :",     rollEdit_},
        {"Father Name",   fatherEdit_}, {"Student Caste", casteEdit_},
        {"District",      districtEdit_}, {"Student GPA", gpaEdit_}};
    int r = 0;
    for (const auto& row : rows) {
        form->addWidget(new QLabel(row.label), r, 0);
        form->addWidget(row.edit, r, 1);
        ++r;
    }
    form->setColumnStretch(1, 1);

    // --- navigation buttons (right side) -----------------------------------
    nextBtn_ = new QPushButton("Next");
    prevBtn_ = new QPushButton("Previous");
    auto* navLayout = new QVBoxLayout;
    navLayout->addWidget(nextBtn_);
    navLayout->addWidget(prevBtn_);
    navLayout->addStretch();

    auto* middle = new QHBoxLayout;
    middle->addLayout(form, 1);
    middle->addSpacing(20);
    middle->addLayout(navLayout);

    // --- action buttons (bottom) -------------------------------------------
    addBtn_    = new QPushButton("Add");
    updateBtn_ = new QPushButton("Update");
    deleteBtn_ = new QPushButton("Delete");
    newBtn_    = new QPushButton("Clear / New");
    deleteBtn_->setObjectName("deleteButton");

    auto* actions = new QHBoxLayout;
    actions->addWidget(addBtn_);
    actions->addWidget(updateBtn_);
    actions->addWidget(deleteBtn_);
    actions->addWidget(newBtn_);

    // --- search row (extra feature) ----------------------------------------
    searchEdit_ = new QLineEdit; searchEdit_->setPlaceholderText("Search by Roll No...");
    findBtn_ = new QPushButton("Find");
    auto* search = new QHBoxLayout;
    search->addWidget(searchEdit_, 1);
    search->addWidget(findBtn_);

    // --- status line -------------------------------------------------------
    statusLabel_ = new QLabel;
    statusLabel_->setObjectName("status");

    // --- assemble ----------------------------------------------------------
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(26, 18, 26, 14);
    root->setSpacing(14);
    root->addWidget(title);
    root->addLayout(search);
    root->addLayout(middle);
    root->addLayout(actions);
    root->addWidget(statusLabel_);

    setStyleSheet(
        "QWidget { background:#eef1f6; font-size:14px; color:#1f2933; }"
        "QLabel#title { font-size:20px; font-weight:bold; color:#1d3b6e; padding:4px; }"
        "QLabel#status { color:#52606d; font-size:12px; }"
        "QLineEdit { background:white; border:1px solid #b8c2cf; border-radius:4px; padding:6px; }"
        "QLineEdit:focus { border:1px solid #2f6fd0; }"
        "QLineEdit:read-only { background:#e4e8ee; }"
        "QPushButton { background:#2f6fd0; color:white; border:none; border-radius:4px;"
        "              padding:8px 16px; font-weight:bold; }"
        "QPushButton:hover { background:#2459ab; }"
        "QPushButton:disabled { background:#b8c2cf; color:#eef1f6; }"
        "QPushButton#deleteButton { background:#d64545; }"
        "QPushButton#deleteButton:hover { background:#ad3333; }"
        "QPushButton#deleteButton:disabled { background:#b8c2cf; }");

    // --- connect signals (button click -> handler) -------------------------
    connect(addBtn_,    &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(updateBtn_, &QPushButton::clicked, this, &MainWindow::onUpdate);
    connect(deleteBtn_, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(nextBtn_,   &QPushButton::clicked, this, &MainWindow::onNext);
    connect(prevBtn_,   &QPushButton::clicked, this, &MainWindow::onPrevious);
    connect(newBtn_,    &QPushButton::clicked, this, &MainWindow::onNew);
    connect(findBtn_,   &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(searchEdit_, &QLineEdit::returnPressed, this, &MainWindow::onSearch);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
void MainWindow::info(const QString& text) { QMessageBox::information(this, "Student Record", text); }
void MainWindow::warn(const QString& text) { QMessageBox::warning(this, "Student Record", text); }

// Switch between "Add" mode and "Browse" mode (this decides which buttons work).
void MainWindow::setMode(Mode mode) {
    mode_ = mode;
    const bool browse = (mode == Mode::Browse);

    addBtn_->setEnabled(!browse);       // Add only when entering a new record
    updateBtn_->setEnabled(browse);     // Update/Delete only for a displayed record
    deleteBtn_->setEnabled(browse);
    rollEdit_->setReadOnly(browse);     // Roll No is the unique ID, so it can't change

    if (!browse) currentIndex_ = -1;
    refreshStatus();
}

void MainWindow::refreshStatus() {
    const bool any = store_.count() > 0;
    nextBtn_->setEnabled(any);
    prevBtn_->setEnabled(any);

    QString text;
    if (mode_ == Mode::Add)
        text = QString("Adding a new record  |  Total records: %1").arg(store_.count());
    else
        text = QString("Record %1 of %2").arg(currentIndex_ + 1).arg(store_.count());
    statusLabel_->setText(text + "  |  File: " + QString::fromStdString(store_.filePath()));
}

void MainWindow::clearForm() {
    nameEdit_->clear(); rollEdit_->clear(); fatherEdit_->clear();
    casteEdit_->clear(); districtEdit_->clear(); gpaEdit_->clear();
}

// Show one stored record in the text fields and switch to Browse mode.
void MainWindow::showRecord(int index) {
    if (index < 0 || index >= static_cast<int>(store_.count())) return;
    const Student& s = store_.all()[static_cast<std::size_t>(index)];

    nameEdit_->setText(QString::fromStdString(s.name));
    rollEdit_->setText(QString::fromStdString(s.rollNo));
    fatherEdit_->setText(QString::fromStdString(s.fatherName));
    casteEdit_->setText(QString::fromStdString(s.caste));
    districtEdit_->setText(QString::fromStdString(s.district));
    gpaEdit_->setText(QString::number(s.gpa, 'f', 2));

    currentIndex_ = index;
    setMode(Mode::Browse);
}

// Copy the form into a Student. Returns false (after telling the user) if invalid.
bool MainWindow::readForm(Student& out) {
    out.name       = nameEdit_->text().toStdString();
    out.rollNo     = rollEdit_->text().toStdString();
    out.fatherName = fatherEdit_->text().toStdString();
    out.caste      = casteEdit_->text().toStdString();
    out.district   = districtEdit_->text().toStdString();

    bool ok = false;
    out.gpa = gpaEdit_->text().trimmed().toDouble(&ok);   // QString::toDouble uses the "C" locale
    if (!ok) {
        warn("GPA must be a number, for example 3.45");
        gpaEdit_->setFocus();
        return false;
    }

    std::string error;
    if (!StudentStore::validate(StudentStore::normalise(out), error)) {
        warn(QString::fromStdString(error));
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Button handlers
// ---------------------------------------------------------------------------
void MainWindow::onAdd() {
    Student s;
    if (!readForm(s)) return;

    std::string error;
    if (!store_.add(s, error)) {                 // also checks that the Roll No is unique
        warn(QString::fromStdString(error));
        return;
    }
    info("Student record added successfully.");  // confirmation message
    clearForm();
    setMode(Mode::Add);
    nameEdit_->setFocus();
}

void MainWindow::onUpdate() {
    Student s;
    if (!readForm(s)) return;

    std::string error;
    if (!store_.update(s, error)) {              // record is found by its Roll No
        warn(QString::fromStdString(error));
        return;
    }
    info("Student record updated successfully.");
    showRecord(currentIndex_);                   // redisplay the saved (cleaned-up) values
}

void MainWindow::onDelete() {
    const QString roll = rollEdit_->text().trimmed();
    const auto answer = QMessageBox::question(
        this, "Delete record",
        QString("Are you sure you want to delete the record of Roll No '%1'?").arg(roll),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    std::string error;
    if (!store_.remove(roll.toStdString(), error)) {
        warn(QString::fromStdString(error));
        return;
    }
    info("Student record deleted successfully.");

    if (store_.count() == 0) {                   // nothing left to browse
        onNew();
    } else {
        int next = currentIndex_;
        if (next >= static_cast<int>(store_.count())) next = static_cast<int>(store_.count()) - 1;
        showRecord(next);
    }
}

void MainWindow::onNext() {
    if (store_.count() == 0) { info("There are no records to show yet."); return; }
    int next = currentIndex_ + 1;                // from Add mode currentIndex_ is -1 -> first record
    if (next >= static_cast<int>(store_.count())) next = 0;   // wrap around after the last record
    showRecord(next);
}

void MainWindow::onPrevious() {
    if (store_.count() == 0) { info("There are no records to show yet."); return; }
    int prev = (currentIndex_ <= 0) ? static_cast<int>(store_.count()) - 1 : currentIndex_ - 1;
    showRecord(prev);
}

void MainWindow::onNew() {
    clearForm();
    setMode(Mode::Add);
    nameEdit_->setFocus();
}

void MainWindow::onSearch() {
    const std::string roll = searchEdit_->text().trimmed().toStdString();
    if (roll.empty()) return;

    const int idx = store_.findByRoll(roll);
    if (idx < 0) { info("No student found with that Roll No."); return; }
    showRecord(idx);
}
